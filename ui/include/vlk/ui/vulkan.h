#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "stx/option.h"
#include "stx/span.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/vulkan_helpers.h"
#include "vlk/utils/utils.h"
#include "vulkan/vulkan.h"

//

#include "vk_mem_alloc.h"

namespace vlk {
namespace vk {

struct InstanceHandle {
  VkInstance instance = nullptr;
  stx::Option<VkDebugUtilsMessengerEXT> debug_messenger = stx::None;

  VLK_MAKE_HANDLE(InstanceHandle)

  ~InstanceHandle() {
    if (instance != nullptr) {
      if (debug_messenger.is_some()) {
        destroy_debug_messenger(instance, debug_messenger.value(), nullptr);
      }
      vkDestroyInstance(instance, nullptr);
    }
  }
};

struct Instance {
  static Instance create(std::string const& app_name, uint32_t app_version,
                         std::string const& engine_name,
                         uint32_t engine_version,
                         stx::Span<char const* const> required_extensions = {},
                         stx::Span<char const* const> validation_layers = {}) {
    auto handle = std::shared_ptr<InstanceHandle>(new InstanceHandle{});
    auto [instance, messenger] = create_vulkan_instance(
        required_extensions, validation_layers,
        make_debug_messenger_create_info(), app_name.c_str(), app_version,
        engine_name.c_str(), engine_version);

    // validation layers are extensions and might not be supported so we still
    // need to check for support
    handle->instance = instance;
    if (messenger != nullptr) {
      handle->debug_messenger = stx::Some(std::move(messenger));
    }
    return Instance{std::move(handle)};
  }

  std::shared_ptr<InstanceHandle const> handle;
};

struct PhysDeviceInfo {
  VkPhysicalDevice phys_device = nullptr;
  VkPhysicalDeviceProperties properties{};
  VkPhysicalDeviceFeatures features{};
  std::vector<VkQueueFamilyProperties> family_properties;
  Instance instance;
};

struct PhysDevice {
  static std::vector<PhysDevice> get_all(Instance const& instance) {
    uint32_t devices_count = 0;

    VLK_MUST_SUCCEED(vkEnumeratePhysicalDevices(instance.handle->instance,
                                                &devices_count, nullptr),
                     "Unable to get physical devices");

    VLK_ENSURE(devices_count != 0, "No Physical Device Found");

    std::vector<VkPhysicalDevice> physical_devices(devices_count);
    VLK_MUST_SUCCEED(
        vkEnumeratePhysicalDevices(instance.handle->instance, &devices_count,
                                   physical_devices.data()),
        "Unable to get physical devices");

    std::vector<PhysDevice> devices;

    for (VkPhysicalDevice device : physical_devices) {
      VkPhysicalDeviceProperties device_properties;
      VkPhysicalDeviceFeatures device_features;

      vkGetPhysicalDeviceProperties(device, &device_properties);
      vkGetPhysicalDeviceFeatures(device, &device_features);

      auto phys_device =
          PhysDevice{PhysDeviceInfo{device, device_properties, device_features,
                                    get_queue_families(device), instance}};

      devices.push_back(std::move(phys_device));
    }

    return devices;
  }

  std::string format() const {
    auto const& properties = info.properties;
    return fmt::format("Device(name: '{}', ID: {}, type: {}) ",
                       properties.deviceName, properties.deviceID,
                       ::vlk::vk::format(properties.deviceType));
  }

  bool has_geometry_shader() const { return info.features.geometryShader; }

  bool has_transfer_command_queue_family() const {
    return std::any_of(info.family_properties.begin(),
                       info.family_properties.end(),
                       [](VkQueueFamilyProperties const& prop) -> bool {
                         return prop.queueFlags & VK_QUEUE_TRANSFER_BIT;
                       });
  }

  bool has_graphics_command_queue_family() const {
    return std::any_of(info.family_properties.begin(),
                       info.family_properties.end(),
                       [](VkQueueFamilyProperties const& prop) -> bool {
                         return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                       });
  }

  std::string format_features() const {
    // nice-to-have(lamarrr): print all properties and capabilities
    return fmt::format("Geometry Shader: {}", has_geometry_shader());
  }

  PhysDeviceInfo info;
};

struct QueueInfo {
  uint32_t family_index = 0;
  VkQueue raw_handle = nullptr;
  float priority = 0.0f;
  uint32_t create_index = 0;
};

struct DeviceHandle {
  VkDevice device = nullptr;
  PhysDevice phys_device;
  std::vector<QueueInfo> command_queues;

  VLK_MAKE_HANDLE(DeviceHandle)

  ~DeviceHandle() {
    if (device != nullptr) vkDestroyDevice(device, nullptr);
  }
};

struct Device {
  static Device create(
      PhysDevice const& phys_device,
      stx::Span<VkDeviceQueueCreateInfo const> command_queue_create_info,
      stx::Span<char const* const> required_extensions = {},
      stx::Span<char const* const> required_validation_layers = {},
      VkPhysicalDeviceFeatures required_features = {}) {
    auto handle = std::shared_ptr<DeviceHandle>(new DeviceHandle{});
    handle->phys_device = phys_device;
    handle->device = create_logical_device(
        phys_device.info.phys_device, required_extensions,
        required_validation_layers, command_queue_create_info, nullptr,
        required_features);

    for (size_t i = 0; i < command_queue_create_info.size(); i++) {
      auto create_info = command_queue_create_info[i];
      auto command_queue_family_index = create_info.queueFamilyIndex;
      auto queue_count = create_info.queueCount;
      VLK_ENSURE(command_queue_family_index <
                 phys_device.info.family_properties.size());

      for (uint32_t queue_index = 0; queue_index < queue_count; queue_index++) {
        float priority = create_info.pQueuePriorities[i];
        VkQueue command_queue = get_command_queue(
            handle->device, command_queue_family_index, queue_index);
        handle->command_queues.push_back(QueueInfo{
            command_queue_family_index, command_queue, priority, queue_index});
      }
    }

    return std::move(Device{std::move(handle)});
  }

  std::shared_ptr<DeviceHandle> handle;
};

struct CommandQueueFamilyInfo {
  // automatically destroyed once the device is destroyed
  uint32_t index = 0;
  PhysDevice phys_device;
};

struct CommandQueueFamily {
  // can also be used for transfer
  static stx::Option<CommandQueueFamily> get_graphics(
      PhysDevice const& phys_device) {
    auto pos = std::find_if(phys_device.info.family_properties.begin(),
                            phys_device.info.family_properties.end(),
                            [](VkQueueFamilyProperties const& prop) -> bool {
                              return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                            });

    if (pos == phys_device.info.family_properties.end()) {
      return stx::None;
    }

    CommandQueueFamilyInfo info;
    info.index = pos - phys_device.info.family_properties.begin();
    info.phys_device = phys_device;

    return stx::Some(CommandQueueFamily{std::move(info)});
  }

  CommandQueueFamilyInfo info;
};

struct CommandQueueInfo {
  // automatically destroyed once the device is destroyed
  VkQueue queue = nullptr;
  uint32_t index = 0;
  float priority = 0.0f;
  CommandQueueFamily family;
  Device device;
};

struct CommandQueue {
  static stx::Option<CommandQueue> get(Device const& device,
                                       CommandQueueFamily const& family,
                                       uint32_t command_queue_create_index) {
    // We shouldn't have to perform checks?
    VLK_ENSURE(device.handle->phys_device.info.phys_device ==
               family.info.phys_device.info.phys_device);

    auto pos = std::find_if(
        device.handle->command_queues.begin(),
        device.handle->command_queues.end(), [&](QueueInfo const& info) {
          return info.family_index == family.info.index &&
                 info.create_index == command_queue_create_index;
        });

    if (pos == device.handle->command_queues.end()) {
      return stx::None;
    }

    CommandQueueInfo info;

    info.queue = pos->raw_handle;
    info.family = CommandQueueFamily{
        CommandQueueFamilyInfo{pos->family_index, device.handle->phys_device}};
    info.index = pos->create_index;
    info.priority = pos->priority;
    info.device = device;

    return stx::Some(CommandQueue{std::move(info)});
  }

  CommandQueueInfo info;
};

struct AllocatorHandle {
  VmaAllocator allocator = nullptr;
  Device device;

  VLK_MAKE_HANDLE(AllocatorHandle)

  ~AllocatorHandle() {
    if (allocator != nullptr) {
      vmaDestroyAllocator(allocator);
    }
  }
};

struct Allocator {
  static Allocator create(Device const& device) {
    VmaAllocatorCreateInfo info{};
    info.vulkanApiVersion =
        device.handle->phys_device.info.properties.apiVersion;
    info.device = device.handle->device;
    info.physicalDevice = device.handle->phys_device.info.phys_device;
    info.instance = device.handle->phys_device.info.instance.handle->instance;

    auto handle = std::shared_ptr<AllocatorHandle>(new AllocatorHandle{});

    handle->device = device;

    VLK_MUST_SUCCEED(vmaCreateAllocator(&info, &handle->allocator),
                     "Unable to create allocator");

    return Allocator{std::move(handle)};
  }

  std::shared_ptr<AllocatorHandle> handle;
};

struct ImageHandle {
  VkImage image = nullptr;
  uint32_t queue_family = 0;
  VmaAllocation allocation = nullptr;
  Extent extent;

  Allocator allocator;

  VLK_MAKE_HANDLE(ImageHandle)

  ~ImageHandle() {
    if (image != nullptr) {
      vmaDestroyImage(allocator.handle->allocator, image, allocation);
    }
  }
};

// 2d Image
struct Image {
  static stx::Option<Image> create(Allocator const& allocator,
                                   CommandQueueFamily const& family,
                                   VkFormat format, Extent extent) {
    if (!extent.visible()) return stx::None;

    VkImageCreateInfo info{};

    // TODO(lamarrr) should we store the queue family object?
    info.arrayLayers = 1;

    info.extent.width = extent.width;
    info.extent.height = extent.height;
    info.extent.depth = 1;

    info.flags = 0;

    info.format = format;
    info.imageType = VK_IMAGE_TYPE_2D;

    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.mipLevels = 1;
    info.pNext = nullptr;
    info.pQueueFamilyIndices = &family.info.index;
    info.queueFamilyIndexCount = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VmaAllocationCreateInfo alloc_info{};

    auto handle = std::shared_ptr<ImageHandle>(new ImageHandle{});
    handle->allocator = allocator;
    handle->queue_family = family.info.index;
    handle->extent = extent;

    VkResult result =
        vmaCreateImage(allocator.handle->allocator, &info, &alloc_info,
                       &handle->image, &handle->allocation, nullptr);

    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
      return stx::None;
    }

    if (result == VK_SUCCESS) {
      return stx::Some(Image{std::move(handle)});
    }

    VLK_MUST_SUCCEED(result, "Unable to create image on device");
  }

  std::shared_ptr<ImageHandle> handle;
};

struct ImageViewHandle {
  VkImageView view = nullptr;
  Image image;

  VLK_MAKE_HANDLE(ImageViewHandle)

  ~ImageViewHandle() {
    if (view != nullptr) {
      vkDestroyImageView(image.handle->allocator.handle->device.handle->device,
                         view, nullptr);
    }
  }
};

struct ImageView {
  std::shared_ptr<ImageViewHandle> handle;
};

}  // namespace vk
}  // namespace vlk
