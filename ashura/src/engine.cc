#include "ashura/engine.h"

#include "ashura/canvas.h"
#include "ashura/sample_image.h"
#include "ashura/sdl_utils.h"
#include "ashura/shaders.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace asr {

namespace impl {

// TODO(lamarrr): take a quick look at UE log file content and structure
static stx::Rc<spdlog::logger*> make_multi_threaded_logger(
    std::string name, std::string file_path) {
  stx::Vec<spdlog::sink_ptr> sinks{stx::os_allocator};
  /* sinks
       .push(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
           std::move(file_path)))
       .unwrap();
 */
  sinks.push(std::make_shared<spdlog::sinks::stdout_color_sink_mt>()).unwrap();

  return stx::rc::make_inplace<spdlog::logger>(
             stx::os_allocator, std::move(name), sinks.begin(), sinks.end())
      .unwrap();
}
}  // namespace impl

inline stx::Option<stx::Span<vk::PhyDeviceInfo const>> select_device(
    stx::Span<vk::PhyDeviceInfo const> const phy_devices,
    stx::Span<VkPhysicalDeviceType const> preferred_device_types,
    vk::Surface const& target_surface) {
  for (VkPhysicalDeviceType type : preferred_device_types) {
    if (stx::Span selected =
            phy_devices.which([&](vk::PhyDeviceInfo const& dev) -> bool {
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
                          dev.phy_device, dev.family_properties,
                          target_surface.surface)
                          .span()
                          .find(true)
                          .is_empty();
            });
        !selected.is_empty()) {
      return stx::Some(std::move(selected));
    }
  }

  return stx::None;
}

Engine::Engine(AppConfig const& cfg) {
  stx::Vec<char const*> required_device_extensions{stx::os_allocator};

  required_device_extensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME).unwrap();

  stx::Vec<char const*> required_validation_layers{stx::os_allocator};

  if (cfg.enable_validation_layers)
    required_validation_layers.push("VK_LAYER_KHRONOS_validation").unwrap();

  logger = stx::Some(
      impl::make_multi_threaded_logger("ashura", cfg.log_file.c_str()));

  auto& xlogger = *logger.value().handle;

  xlogger.info("Initializing Window API");

  window_api =
      stx::Some(stx::rc::make_inplace<WindowApi>(stx::os_allocator).unwrap());

  xlogger.info("Initialized Window API");
  xlogger.info("Creating root window");

  window = stx::Some(
      create_window(window_api.value().share(), cfg.window_config.copy()));

  xlogger.info("Created root window");

  stx::Vec window_required_instance_extensions =
      window.value().handle->get_required_instance_extensions();

  // TODO(lamarrr): check for validation layers requirement
  stx::Rc<vk::Instance*> vk_instance = vk::create_instance(
      cfg.name.c_str(), VK_MAKE_VERSION(0, 0, 1), cfg.name.c_str(),
      VK_MAKE_VERSION(cfg.version.major, cfg.version.minor, cfg.version.patch),
      window_required_instance_extensions, required_validation_layers);

  window.value().handle->attach_surface(vk_instance.share());

  stx::Vec phy_devices = vk::get_all_devices(vk_instance);

  VkPhysicalDeviceType const device_preference[] = {
      VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
      VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
      VK_PHYSICAL_DEVICE_TYPE_CPU};

  xlogger.info("Available Physical Devices:");

  for (vk::PhyDeviceInfo const& device : phy_devices) {
    xlogger.info("\t{}", vk::format(device));
    // TODO(lamarrr): log graphics families on devices and other properties
  }

  stx::Rc<vk::PhyDeviceInfo*> phy_device =
      stx::rc::make(
          stx::os_allocator,
          select_device(phy_devices, device_preference,
                        *window.value().handle->surface_.value().handle)
              .expect("Unable to find any suitable rendering device")[0]
              .copy())
          .unwrap();

  xlogger.info("Selected Physical Device: {}", vk::format(*phy_device.handle));

  // we might need multiple command queues, one for data transfer and one for
  // rendering
  f32 queue_priorities[] = {// priority for command queue used for
                            // presentation, rendering, data transfer
                            1.0f};

  stx::Rc graphics_command_queue_family =
      stx::rc::make(stx::os_allocator,
                    vk::get_graphics_command_queue(phy_device)
                        .expect("Unable to get graphics command queue"))
          .unwrap();

  // we can accept queue family struct here instead and thus not have to
  // perform extra manual checks
  // the user shouldn't have to touch handles
  VkDeviceQueueCreateInfo command_queue_create_infos[] = {
      VkDeviceQueueCreateInfo{
          .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .queueFamilyIndex = graphics_command_queue_family.handle->index,
          .queueCount = AS_U32(std::size(queue_priorities)),
          .pQueuePriorities = queue_priorities}};

  VkPhysicalDeviceFeatures required_features{};

  required_features.samplerAnisotropy = VK_TRUE;

  stx::Rc<vk::Device*> device = vk::create_device(
      phy_device, command_queue_create_infos, required_device_extensions,
      required_validation_layers, required_features);

  stx::Rc<vk::CommandQueue*> xqueue =
      stx::rc::make_inplace<vk::CommandQueue>(
          stx::os_allocator,
          vk::get_command_queue(device, *graphics_command_queue_family.handle,
                                0)
              .expect("Failed to create graphics command queue"))
          .unwrap();

  queue = stx::Some(xqueue.share());

  window.value().handle->recreate_swapchain(xqueue);

  canvas_context = stx::Some(stx::rc::make_inplace<gfx::CanvasContext>(
                                 stx::os_allocator, xqueue.share())
                                 .unwrap());

  canvas_context.value().handle->recording_context.on_swapchain_changed(
      queue.value().handle->device.handle->device,
      window.value().handle->surface_.value().handle->swapchain.value());

  window.value().handle->on(
      WindowEvent::Resized,
      stx::fn::rc::make_unique_functor(stx::os_allocator, []() {
        ASR_LOG("resized");
      }).unwrap());

  window.value().handle->mouse_motion_listener =
      stx::fn::rc::make_unique_static(
          [](MouseMotionEvent const&) { ASR_LOG("mouse motion detected"); });

  u32 const transparent_image_data[1] = {0x00000000};
  // TODO(lamarrr): fill with zeros
  // auto transparent_image =
  // vk::upload_rgba_image(xqueue, 1, 1, transparent_image_data);

  auto transparent_image =
      canvas_context.value().handle->recording_context.upload_image(
          queue.value(), ImageDims{.width = 1920, .height = 1080, .nchannels = 4},
          sample_image);

  auto sampler = vk::create_image_sampler(transparent_image);

  canvas = stx::Some(gfx::Canvas{{0.0f, 0.0f}, sampler});

  VkDevice dev = queue.value().handle->device.handle->device;

  window.value().handle->on(
      WindowEvent::Close,
      stx::fn::rc::make_unique_functor(stx::os_allocator, []() {
        std::exit(0);
      }).unwrap());
};

void Engine::tick(std::chrono::nanoseconds interval) {
  // TODO(lamarrr): try getting window extent on each frame instead
  window.value().handle->tick(interval);

  auto draw_content = [&]() {
    VkExtent2D extent = window.value()
                            .handle->surface_.value()
                            .handle->swapchain.value()
                            .window_extent;

    gfx::Canvas& c = canvas.value();

    c.restart(vec2{1920, 1080});
    c.brush.color = colors::TRANSPARENT;
    c.clear();
    //c.draw_rect({0, 0}, {1920, 1080});
    c.brush.color = colors::GREEN;
    c.draw_rect({.25 * 1920, .25 * 1080}, {.5 * 1920, .5 * 1080});
  };

  draw_content();
  // only try to present if the pipeline has new changes or window was
  // resized

  // TODO(lamarrr): make presentation happen after recreation, for the first
  // iteration. and remove the created swapchain in the init method

  // only try to recreate swapchain if the present swapchain can't be used for
  // presentation

  WindowSwapchainDiff swapchain_diff = WindowSwapchainDiff::None;

  do {
    if (swapchain_diff != WindowSwapchainDiff::None) {
      window.value().handle->recreate_swapchain(queue.value());
      canvas_context.value().handle->recording_context.on_swapchain_changed(
          queue.value().handle->device.handle->device,
          window.value().handle->surface_.value().handle->swapchain.value());

      draw_content();
    }

    vk::SwapChain& swapchain =
        window.value().handle->surface_.value().handle->swapchain.value();

    auto [diff, next_swapchain_image_index] =
        window.value().handle->acquire_image();

    swapchain_diff = diff;

    if (swapchain_diff != WindowSwapchainDiff::None) {
      continue;
    }

    ASR_VK_CHECK(vkWaitForFences(
        swapchain.queue.handle->device.handle->device, 1,
        &swapchain.image_acquisition_fences[swapchain.next_frame_flight_index],
        VK_TRUE, COMMAND_TIMEOUT));

    canvas_context.value().handle->submit(
        window.value().handle->surface_.value().handle->swapchain.value(),
        next_swapchain_image_index, canvas.value().draw_list);

    swapchain_diff = window.value().handle->present(next_swapchain_image_index);

    // the frame semaphores and synchronization primitives are still used even
    // if an error is returned
    swapchain.next_frame_flight_index =
        (swapchain.next_frame_flight_index + 1) %
        vk::SwapChain::MAX_FRAMES_INFLIGHT;

  } while (swapchain_diff != WindowSwapchainDiff::None);

  // poll events to make the window not be marked as unresponsive.
  // we also poll events from SDL's event queue until there are none left.
  //
  // any missed event should be rolled over to the next tick()
  do {
  } while (window_api.value().handle->poll_events());

  //   pipeline->dispatch_events(
  //   window->handle.handle->event_queue.mouse_button_events,
  //   window->handle.handle->event_queue.window_events);

  //   bool window_extent_changed =
  //   any_eq(window.value().handle->, WindowEvent::SizeChanged);

  // TODO(lamarrr): ???
  //   window->handle.handle->event_queue.clear();
}

}  // namespace asr
