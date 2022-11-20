#include "ashura/engine.h"

#include "ashura/canvas.h"
#include "ashura/render_object.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace asr {

namespace impl {

// TODO(lamarrr): take a quick look at UE log file content and structure
// Valkyrie.App
static stx::Rc<spdlog::logger*> make_multi_threaded_logger(
    std::string name, std::string file_path) {
  stx::Vec<spdlog::sink_ptr> sinks{stx::os_allocator};
  sinks
      .push(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
          std::move(file_path)))
      .unwrap();

  sinks.push(std::make_shared<spdlog::sinks::stdout_color_sink_mt>()).unwrap();

  return stx::rc::make_inplace<spdlog::logger>(
             stx::os_allocator, std::move(name), sinks.begin(), sinks.end())
      .unwrap();
}
}  // namespace impl

static stx::Option<stx::Span<vk::PhyDeviceInfo const>> select_device(
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
                     any_true(
                         vk::get_surface_presentation_command_queue_support(
                             dev.phy_device, dev.family_properties,
                             target_surface.surface));
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

  logger_ = stx::Some(
      impl::make_multi_threaded_logger("ashura", cfg.log_file.c_str()));

  auto& logger = *logger_.value().handle;

  logger.info("Initializing Window API");

  window_api_ =
      stx::Some(stx::rc::make_inplace<WindowApi>(stx::os_allocator).unwrap());

  logger.info("Initialized Window API");
  logger.info("Creating root window");

  root_window_ = stx::Some(
      create_window(window_api_.value().share(), cfg.window_config.copy()));

  logger.info("Created root window");

  stx::Vec window_required_instance_extensions =
      root_window_.value().handle->get_required_instance_extensions();

  // TODO(lamarrr): check for validation layers requirement
  stx::Rc<vk::Instance*> vk_instance = vk::create_instance(
      cfg.name.c_str(), VK_MAKE_VERSION(0, 0, 1), cfg.name.c_str(),
      VK_MAKE_VERSION(cfg.version.major, cfg.version.minor, cfg.version.patch),
      window_required_instance_extensions, required_validation_layers);

  root_window_.value().handle->attach_surface(vk_instance.share());

  stx::Vec phy_devices = vk::get_all_devices(vk_instance);

  VkPhysicalDeviceType const device_preference[] = {
      VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
      VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
      VK_PHYSICAL_DEVICE_TYPE_CPU};

  logger.info("Available Physical Devices:");

  for (vk::PhyDeviceInfo const& device : phy_devices) {
    logger.info("\t{}", vk::format(device));
    // TODO(lamarrr): log graphics families on devices and other properties
  }

  stx::Rc<vk::PhyDeviceInfo*> phy_device =
      stx::rc::make(
          stx::os_allocator,
          select_device(phy_devices, device_preference,
                        *root_window_.value().handle->surface_.value().handle)
              .expect("Unable to find any suitable rendering device")[0]
              .copy())
          .unwrap();

  logger.info("Selected Physical Device: {}", vk::format(*phy_device.handle));

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
          .flags = 0,
          .pNext = nullptr,
          .pQueuePriorities = queue_priorities,
          .queueCount = std::size(queue_priorities),
          .queueFamilyIndex = graphics_command_queue_family.handle->index,
          .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO}};

  VkPhysicalDeviceFeatures required_features{};

  stx::Rc<vk::Device*> device = vk::create_device(
      phy_device, command_queue_create_infos, required_device_extensions,
      required_validation_layers, required_features);

  stx::Rc<vk::CommandQueue*> graphics_command_queue =
      stx::rc::make_inplace<vk::CommandQueue>(
          stx::os_allocator,
          vk::get_command_queue(device, *graphics_command_queue_family.handle,
                                0)
              .expect("Failed to create graphics command queue"))
          .unwrap();

  root_window_.value().handle->recreate_swapchain(graphics_command_queue);

  root_window_.value().handle->on(
      WindowEvent::Resized,
      stx::fn::rc::make_unique_functor(stx::os_allocator, []() {
        ASR_LOG("resized");
      }).unwrap());

  root_window_.value().handle->mouse_motion_listener =
      stx::fn::rc::make_unique_static(
          [](MouseMotionEvent const&) { ASR_LOG("mouse motion detected"); });
};

void Engine::tick(std::chrono::nanoseconds interval) {
  window_api_.value().handle->poll_events();
  root_window_.value().handle->tick(interval);
}

}  // namespace asr
