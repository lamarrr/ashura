#include "vlk/ui/app.h"

#include "vlk/ui/pipeline.h"
#include "vlk/ui/vulkan.h"
#include "vlk/ui/window.h"
#include "vlk/ui/window_handle.h"

namespace vlk {
namespace ui {

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

static constexpr char const* const required_device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

static constexpr char const* required_validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"};

App::~App() = default;

void App::init() {
  window_api = std::unique_ptr<WindowApi>{new WindowApi{}};
  window_api->init();

  WindowCfg win_cfg;
  win_cfg.extent = cfg.extent;
  win_cfg.maximized = cfg.maximized;
  win_cfg.resizable = cfg.resizable;
  win_cfg.title = cfg.resizable;

  window =
      std::unique_ptr<Window>{new Window{Window::create(*window_api, win_cfg)}};
  std::vector window_required_instance_extensions =
      window->handle->get_required_instance_extensions();

  // TODO(lamarrr): check for validation layers requirement
  vk_instance =
      std::unique_ptr<vk::Instance>{new vk::Instance{vk::Instance::create(
          cfg.name, VK_MAKE_VERSION(0, 0, 1), engine_cfg.name,
          VK_MAKE_VERSION(engine_cfg.version.major, engine_cfg.version.minor,
                          engine_cfg.version.patch),
          window_required_instance_extensions, required_validation_layers)}};

  window->attach_surface(*vk_instance);

  std::vector phys_devices = vk::PhysDevice::get_all(*vk_instance);

  VkPhysicalDeviceType const device_preference[] = {
      VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
      VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
      VK_PHYSICAL_DEVICE_TYPE_CPU};

  VLK_LOG("Available Physical Devices:");

  for (vk::PhysDevice const& device : phys_devices) {
    VLK_LOG("\t{}", device.format());
    // TODO(lamarrr): log graphics families on devices and other properties
  }

  vk::PhysDevice phys_device =
      select_device(phys_devices, device_preference, window->handle->surface)
          .expect("Unable to find any suitable rendering device");

  VLK_LOG("Selected Physical Device: {}", phys_device.format());

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

  vk_graphics_command_queue =
      std::unique_ptr<vk::CommandQueue>{new vk::CommandQueue{
          vk::CommandQueue::get(device, graphic_command_queue_family, 0)
              .expect("Failed to create graphics command queue")}};

  GrVkBackendContext sk_vk_context{};

  GrVkExtensions sk_extensions_cache{};
  sk_vk_context.fVkExtensions = &sk_extensions_cache;
  sk_vk_context.fInstance = vk_instance->handle->instance;
  sk_vk_context.fPhysicalDevice = phys_device.info.phys_device;
  sk_vk_context.fDevice = device.handle->device;
  sk_vk_context.fQueue = vk_graphics_command_queue->info.queue;
  sk_vk_context.fGraphicsQueueIndex = vk_graphics_command_queue->info.index;
  sk_vk_context.fMaxAPIVersion = VK_API_VERSION_1_1;
  sk_vk_context.fDeviceFeatures = &phys_device.info.features;
  // TODO(lamarrr): vk_context.fMemoryAllocator, see: GrVkAMDMemoryAllocator
  sk_vk_context.fGetProc = [](char const* proc_name, VkInstance instance,
                              VkDevice device) {
    VLK_ENSURE(instance == nullptr || device == nullptr);
    VLK_ENSURE(!(instance != nullptr && device != nullptr));
    if (device != nullptr) {
      // get process address
      return vkGetDeviceProcAddr(device, proc_name);
    } else {
      return vkGetInstanceProcAddr(instance, proc_name);
    }
  };

  // TODO(lamarrr): we need to take care of lifetime interactions here:
  // - direct context, render context and the likes, so we can also pass to
  // swapchain and the likes and not have to enforce ordering of struct members
  // -
  // -

  sk_sp<GrDirectContext> sk_direct_context =
      GrDirectContext::MakeVulkan(sk_vk_context);

  RenderContext{
      RasterTarget::Gpu,
      // sk_direct_context->asDirectContext()
  };

  // TODO(lamarrr): initial extent?, RenderContext
  pipeline = std::unique_ptr<Pipeline>{new Pipeline{*root_widget}};

  // resize viewport
}

}  // namespace ui
}  // namespace vlk
