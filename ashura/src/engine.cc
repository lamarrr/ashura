#include "ashura/engine.h"

#include <chrono>
#include <fstream>

#include "ashura/animation.h"
#include "ashura/asset_bundle.h"
#include "ashura/canvas.h"
#include "ashura/palletes.h"
#include "ashura/sdl_utils.h"
#include "ashura/shaders.h"
#include "ashura/vulkan_context.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace ash
{

namespace impl
{

static stx::Rc<spdlog::logger *> make_multi_threaded_logger(std::string name, std::string file_path)
{
  stx::Vec<spdlog::sink_ptr> sinks{stx::os_allocator};
  sinks.push(std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::move(file_path))).unwrap();

  sinks.push(std::make_shared<spdlog::sinks::stdout_color_sink_mt>()).unwrap();

  return stx::rc::make_inplace<spdlog::logger>(stx::os_allocator, std::move(name), sinks.begin(), sinks.end()).unwrap();
}
}        // namespace impl

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
  widget_context.task_scheduler = &task_scheduler;
  stx::Vec<char const *> required_device_extensions{stx::os_allocator};

  required_device_extensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME).unwrap();

  stx::Vec<char const *> required_validation_layers{stx::os_allocator};

  if (cfg.enable_validation_layers)
  {
    required_validation_layers.push("VK_LAYER_KHRONOS_validation").unwrap();
  }

  logger = stx::Some(impl::make_multi_threaded_logger("ashura", cfg.log_file.c_str()));

  auto &xlogger = *logger.value().handle;

  xlogger.info("Initializing Window API");

  window_api = stx::Some(stx::rc::make_inplace<WindowApi>(stx::os_allocator).unwrap());

  xlogger.info("Creating root window");

  window = stx::Some(create_window(window_api.value().share(), cfg.window_config.copy()));

  stx::Vec window_required_instance_extensions = window_api.value()->get_required_instance_extensions();

  stx::Rc vk_instance = vk::create_instance(cfg.name.c_str(), VK_MAKE_VERSION(0, 0, 1), cfg.name.c_str(),
                                            VK_MAKE_VERSION(cfg.version.major, cfg.version.minor, cfg.version.patch),
                                            window_required_instance_extensions, required_validation_layers, xlogger);

  window.value()->attach_surface(vk_instance.share());

  stx::Vec phy_devices = vk::get_all_devices(vk_instance);

  VkPhysicalDeviceType const device_preference[] = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                                                    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
                                                    VK_PHYSICAL_DEVICE_TYPE_CPU};

  xlogger.info("Available Physical Devices:");

  for (vk::PhyDeviceInfo const &device : phy_devices)
  {
    // TODO(lamarrr): log graphics families on devices and other properties
    xlogger.info("Device(name: '{}', ID: {}, type: {})", device.properties.deviceName, device.properties.deviceID,
                 string_VkPhysicalDeviceType(device.properties.deviceType));
  }

  stx::Rc<vk::PhyDeviceInfo *> phy_device =
      stx::rc::make(stx::os_allocator, select_device(phy_devices, device_preference, *window.value()->surface.value().handle)
                                           .expect("Unable to find any suitable rendering device")[0]
                                           .copy())
          .unwrap();

  xlogger.info("Selected Physical Device: Device(name: '{}', ID: {}, type: {})", phy_device->properties.deviceName,
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
                                                   required_validation_layers, required_features, xlogger);

  stx::Rc<vk::CommandQueue *> xqueue =
      stx::rc::make_inplace<vk::CommandQueue>(stx::os_allocator,
                                              vk::get_command_queue(device, *graphics_command_queue_family.handle, 0)
                                                  .expect("Failed to create graphics command queue"))
          .unwrap();

  queue = stx::Some(xqueue.share());

  window.value()->recreate_swapchain(xqueue, DEFAULT_MAX_FRAMES_IN_FLIGHT, xlogger);
  auto &swp = window.value()->surface.value()->swapchain.value();

  xlogger.info("recreated swapchain for logical/window/viewport extent: [{}, {}], "
               "physical/surface extent: [{}, {}]",
               swp.window_extent.width, swp.window_extent.height, swp.image_extent.width, swp.image_extent.height);

  renderer.init(xqueue.share(), DEFAULT_MAX_FRAMES_IN_FLIGHT);

  renderer.ctx.rebuild(swp.render_pass, swp.msaa_sample_count);

  manager.init(xqueue.share());

  window.value()->on(WindowEvents::CloseRequested,
                     stx::fn::rc::make_unique_functor(stx::os_allocator, [](WindowEvents) { std::exit(0); }).unwrap());

  window.value()->on(WindowEvents::Resized | WindowEvents::PixelSizeChanged,
                     stx::fn::rc::make_unique_functor(stx::os_allocator, [this](WindowEvents) {
                       logger.value()->info("WINDOW RESIZED");
                     }).unwrap());

  window.value()->mouse_motion_listeners.push(stx::fn::rc::make_unique_static([](MouseMotionEvent) {})).unwrap();

  window.value()
      ->mouse_click_listeners
      .push(stx::fn::rc::make_unique_static([](MouseClickEvent event) {
        std::cout << "clicks: " << event.clicks << std::endl;
        if (event.action == MouseAction::Press && event.button == MouseButton::A2)
          std::exit(0);
      }))
      .unwrap();

  u8 transparent_image_data[] = {0xFF, 0xFF, 0xFF, 0xFF};

  gfx::image transparent_image = manager.add(ImageView{.data = transparent_image_data, .extent = {1, 1}, .format = ImageFormat::Rgba});

  ASH_CHECK(transparent_image == 0);

  /*
  gfx::CachedFont* font;
  gfx::image img;
    font = new gfx::CachedFont[]{
        vk::cache_font(
            upload_context, image_bundle,
            load_font_from_file(
                R"(C:\Users\Basit\OneDrive\Documents\workspace\oss\ashura\assets\fonts\JetBrainsMono-Regular.ttf)"_str)
                .unwrap(),
            26),
        vk::cache_font(
            upload_context, image_bundle,
            load_font_from_file(
                R"(C:\Users\Basit\OneDrive\Desktop\segoeuiemoji\seguiemj.ttf)"_str)
                .unwrap(),
            50),
        vk::cache_font(
            upload_context, image_bundle,
            load_font_from_file(
                R"(C:\Users\Basit\OneDrive\Desktop\adobe-arabic-regular\Adobe
  Arabic Regular\Adobe Arabic Regular.ttf)"_str) .unwrap(), 26), vk::cache_font(
            upload_context, image_bundle,
            load_font_from_file(
                R"(C:\Users\Basit\OneDrive\Documents\workspace\oss\ashura-assets\fonts\MaterialIcons-Regular.ttf)"_str)
                .unwrap(),
            50),
        vk::cache_font(
            upload_context, image_bundle,
            load_font_from_file(
                R"(C:\Users\Basit\OneDrive\Desktop\gen-shin-gothic-monospace-bold\Gen
  Shin Gothic Monospace Bold\Gen Shin Gothic Monospace Bold.ttf)"_str)
                .unwrap(),
            50)};
  */
  widget_context.register_plugin(new VulkanImageBundle{manager});

  window.value()
      ->mouse_click_listeners
      .push(stx::fn::rc::make_unique_functor(
                stx::os_allocator, [this](MouseClickEvent event) { widget_system.events.push_inplace(event).unwrap(); })
                .unwrap())
      .unwrap();

  window.value()
      ->mouse_motion_listeners
      .push(stx::fn::rc::make_unique_functor(
                stx::os_allocator, [this](MouseMotionEvent event) { widget_system.events.push_inplace(event).unwrap(); })
                .unwrap())
      .unwrap();

  window.value()->on(WindowEvents::All, stx::fn::rc::make_unique_functor(stx::os_allocator, [this](WindowEvents events) {
                                          if ((events & WindowEvents::MouseLeave) != WindowEvents::None)
                                          {
                                            widget_system.events.push_inplace(events).unwrap();
                                          }
                                        }).unwrap());

  // TODO(lamarrr): attach debug widgets: FPS stats, memory usage, etc
  widget_system.launch(widget_context);
}

void Engine::tick(std::chrono::nanoseconds interval)
{
  // poll events to make the window not be marked as unresponsive.
  // poll events from SDL's event queue until there are none left.
  //
  task_scheduler.tick(interval);
  do
  {
  } while (window_api.value()->poll_events());

  window.value()->tick(interval);
  widget_system.pump_events(widget_context);
  widget_system.tick_widgets(widget_context, interval);
  // new widgets could have been added
  widget_system.assign_ids();
  manager.flush_deletes();
  manager.flush_uploads();

  auto record_draw_commands = [&]() {
    VkExtent2D extent = window.value()->surface.value()->swapchain.value().window_extent;
    vec2       viewport_extent{AS(f32, extent.width), AS(f32, extent.height)};
    canvas.restart(viewport_extent);
    widget_system.perform_widget_layout(viewport_extent);
    widget_system.rebuild_draw_entries();
    widget_system.draw_widgets(widget_context, canvas);
  };

  // only record if swapchain visible,
  // if extent is zero, do not present, record, or recreate swapchain, or
  // acquire swapchain image, or submit to renderer
  // do not increase the frame flight indices as well since the sync primitves
  // aren't used
  if (!window.value()->surface.value()->is_zero_sized_swapchain)
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
      window.value()->recreate_swapchain(queue.value(), DEFAULT_MAX_FRAMES_IN_FLIGHT, *logger.value().handle);
      // TODO(lamarrr): fix
      if (!window.value()->surface.value()->is_zero_sized_swapchain)
      {
        auto &swp = window.value()->surface.value()->swapchain.value();
        logger.value()->info("recreated swapchain for logical/window/viewport extent: [{}, {}], "
                             "physical/surface extent: [{}, {}]",
                             swp.window_extent.width, swp.window_extent.height, swp.image_extent.width,
                             swp.image_extent.height);
        renderer.ctx.rebuild(swp.render_pass, swp.msaa_sample_count);
        record_draw_commands();
      }
    }

    if (!window.value()->surface.value()->is_zero_sized_swapchain)
    {
      vk::SwapChain &swapchain = window.value()->surface.value()->swapchain.value();

      auto [state, swapchain_image_index] = window.value()->acquire_image();

      swapchain_state = state;

      if (swapchain_state != SwapChainState::Ok)
      {
        continue;
      }

      gfx::DrawList const &draw_list = canvas.draw_list;

      renderer.submit(swapchain.window_extent, swapchain.image_extent, swapchain_image_index, swapchain.frame,
                      swapchain.render_fences[swapchain.frame], swapchain.image_acquisition_semaphores[swapchain.frame],
                      swapchain.render_semaphores[swapchain.frame], swapchain.render_pass,
                      swapchain.framebuffers[swapchain_image_index], draw_list.cmds, draw_list.vertices, draw_list.indices,
                      manager);

      swapchain_state = window.value()->present(queue.value()->info.queue, swapchain_image_index);

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
