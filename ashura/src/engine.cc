#include "ashura/engine.h"

#include <chrono>
#include <fstream>

#include "ashura/animation.h"
#include "ashura/canvas.h"
#include "ashura/loggers.h"
#include "ashura/palletes.h"
#include "ashura/plugins/font_loader.h"
#include "ashura/plugins/font_manager.h"
#include "ashura/plugins/image_loader.h"
#include "ashura/plugins/image_manager.h"
#include "ashura/plugins/vulkan_font_manager.h"
#include "ashura/plugins/vulkan_image_manager.h"
#include "ashura/sdl_utils.h"
#include "ashura/shaders.h"
#include "ashura/vulkan_context.h"
#include "ashura/window_manager.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#define TIMER_BEGIN(name) ::std::chrono::steady_clock::time_point name##_TIMER_Begin = ::std::chrono::steady_clock::now()
#define TIMER_END(name, str) ASH_LOG_INFO(FontRenderer, "Timer: {}, Task: {}, took: {}ms", #name, str, (::std::chrono::steady_clock::now() - name##_TIMER_Begin).count() / 1'000'000.0f)

namespace ash
{

inline stx::Option<stx::Span<vk::PhyDeviceInfo const>>
    select_device(stx::Span<vk::PhyDeviceInfo const> const phy_devices,
                  stx::Span<VkPhysicalDeviceType const> preferred_device_types, vk::Surface const &target_surface)
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
                 !vk::get_surface_presentation_command_queue_support(dev.phy_device, dev.family_properties,
                                                                     target_surface.surface)
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
    task_scheduler{stx::os_allocator, std::chrono::steady_clock::now()}, root_widget{iroot_widget}, widget_system{*root_widget}
{
  ctx.task_scheduler = &task_scheduler;
  ctx.clipboard      = &clipboard;
  ctx.window_manager = &window_manager;
  stx::Vec<char const *> required_device_extensions;

  required_device_extensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME).unwrap();

  stx::Vec<char const *> required_validation_layers;

  if (cfg.enable_validation_layers)
  {
    required_validation_layers.push("VK_LAYER_KHRONOS_validation").unwrap();
  }

  ASH_LOG_INFO(Init, "Initializing Window API");

  root_window = stx::Some(window_manager.create_window(cfg.name.c_str(), cfg.root_window_type, cfg.root_window_create_flags, cfg.root_window_extent));

  ASH_LOG_INFO(Init, "Creating root window");

  stx::Vec required_window_instance_extensions = Window::get_required_instance_extensions();

  stx::Rc vk_instance = vk::create_instance(cfg.name.c_str(), VK_MAKE_VERSION(0, 0, 1), cfg.name.c_str(),
                                            VK_MAKE_VERSION(cfg.version.major, cfg.version.minor, cfg.version.patch),
                                            required_window_instance_extensions, required_validation_layers);

  root_window.value()->attach_surface(vk_instance.share());

  stx::Vec phy_devices = vk::get_all_devices(vk_instance);

  VkPhysicalDeviceType const device_preference[] = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                                                    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
                                                    VK_PHYSICAL_DEVICE_TYPE_CPU};

  ASH_LOG_INFO(Init, "Available Physical Devices:");

  for (vk::PhyDeviceInfo const &device : phy_devices)
  {
    // TODO(lamarrr): log graphics families on devices and other properties
    ASH_LOG_INFO(Init, "Device(name: '{}', ID: {}, type: {})", device.properties.deviceName, device.properties.deviceID, string_VkPhysicalDeviceType(device.properties.deviceType));
  }

  stx::Rc<vk::PhyDeviceInfo *> phy_device =
      stx::rc::make(stx::os_allocator, select_device(phy_devices, device_preference, *root_window.value()->surface.value())
                                           .expect("Unable to find any suitable rendering device")[0]
                                           .copy())
          .unwrap();

  ASH_LOG_INFO(Init, "Selected Physical Device: Device(name: '{}', ID: {}, type: {})", phy_device->properties.deviceName,
               phy_device->properties.deviceID, string_VkPhysicalDeviceType(phy_device->properties.deviceType));

  // we might need multiple command queues, one for data transfer and one for
  // rendering
  f32 queue_priorities[] = {// priority for command queue used for
                            // presentation, rendering, data transfer
                            1};

  stx::Rc graphics_command_queue_family =
      stx::rc::make(stx::os_allocator,
                    vk::get_graphics_command_queue(phy_device).expect("Unable to get graphics command queue"))
          .unwrap();

  // we can accept queue family struct here instead and thus not have to
  // perform extra manual checks
  // the user shouldn't have to touch handles
  VkDeviceQueueCreateInfo command_queue_create_infos[] = {{.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                           .pNext            = nullptr,
                                                           .flags            = 0,
                                                           .queueFamilyIndex = graphics_command_queue_family->index,
                                                           .queueCount       = AS(u32, std::size(queue_priorities)),
                                                           .pQueuePriorities = queue_priorities}};

  VkPhysicalDeviceFeatures required_features{};

  required_features.samplerAnisotropy = VK_TRUE;

  stx::Rc<vk::Device *> device = vk::create_device(phy_device, command_queue_create_infos, required_device_extensions,
                                                   required_validation_layers, required_features);

  stx::Rc<vk::CommandQueue *> xqueue =
      stx::rc::make_inplace<vk::CommandQueue>(stx::os_allocator,
                                              vk::get_command_queue(device, *graphics_command_queue_family.handle, 0)
                                                  .expect("Failed to create graphics command queue"))
          .unwrap();

  queue = stx::Some(xqueue.share());

  root_window.value()->recreate_swapchain(xqueue, DEFAULT_MAX_FRAMES_IN_FLIGHT);
  auto &swp = root_window.value()->surface.value()->swapchain.value();

  ASH_LOG_INFO(Init, "recreated swapchain for logical/window/viewport extent: [{}, {}], physical/surface extent: [{}, {}]",
               swp.window_extent.width, swp.window_extent.height, swp.image_extent.width, swp.image_extent.height);

  renderer.init(xqueue.share(), DEFAULT_MAX_FRAMES_IN_FLIGHT);

  renderer.ctx.rebuild(swp.render_pass, swp.msaa_sample_count);

  manager.init(xqueue.share());

  root_window.value()->on(WindowEvents::CloseRequested,
                          stx::fn::rc::make_unique_functor(stx::os_allocator, [](WindowEvents) { std::exit(0); }).unwrap());

  root_window.value()->on(WindowEvents::Resized | WindowEvents::PixelSizeChanged,
                          stx::fn::rc::make_unique_functor(stx::os_allocator, [](WindowEvents) {
                            ASH_LOG_INFO(Init, "WINDOW RESIZED");
                          }).unwrap());

  root_window.value()->on_mouse_motion(stx::fn::rc::make_unique_static([](MouseMotionEvent) {}));

  root_window.value()->on_mouse_click(stx::fn::rc::make_unique_static([](MouseClickEvent event) {
    if (event.action == MouseAction::Press && event.button == MouseButton::A2)
    {
      std::exit(0);
    }
  }));

  u8 transparent_image_data[] = {0xFF, 0xFF, 0xFF, 0xFF};

  gfx::image transparent_image = manager.add_image(ImageView{.data = transparent_image_data, .extent = {1, 1}, .format = ImageFormat::Rgba}, false);

  ASH_CHECK(transparent_image == 0);

  root_window.value()->set_icon(ImageView{.data = transparent_image_data, .extent = {1, 1}, .format = ImageFormat::Rgba});

  /*
 C:\Users\Basit\OneDrive\Desktop\segoeuiemoji\seguiemj.ttf
C:\Users\Basit\OneDrive\Desktop\adobe-arabic-regular\Adobe
  Arabic Regular\Adobe Arabic Regular.ttf
  C:\Users\Basit\OneDrive\Documents\workspace\oss\ashura-assets\fonts\MaterialIcons-Regular.ttf
      C:\Users\Basit\OneDrive\Desktop\gen-shin-gothic-monospace-bold\Gen
  Shin Gothic Monospace Bold\Gen Shin Gothic Monospace Bold.ttf
  */
  ctx.register_plugin(new VulkanImageManager{manager});
  ctx.register_plugin(new ImageLoader{});

  root_window.value()->on_mouse_click(stx::fn::rc::make_unique_functor(
                                          stx::os_allocator, [this](MouseClickEvent event) { widget_system.events.push_inplace(event).unwrap(); })
                                          .unwrap());

  root_window.value()->on_mouse_motion(stx::fn::rc::make_unique_functor(
                                           stx::os_allocator, [this](MouseMotionEvent event) { widget_system.events.push_inplace(event).unwrap(); })
                                           .unwrap());

  root_window.value()->on(WindowEvents::All, stx::fn::rc::make_unique_functor(stx::os_allocator, [this](WindowEvents events) {
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
    stx::Result result = load_font_from_file(spec.path);
    TIMER_END(FontLoadFromFile, "Rendering Font");

    if (result.is_ok())
    {
      TIMER_BEGIN(FontGlyphRender);
      ASH_LOG_INFO(Init, "Loaded font: {} from file: {}", spec.name.view(), spec.path.view());
      auto [atlas, image_buffer] = render_font_atlas(*result.value(), spec.atlas_font_height, spec.max_atlas_extent);
      atlas.texture              = manager.add_image(image_buffer, false);
      stx::Option<FontStrokeAtlas> stroke_atlas_o;
      TIMER_END(FontGlyphRender, "Rendering Font");

      if (spec.stroke_thickness != 0)
      {
        TIMER_BEGIN(FontStrokeRender);
        auto [stroke_atlas, stroke_image_buffer] = render_font_stroke_atlas(*result.value(), spec.atlas_font_height, spec.stroke_thickness, spec.max_atlas_extent);
        stroke_atlas.texture                     = manager.add_image(stroke_image_buffer, false);
        stroke_atlas_o                           = stx::Some(std::move(stroke_atlas));
        TIMER_END(FontStrokeRender, "Rendering Font");
      }

      font_bundle.push(BundledFont{.name = spec.name.copy(stx::os_allocator).unwrap(), .font = std::move(result.value()), .atlas = std::move(atlas), .stroke_atlas = std::move(stroke_atlas_o)}).unwrap();
    }
    else
    {
      ASH_LOG_ERR(Init, "Failed to load font: {} from file: {}, error: {}", spec.name.view(), spec.path.view(), AS(i64, result.err()));
    }
  }

  TIMER_END(AllFontLoad, "All Font Rendering");

  ctx.font_bundle = font_bundle;

  // TODO(lamarrr): attach debug widgets: FPS stats, memory usage, etc
  widget_system.on_startup(ctx);

  for (Plugin *plugin : ctx.plugins)
  {
    plugin->on_startup(ctx);
    ASH_LOG_INFO(Context, "Initialized plugin: {} (type: {})", plugin->get_name(), typeid(*plugin).name());
  }
}

void Engine::tick(std::chrono::nanoseconds interval)
{
  // poll events to make the window not be marked as unresponsive.
  // poll events from SDL's event queue until there are none left.
  //
  task_scheduler.tick(interval);
  do
  {
  } while (ctx.poll_events());

  // TODO(lamarrr): tick plugins
  // root_window->tick(interval);
  ctx.tick(interval);
  widget_system.pump_events(ctx);
  widget_system.tick_widgets(ctx, interval);
  // new widgets could have been added
  widget_system.assign_ids(ctx);
  manager.flush_deletes();
  manager.submit_uploads();

  auto record_draw_commands = [&]() {
    VkExtent2D extent = root_window.value()->surface.value()->swapchain.value().window_extent;
    vec2       viewport_extent{AS(f32, extent.width), AS(f32, extent.height)};
    canvas.restart(viewport_extent);
    widget_system.perform_widget_layout(ctx, viewport_extent);
    widget_system.rebuild_draw_entries(ctx);
    widget_system.draw_widgets(ctx, viewport_extent, canvas, mat4::identity());
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
        ASH_LOG_INFO(Window, "recreated swapchain for logical/window/viewport extent: [{}, {}], "
                             "physical/surface extent: [{}, {}]",
                     swp.window_extent.width, swp.window_extent.height, swp.image_extent.width,
                     swp.image_extent.height);
        renderer.ctx.rebuild(swp.render_pass, swp.msaa_sample_count);
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
                      swapchain.render_fences[swapchain.frame], swapchain.image_acquisition_semaphores[swapchain.frame],
                      swapchain.render_semaphores[swapchain.frame], swapchain.render_pass,
                      swapchain.framebuffers[swapchain_image_index], draw_list.cmds, draw_list.vertices, draw_list.indices,
                      manager);

      swapchain_state = root_window.value()->present(queue.value()->info.queue, swapchain_image_index);

      // the frame semaphores and synchronization primitives are still used even
      // if an error is returned
      swapchain.frame = (swapchain.frame + 1) % swapchain.max_nframes_in_flight;
    }
    else
    {
      swapchain_state = SwapChainState::Ok;
    }
  } while (swapchain_state != SwapChainState::Ok);
}

}        // namespace ash
