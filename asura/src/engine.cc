#include "asr/engine.h"

namespace asr {

namespace impl {

// TODO(lamarrr): take a quick look at UE log file content and structure
// Valkyrie.App
static stx::Unique<spdlog::logger*> make_multi_threaded_logger(
    std::string name, std::string file_path) {
  stx::Vec<spdlog::sink_ptr> sinks;
  sinks
      .push(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
          std::move(file_path)))
      .unwrap();

  sinks.push(std::make_shared<spdlog::sinks::stdout_color_sink_mt>()).unwrap();

  return stx::rc::make_unique_inplace<spdlog::logger>(
             stx::os_allocaor, std::move(name), sinks.begin(), sinks.end())
      .unwrap();
}
}  // namespace impl

static stx::Option<vk::PhysDevice> select_device(
    stx::Span<vk::PhysDevice const> const physical_devices,
    stx::Span<VkPhysicalDeviceType const> preferred_device_types,
    WindowSurface const& target_surface) {
  for (auto type : preferred_device_types) {
    auto selected_device_it = std::find_if(
        physical_devices.begin(), physical_devices.end(),
        [&](vk::PhysDevice const& dev) -> bool {
          return dev.info.properties.deviceType == type &&
                 // can use shaders (fragment and vertex)
                 dev.has_geometry_shader() &&
                 // has graphics command queue for rendering commands
                 dev.has_graphics_command_queue_family() &&
                 // has data transfer command queue for uploading textures
                 // or data
                 dev.has_transfer_command_queue_family() &&
                 // can be used for presenting to a specific surface
                 any_true(vk::get_surface_presentation_command_queue_support(
                     dev.info.phys_device, dev.info.family_properties,
                     target_surface.handle->surface));
        });
    if (selected_device_it != physical_devices.end()) {
      return stx::Some(vk::PhysDevice{*selected_device_it});
    }
  }

  return stx::None;
}

Engine::Engine(AppConfig const& cfg) {
  stx::Vec<char const*> required_device_extensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  stx::Vec<char const*> required_validation_layers;

  if (cfg.enable_validation_layers)
    required_validation_layers.push_back("VK_LAYER_KHRONOS_validation");

  logger_ = impl::make_multi_threaded_logger("Asura", cfg.log_file.c_str());

  void* logger;

  logger->info("Initializing Window API");

  window_api_ = stx::Some(
      stx::rc::make_rc_inplace<WindowApi>(stx::os_allocator).unwrap());

  logger->info("Initialized Window API");
  logger->info("Creating root window");

  window_api;

  root_window_ = stx::Some(create_window(window_api_->share(), ));

  logger->info("Created root window");

  stx::Vec window_required_instance_extensions =
      window->handle.handle->get_required_instance_extensions();

  // TODO(lamarrr): check for validation layers requirement
  vk::Instance vk_instance = vk::Instance::create(
      cfg.name.c_str(), VK_MAKE_VERSION(0, 0, 1), engine_cfg.name.c_str(),
      VK_MAKE_VERSION(engine_cfg.version.major, engine_cfg.version.minor,
                      engine_cfg.version.patch),
      window_required_instance_extensions, required_validation_layers);

  window->attach_surface(vk_instance);

  stx::Vec phys_devices = vk::PhysDevice::get_all(vk_instance);

  VkPhysicalDeviceType const device_preference[] = {
      VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
      VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
      VK_PHYSICAL_DEVICE_TYPE_CPU};

  logger->info("Available Physical Devices:");

  for (vk::PhysDevice const& device : phys_devices) {
    logger->info("\t{}", vk::format(device));
    // TODO(lamarrr): log graphics families on devices and other properties
  }

  vk::PhysDevice phys_device =
      select_device(phys_devices, device_preference,
                    window->handle.handle->surface)
          .expect("Unable to find any suitable rendering device");

  logger->info("Selected Physical Device: {}", vk::format(phys_device));

  // we might need multiple command queues, one for data transfer and one for
  // rendering
  float const queue_priorities[] = {// priority for command queue used for
                                    // presentation, rendering, data transfer
                                    1.0f};

  vk::CommandQueueFamily graphic_command_queue_family =
      vk::CommandQueueFamily::get_graphics(phys_device)
          .expect("Unable to get graphics command queue");

  // we can accept queue family struct here instead and thus not have to
  // perform extra manual checks
  // the user shouldn't have to touch handles
  VkDeviceQueueCreateInfo const command_queue_create_infos[] = {
      vk::make_command_queue_create_info(
          graphic_command_queue_family.info.index, queue_priorities)};

  VkPhysicalDeviceFeatures required_features{};

  vk::Device device = vk::Device::create(
      phys_device, command_queue_create_infos, required_device_extensions,
      required_validation_layers, required_features);

  vk::CommandQueue vk_graphics_command_queue = vk::CommandQueue{
      vk::CommandQueue::get(device, graphic_command_queue_family, 0)
          .expect("Failed to create graphics command queue")};

  window->handle.handle->recreate_swapchain(vk_render_context);
}

}  // namespace asr
