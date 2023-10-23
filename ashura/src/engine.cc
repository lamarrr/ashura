#include "ashura/engine.h"

#include <chrono>
#include <fstream>

#include "ashura/animation.h"
#include "ashura/canvas.h"
#include "ashura/loggers.h"
#include "ashura/palletes.h"
#include "ashura/sdl_utils.h"
#include "ashura/shaders.h"
#include "ashura/subsystems/font_loader.h"
#include "ashura/subsystems/font_manager.h"
#include "ashura/subsystems/image_loader.h"
#include "ashura/subsystems/image_manager.h"
#include "ashura/subsystems/vulkan_font_manager.h"
#include "ashura/subsystems/vulkan_image_manager.h"
#include "ashura/vulkan_context.h"
#include "ashura/window_manager.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#define TIMER_BEGIN(name) \
  ::std::chrono::steady_clock::time_point name##_TIMER_Begin = ::std::chrono::steady_clock::now()
#define TIMER_END(name, str)                                                \
  ASH_LOG_INFO(FontRenderer, "Timer: {}, Task: {}, took: {}ms", #name, str, \
               (::std::chrono::steady_clock::now() - name##_TIMER_Begin).count() / 1'000'000.0f)

namespace ash
{

inline stx::Option<stx::Span<vk::PhyDeviceInfo const>>
    select_device(stx::Span<vk::PhyDeviceInfo const> const phy_devices,
                  stx::Span<VkPhysicalDeviceType const>    preferred_device_types,
                  vk::Surface const                       &target_surface)
{
  for (VkPhysicalDeviceType type : preferred_device_types)
  {
    if (stx::Span selected = phy_devices.which([&](vk::PhyDeviceInfo const &dev) -> bool {
          return dev.properties.deviceType == type &&
                 // can use shaders (fragment and vertex)
                 dev.has_geometry_shader() &&
                 // has graphics command queue for rendering commands
                 dev.has_graphics_command_queue_family() &&
                 // has data transfer command queue for uploading textures
                 // or data
                 dev.has_transfer_command_queue_family() &&
                 // can be used for presenting to a specific surface
                 !vk::get_surface_presentation_command_queue_support(
                      dev.phy_device, dev.family_properties, target_surface.surface)
                      .span()
                      .find(true)
                      .is_empty();
        });
        !selected.is_empty())
    {
      return stx::Some(std::move(selected));
    }
  }

  return stx::None;
}

Engine::Engine(AppConfig const &cfg, Widget *iroot_widget) :
    uuid_generator{stx::rc::make_inplace<PrngUuidGenerator>(stx::os_allocator).unwrap()},
    task_scheduler{stx::os_allocator, std::chrono::steady_clock::now()},
    root_widget{iroot_widget}
{
  ctx.task_scheduler = &task_scheduler;
  ctx.clipboard      = &clipboard;
  ctx.window_manager = &window_manager;
  ctx.root           = root_widget;

  stx::Vec<char const *> required_device_extensions;

  required_device_extensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME).unwrap();

  stx::Vec<char const *> required_validation_layers;

  if (cfg.enable_validation_layers)
  {
    required_validation_layers.push("VK_LAYER_KHRONOS_validation").unwrap();
  }

  ASH_LOG_INFO(Init, "Initializing Window API");

  root_window =
      stx::Some(window_manager.create_window(cfg.name.c_str(), cfg.root_window_type,
                                             cfg.root_window_create_flags, cfg.root_window_extent));

  ASH_LOG_INFO(Init, "Creating root window");

  stx::Vec required_window_instance_extensions = Window::get_required_instance_extensions();

  stx::Rc vk_instance =
      vk::create_instance(cfg.name.c_str(), VK_MAKE_VERSION(0, 0, 1), cfg.name.c_str(),
                          VK_MAKE_VERSION(cfg.version.major, cfg.version.minor, cfg.version.patch),
                          required_window_instance_extensions, required_validation_layers);

  root_window.value()->attach_surface(vk_instance.share());

  stx::Vec phy_devices = vk::get_all_devices(vk_instance);

  VkPhysicalDeviceType const device_preference[] = {
      VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
      VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, VK_PHYSICAL_DEVICE_TYPE_CPU};

  ASH_LOG_INFO(Init, "Available Physical Devices:");

  for (vk::PhyDeviceInfo const &device : phy_devices)
  {
    // TODO(lamarrr): log graphics families on devices and other properties
    ASH_LOG_INFO(Init, "Device(name: '{}', ID: {}, type: {})", device.properties.deviceName,
                 device.properties.deviceID,
                 string_VkPhysicalDeviceType(device.properties.deviceType));
  }

  stx::Rc<vk::PhyDeviceInfo *> phy_device =
      stx::rc::make(
          stx::os_allocator,
          select_device(phy_devices, device_preference, *root_window.value()->surface.value())
              .expect("Unable to find any suitable rendering device")[0]
              .copy())
          .unwrap();

  ASH_LOG_INFO(Init, "Selected Physical Device: Device(name: '{}', ID: {}, type: {})",
               phy_device->properties.deviceName, phy_device->properties.deviceID,
               string_VkPhysicalDeviceType(phy_device->properties.deviceType));

  // we might need multiple command queues, one for data transfer and one for
  // rendering
  f32 queue_priorities[] = {// priority for command queue used for
                            // presentation, rendering, data transfer
                            1};

  stx::Rc graphics_command_queue_family =
      stx::rc::make(
          stx::os_allocator,
          vk::get_graphics_command_queue(phy_device).expect("Unable to get graphics command queue"))
          .unwrap();

  // we can accept queue family struct here instead and thus not have to
  // perform extra manual checks
  // the user shouldn't have to touch handles
  VkDeviceQueueCreateInfo command_queue_create_infos[] = {
      {.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
       .pNext            = nullptr,
       .flags            = 0,
       .queueFamilyIndex = graphics_command_queue_family->index,
       .queueCount       = AS(u32, std::size(queue_priorities)),
       .pQueuePriorities = queue_priorities}};

  VkPhysicalDeviceFeatures required_features{};

  required_features.samplerAnisotropy       = VK_TRUE;
  required_features.pipelineStatisticsQuery = VK_TRUE;

  stx::Rc<vk::Device *> device =
      vk::create_device(phy_device, command_queue_create_infos, required_device_extensions,
                        required_validation_layers, required_features);

  stx::Rc<vk::CommandQueue *> xqueue =
      stx::rc::make_inplace<vk::CommandQueue>(
          stx::os_allocator, vk::get_command_queue(device, *graphics_command_queue_family.handle, 0)
                                 .expect("Failed to create graphics command queue"))
          .unwrap();

  queue = stx::Some(xqueue.share());

  root_window.value()->recreate_swapchain(xqueue, DEFAULT_MAX_FRAMES_IN_FLIGHT);
  auto &swp = root_window.value()->surface.value()->swapchain.value();

  ASH_LOG_INFO(Init,
               "recreated swapchain for logical/window/viewport extent: [{}, {}], physical/surface "
               "extent: [{}, {}]",
               swp.window_extent.width, swp.window_extent.height, swp.image_extent.width,
               swp.image_extent.height);

  render_resource_manager.init(xqueue.share());
  pipeline_manager.init(xqueue->device->dev);

  renderer.init(xqueue->device->dev, xqueue->info.queue, xqueue->info.family.index,
                xqueue->device->phy_dev->properties.limits.timestampPeriod,
                xqueue->device->phy_dev->memory_properties, DEFAULT_MAX_FRAMES_IN_FLIGHT);

  for (CanvasPipelineSpec const &spec : cfg.pipelines)
  {
    pipeline_manager.add_pipeline(spec);
  }

  pipeline_manager.rebuild_for_renderpass(swp.render_pass, swp.msaa_sample_count);

  root_window.value()->on(WindowEvents::CloseRequested,
                          stx::fn::rc::make_unique_functor(stx::os_allocator, [](WindowEvents) {
                            std::exit(0);
                          }).unwrap());

  root_window.value()->on(WindowEvents::Resized | WindowEvents::PixelSizeChanged,
                          stx::fn::rc::make_unique_functor(stx::os_allocator, [](WindowEvents) {
                            ASH_LOG_INFO(Init, "WINDOW RESIZED");
                          }).unwrap());
  root_window.value()->on_mouse_click(stx::fn::rc::make_unique_static([](MouseClickEvent event) {
    if (event.action == KeyAction::Press && event.button == MouseButton::A2)
    {
      std::exit(0);
    }
  }));
  u8 transparent_image_data[] = {0xFF, 0xFF, 0xFF, 0xFF};

  gfx::image transparent_image =
      render_resource_manager.add_image(ImageView<u8 const>{.span   = transparent_image_data,
                                                            .extent = {1, 1},
                                                            .pitch  = 4,
                                                            .format = ImageFormat::Rgba8888},
                                        false);

  ASH_CHECK(transparent_image == 0);

  u8 icon[] = {0xFF, 0xFF, 0xFF, 0xFF};

  root_window.value()->set_icon(ImageView<u8 const>{
      .span = icon, .extent = {1, 1}, .pitch = 4, .format = ImageFormat::Rgba8888});

  ctx.register_subsystem(new VulkanImageManager{render_resource_manager});
  ctx.register_subsystem(new ImageLoader{});

  root_window.value()->on_mouse_click(
      stx::fn::rc::make_unique_functor(stx::os_allocator, [this](MouseClickEvent event) {
        widget_system.events.push_inplace(event).unwrap();
      }).unwrap());
  root_window.value()->on_mouse_motion(
      stx::fn::rc::make_unique_functor(stx::os_allocator, [this](MouseMotionEvent event) {
        widget_system.events.push_inplace(event).unwrap();
      }).unwrap());
  root_window.value()->on_key(
      stx::fn::rc::make_unique_functor(stx::os_allocator, [this](KeyEvent event) {
        ctx.key_events.push_inplace(event).unwrap();
      }).unwrap());
  root_window.value()->on(
      WindowEvents::All,
      stx::fn::rc::make_unique_functor(stx::os_allocator, [this](WindowEvents events) {
        if ((events & WindowEvents::MouseLeave) != WindowEvents::None)
        {
          widget_system.events.push_inplace(events).unwrap();
        }
      }).unwrap());
  TIMER_BEGIN(AllFontLoad);

  for (FontSpec const &spec : cfg.fonts)
  {
    TIMER_BEGIN(FontLoadFromFile);
    ASH_LOG_INFO(Init, "Loading font: {} from file: {}", spec.name.view(), spec.path.view());
    stx::Result result = load_font_from_file(spec.path, spec.face);
    TIMER_END(FontLoadFromFile, "Rendering Font");

    if (result.is_ok())
    {
      TIMER_BEGIN(FontGlyphRender);
      ASH_LOG_INFO(Init, "Loaded font: {} from file: {}", spec.name.view(), spec.path.view());
      auto [atlas, image_buffers] = render_font_atlas(*result.value(), spec);
      for (usize i = 0; i < image_buffers.size(); i++)
      {
        atlas.bins[i].texture = render_resource_manager.add_image(image_buffers[i], false);
      }
      font_bundle
          .push(BundledFont{.name  = spec.name.copy(stx::os_allocator).unwrap(),
                            .font  = std::move(result.value()),
                            .atlas = std::move(atlas)})
          .unwrap();
    }
    else
    {
      ASH_LOG_ERR(Init, "Failed to load font: {} from file: {}, error: {}", spec.name.view(),
                  spec.path.view(), AS(i64, result.err()));
    }
  }

  TIMER_END(AllFontLoad, "All Font Rendering");

  ctx.font_bundle = font_bundle;

  // TODO(lamarrr): attach debug widgets: FPS stats, memory usage, etc

  for (Subsystem *subsystem : ctx.subsystems)
  {
    subsystem->on_startup(ctx);
    ASH_LOG_INFO(Context, "Initialized subsystem: {} (type: {})", subsystem->get_name(),
                 typeid(*subsystem).name());
  }

  VkImageFormatProperties image_format_properties;
  ASH_VK_CHECK(vkGetPhysicalDeviceImageFormatProperties(
      queue.value()->device->phy_dev->phy_device, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TYPE_2D,
      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, 0, &image_format_properties));

  fmt::println("max: {}", image_format_properties.maxMipLevels);
}

void Engine::tick(std::chrono::nanoseconds interval)
{
  timepoint begin = Clock::now();
  // poll events to make the window not be marked as unresponsive.
  // poll events from SDL's event queue until there are none left.
  //
  task_scheduler.tick(interval);
  do
  {
  } while (ctx.poll_events());

  // root_window->tick(interval);
  ctx.tick(interval);
  widget_system.pump_widget_events(widget_tree, ctx);
  widget_system.tick_widgets(ctx, *root_widget, interval);
  ctx.key_events.clear();
  // new widgets could have been added
  widget_system.assign_widget_uuids(ctx, *root_widget, *uuid_generator);
  widget_tree.build(ctx, *root_widget);
  render_resource_manager.execute_deletes();
  render_resource_manager.submit_uploads();

  auto record_draw_commands = [&]() {
    VkExtent2D extent = root_window.value()->surface.value()->swapchain.value().window_extent;
    Vec2       viewport_extent{AS(f32, extent.width), AS(f32, extent.height)};
    widget_tree.layout(ctx, viewport_extent);
    widget_tree.render(ctx, canvas, Rect{.offset = {}, .extent = viewport_extent}, viewport_extent);
  };

  // only record if swapchain visible,
  // if extent is zero, do not present, record, or recreate swapchain, or
  // acquire swapchain image, or submit to renderer
  // do not increase the frame flight indices as well since the sync primitves
  // aren't used
  if (!root_window.value()->surface.value()->is_zero_sized_swapchain)
  {
    record_draw_commands();
  }
  // only try to present if the pipeline has new changes or window was
  // resized

  // only try to recreate swapchain if the present swapchain can't be used for
  // presentation

  // TODO(lamarrr): re-think this structure
  SwapChainState swapchain_state = SwapChainState::Ok;

  // TODO(lamarrr): restructure this part and make it more sane
  do
  {
    if (swapchain_state != SwapChainState::Ok)
    {
      root_window.value()->recreate_swapchain(queue.value(), DEFAULT_MAX_FRAMES_IN_FLIGHT);
      // TODO(lamarrr): fix
      if (!root_window.value()->surface.value()->is_zero_sized_swapchain)
      {
        auto &swp = root_window.value()->surface.value()->swapchain.value();
        ASH_LOG_INFO(Window,
                     "recreated swapchain for logical/window/viewport extent: [{}, {}], "
                     "physical/surface extent: [{}, {}]",
                     swp.window_extent.width, swp.window_extent.height, swp.image_extent.width,
                     swp.image_extent.height);
        pipeline_manager.rebuild_for_renderpass(swp.render_pass, swp.msaa_sample_count);
        record_draw_commands();
      }
    }

    if (!root_window.value()->surface.value()->is_zero_sized_swapchain)
    {
      vk::SwapChain &swapchain = root_window.value()->surface.value()->swapchain.value();

      auto [state, swapchain_image_index] = root_window.value()->acquire_image();

      swapchain_state = state;

      if (swapchain_state != SwapChainState::Ok)
      {
        continue;
      }

      gfx::DrawList const &draw_list = canvas.draw_list;

      renderer.submit(swapchain.window_extent, swapchain.image_extent, swapchain.frame,
                      swapchain.render_fences[swapchain.frame],
                      swapchain.image_acquisition_semaphores[swapchain.frame],
                      swapchain.render_semaphores[swapchain.frame], swapchain.render_pass,
                      swapchain.framebuffers[swapchain_image_index], draw_list.commands,
                      draw_list.vertices, draw_list.indices, pipeline_manager,
                      render_resource_manager, ctx.frame_stats);

      swapchain_state =
          root_window.value()->present(queue.value()->info.queue, swapchain_image_index);

      // the frame semaphores and synchronization primitives are still used even
      // if an error is returned
      swapchain.frame = (swapchain.frame + 1) % swapchain.max_nframes_in_flight;
    }
    else
    {
      swapchain_state = SwapChainState::Ok;
    }
  } while (swapchain_state != SwapChainState::Ok);

  ctx.frame_stats.cpu_time = Clock::now() - begin;
}

}        // namespace ash
