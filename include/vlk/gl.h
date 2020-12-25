
#pragma once

#include <algorithm>
#include <chrono>
#include <limits>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "GLFW/glfw3.h"
#include "stx/option.h"
#include "stx/result.h"
#include "stx/span.h"
#include "vlk/gl_debug.h"
#include "vlk/utils.h"
#include "vulkan/vulkan.h"

namespace vlk {

// terminology: every object created using a `create_*` requires a `vkDestroy`
// call.
// `make_*` returns plain structs that could possibly contain immutable
// view of data.

using DevicePropFt = std::tuple<VkPhysicalDevice, VkPhysicalDeviceProperties,
                                VkPhysicalDeviceFeatures>;

[[nodiscard]] VkInstance create_vulkan_instance(
    stx::Span<char const* const> const& required_extensions,
    [[maybe_unused]] stx::Span<char const* const> const&
        required_validation_layers,
    [[maybe_unused]] VkDebugUtilsMessengerCreateInfoEXT const*
        default_debug_messenger_create_info = nullptr,
    char const* const application_name = "Valkyrie",
    uint32_t application_version = VK_MAKE_VERSION(1, 0, 0),
    char const* const engine_name = "Valkyrie Engine",
    uint32_t engine_version = VK_MAKE_VERSION(1, 0, 0)) {
  // helps bnt not necessary
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = application_name;
  app_info.applicationVersion = application_version;
  app_info.pEngineName = engine_name;
  app_info.engineVersion = engine_version;
  app_info.apiVersion = VK_API_VERSION_1_2;
  app_info.pNext = nullptr;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.pNext = nullptr;

  uint32_t available_vk_extensions_count = 0;
  vkEnumerateInstanceExtensionProperties(
      nullptr, &available_vk_extensions_count, nullptr);

  std::vector<VkExtensionProperties> available_vk_extensions(
      available_vk_extensions_count);
  vkEnumerateInstanceExtensionProperties(
      nullptr, &available_vk_extensions_count, available_vk_extensions.data());

  VLK_LOG("Available Vulkan Extensions:");
  for (auto extension : available_vk_extensions) {
    VLK_LOG("\t{},  spec version: {}", extension.extensionName,
            extension.specVersion);
  }

  create_info.enabledExtensionCount = required_extensions.size();
  create_info.ppEnabledExtensionNames = required_extensions.data();

  // debug message callback extension

#if VLK_DEBUG
  // validation layers
  ensure_validation_layers_supported(required_validation_layers);
  create_info.enabledLayerCount = required_validation_layers.size();
  create_info.ppEnabledLayerNames = required_validation_layers.data();

  // debug messenger for when the installed debug messenger is uninstalled
  create_info.pNext = default_debug_messenger_create_info;

#endif

  VkInstance vulkan_instance;
  VLK_MUST_SUCCEED(vkCreateInstance(&create_info, nullptr, &vulkan_instance),
                   "Unable to create vulkan instance");

  return vulkan_instance;
}

[[nodiscard]] constexpr bool device_gt_eq(DevicePropFt const& a,
                                          DevicePropFt const& b) {
  // Dedicated GPUs
  VkPhysicalDeviceType a_t = std::get<1>(a).deviceType;
  VkPhysicalDeviceType b_t = std::get<1>(b).deviceType;

  if (a_t == b_t) return true;

  if (a_t == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    return true;
  }

  if (b_t == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    return false;
  }

  // virtual GPUs

  if (a_t == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
    return true;
  }

  if (b_t == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
    return false;
  }

  // Integrated GPUs

  if (a_t == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
    return true;
  }

  if (b_t == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
    return false;
  }

  // CPUs
  if (a_t == VK_PHYSICAL_DEVICE_TYPE_CPU) {
    return true;
  }

  if (b_t == VK_PHYSICAL_DEVICE_TYPE_CPU) {
    return false;
  }

  // OTHER
  if (a_t == VK_PHYSICAL_DEVICE_TYPE_OTHER) {
    return true;
  }

  if (b_t == VK_PHYSICAL_DEVICE_TYPE_OTHER) {
    return false;
  }

  // rest are treated as same
  return false;
}

[[nodiscard]] constexpr bool device_lt(DevicePropFt const& a,
                                       DevicePropFt const& b) {
  return !device_gt_eq(a, b);
}

[[nodiscard]] std::string name_physical_device(
    VkPhysicalDeviceProperties const& properties) {
  std::string name = properties.deviceName;

  name += " (id: " + std::to_string(properties.deviceID) + ", type: ";

  switch (properties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      name += "CPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      name += "dGPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      name += "iGPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      name += "vGPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
      name += "other";
      break;
    default:
      name += "unknown";
  }

  name += ")";

  return name;
}

[[nodiscard]] std::vector<DevicePropFt> get_physical_devices(
    VkInstance vk_instance) {
  uint32_t devices_count = 0;

  VLK_MUST_SUCCEED(
      vkEnumeratePhysicalDevices(vk_instance, &devices_count, nullptr),
      "Unable to get physical devices");

  VLK_ENSURE(devices_count != 0, "No Physical Device Found");

  std::vector<VkPhysicalDevice> physical_devices(devices_count);
  VLK_MUST_SUCCEED(vkEnumeratePhysicalDevices(vk_instance, &devices_count,
                                              physical_devices.data()),
                   "Unable to get physical devices");

  std::vector<DevicePropFt> device_prop_ft;

  VLK_LOG("Available Physical Devices:");
  for (VkPhysicalDevice device : physical_devices) {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceProperties(device, &device_properties);

    vkGetPhysicalDeviceFeatures(device, &device_features);

    VLK_LOG("\t{} (geometry shader: {}) ",
            name_physical_device(device_properties),
            device_features.geometryShader);
    device_prop_ft.emplace_back(device, device_properties, device_features);
  }

  return device_prop_ft;
}

// selects GPU, in the following preference order: dGPU => vGPU => iGPU => CPU
[[nodiscard]] DevicePropFt most_suitable_physical_device(
    stx::Span<DevicePropFt const> const& physical_devices,
    std::function<VkBool32(DevicePropFt const&)> const& criteria) {
  std::vector<DevicePropFt> prioritized_physical_devices{
      physical_devices.begin(), physical_devices.end()};

  std::sort(prioritized_physical_devices.begin(),
            prioritized_physical_devices.end(), device_gt_eq);

  auto It_selected_device = std::find_if(
      prioritized_physical_devices.begin(), prioritized_physical_devices.end(),
      [&](DevicePropFt const& dev) -> bool { return criteria(dev); });

  VLK_ENSURE(It_selected_device != prioritized_physical_devices.end(),
             "No Suitable Physical Device Found");

  return *It_selected_device;
}

//  to do anything on the GPU (render, draw, compute, allocate memory, create
//  texture, etc.) we use command queues
[[nodiscard]] std::vector<VkQueueFamilyProperties> get_queue_families(
    VkPhysicalDevice device) {
  uint32_t queue_families_count;

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queue_families_properties(
      queue_families_count);

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count,
                                           queue_families_properties.data());

  return queue_families_properties;
}

[[nodiscard]] std::vector<bool> get_command_queue_support(
    stx::Span<VkQueueFamilyProperties const> const& queue_families,
    VkQueueFlagBits required_command_queue) {
  std::vector<bool> supports;

  for (auto const& fam_props : queue_families) {
    supports.push_back(fam_props.queueFlags & required_command_queue);
  }

  return supports;
}

// find the device's queue family capable of supporting surface presentation
[[nodiscard]] std::vector<bool> get_surface_presentation_command_queue_support(
    VkPhysicalDevice physical_device,
    stx::Span<VkQueueFamilyProperties const> const& queue_families,
    VkSurfaceKHR surface) {
  std::vector<bool> supports;

  for (size_t i = 0; i < queue_families.size(); i++) {
    VkBool32 surface_presentation_supported;
    VLK_MUST_SUCCEED(
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                             &surface_presentation_supported),
        "Unable to query physical device' surface support");
    supports.push_back(surface_presentation_supported);
  }

  return supports;
}

[[nodiscard]] VkDevice create_logical_device(
    VkPhysicalDevice physical_device,
    stx::Span<char const* const> const& required_extensions,
    stx::Span<char const* const> const& required_validation_layers,
    stx::Span<VkDeviceQueueCreateInfo const> const& command_queue_create_infos,
    VkAllocationCallbacks const* allocation_callback,
    VkPhysicalDeviceFeatures const& required_features = {}) {
  VkDeviceCreateInfo device_create_info{};
  device_create_info.pQueueCreateInfos = command_queue_create_infos.data();
  device_create_info.queueCreateInfoCount = command_queue_create_infos.size();
  device_create_info.pEnabledFeatures = &required_features;

  uint32_t available_extensions_count;
  VLK_MUST_SUCCEED(
      vkEnumerateDeviceExtensionProperties(
          physical_device, nullptr, &available_extensions_count, nullptr),
      "Unable to get physical device extensions");

  // device specific extensions
  std::vector<VkExtensionProperties> available_device_extensions(
      available_extensions_count);

  VLK_MUST_SUCCEED(vkEnumerateDeviceExtensionProperties(
                       physical_device, nullptr, &available_extensions_count,
                       available_device_extensions.data()),
                   "Unable to get physical device extensions");

  VLK_LOG("Required Device Extensions: ");
  std::for_each(required_extensions.begin(), required_extensions.end(),
                [](char const* ext) { VLK_LOG("\t{}", ext); });

  VLK_LOG("Available Device Extensions: ");
  std::for_each(
      available_device_extensions.begin(), available_device_extensions.end(),
      [](VkExtensionProperties ext) {
        VLK_LOG("\t{} (spec version: {})", ext.extensionName, ext.specVersion);
      });

  VLK_ENSURE(
      std::all_of(required_extensions.begin(), required_extensions.end(),
                  [&](char const* const ext) {
                    return std::find_if(
                               available_device_extensions.begin(),
                               available_device_extensions.end(),
                               [=](VkExtensionProperties a_ext) {
                                 return std::string_view(ext) ==
                                        std::string_view(a_ext.extensionName);
                               }) != available_device_extensions.end();
                  }),
      "Can't find all required extensions");

  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  device_create_info.ppEnabledLayerNames = required_validation_layers.data();
  device_create_info.enabledLayerCount = required_validation_layers.size();

  device_create_info.ppEnabledExtensionNames = required_extensions.data();
  device_create_info.enabledExtensionCount = required_extensions.size();

  device_create_info.pQueueCreateInfos = command_queue_create_infos.data();
  device_create_info.queueCreateInfoCount = command_queue_create_infos.size();

  VkDevice logical_device;

  VLK_ENSURE(vkCreateDevice(physical_device, &device_create_info,
                            allocation_callback, &logical_device) == VK_SUCCESS,
             "Unable to Create Physical Device");

  return logical_device;
}

[[nodiscard]] VkQueue get_command_queue(
    VkDevice device, uint32_t queue_family_index,
    uint32_t command_queue_index_in_family) {
  VkQueue command_queue;
  vkGetDeviceQueue(device, queue_family_index, command_queue_index_in_family,
                   &command_queue);
  VLK_ENSURE(command_queue != nullptr,
             "Requested command queue not created on target device");
  return command_queue;
}

struct [[nodiscard]] SwapChainProperties {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> supported_formats;
  std::vector<VkPresentModeKHR> presentation_modes;
};

[[nodiscard]] SwapChainProperties get_swapchain_properties(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
  SwapChainProperties details{};

  VLK_MUST_SUCCEED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                       physical_device, surface, &details.capabilities),
                   "Unable to get physical device' surface capabilities");

  uint32_t supported_surface_formats_count = 0;

  VLK_MUST_SUCCEED(
      vkGetPhysicalDeviceSurfaceFormatsKHR(
          physical_device, surface, &supported_surface_formats_count, nullptr),
      "Unable to get physical device' surface format");

  details.supported_formats.resize(supported_surface_formats_count);

  VLK_MUST_SUCCEED(
      vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                           &supported_surface_formats_count,
                                           details.supported_formats.data()),
      "Unable to get physical device' surface format");

  uint32_t surface_presentation_modes_count;
  VLK_MUST_SUCCEED(
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          physical_device, surface, &surface_presentation_modes_count, nullptr),
      "Unable to get physical device' surface presentation mode");

  details.presentation_modes.resize(surface_presentation_modes_count);
  VLK_MUST_SUCCEED(
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          physical_device, surface, &surface_presentation_modes_count,
          details.presentation_modes.data()),
      "Unable to get physical device' surface presentation mode");

  return details;
}

[[nodiscard]] bool is_swapchain_adequate(
    SwapChainProperties const& properties) {
  // we use any available for selecting devices
  VLK_ENSURE(properties.supported_formats.size() != 0,
             "Physical Device does not support any window surface "
             "format");

  VLK_ENSURE(properties.presentation_modes.size() != 0,
             "Physical Device does not support any window surface "
             "presentation mode");

  return true;
}

// choose a specific surface format available on the GPU
[[nodiscard]] VkSurfaceFormatKHR select_surface_formats(
    stx::Span<VkSurfaceFormatKHR const> const& formats) {
  VLK_ENSURE(formats.size() != 0, "No window surface format gotten as arg");
  auto It_format = std::find_if(
      formats.begin(), formats.end(), [](VkSurfaceFormatKHR const& format) {
        return format.format == VK_FORMAT_R8G8B8_SRGB &&
               format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      });

  return (It_format != formats.end()) ? *It_format : formats[0];
}

[[nodiscard]] VkPresentModeKHR select_surface_presentation_mode(
    stx::Span<VkPresentModeKHR const> const& available_presentation_modes) {
  /*
  - VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are
  transferred to the screen right away, which may result in tearing.

  - VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes
  an image from the front of the queue when the display is refreshed and the
  program inserts rendered images at the back of the queue. If the queue is full
  then the program has to wait. This is most similar to vertical sync as found
  in modern games. The moment that the display is refreshed is known as
  "vertical blank".

  - VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs
  from the previous one if the application is late and the queue was empty at
  the last vertical blank. Instead of waiting for the next vertical blank, the
  image is transferred right away when it finally arrives. This may result in
  visible tearing.

  - VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the
  second mode. Instead of blocking the application when the queue is full, the
  images that are already queued are simply replaced with the newer ones. This
  mode can be used to implement triple buffering, which allows you to avoid
  tearing with significantly less latency issues than standard vertical sync
  that uses double buffering.
  */

  VLK_ENSURE(available_presentation_modes.size() != 0,
             "No surface presentation mode available");

  auto mode = std::find(available_presentation_modes.begin(),
                        available_presentation_modes.end(),
                        VK_PRESENT_MODE_MAILBOX_KHR);
  if (mode != available_presentation_modes.end()) return *mode;

  if (std::find(available_presentation_modes.begin(),
                available_presentation_modes.end(), VK_PRESENT_MODE_FIFO_KHR) ==
      available_presentation_modes.end()) {
    VLK_WARN(
        "Device does not support the Mailbox surface presentation mode nor "
        "blocking FIFO surface presentation mode, using a random surface "
        "presentation mode");

    return available_presentation_modes[0];
  } else {
    VLK_WARN(
        "Device does not support the Mailbox surface presentation mode, using "
        "blocking FIFO");

    return VK_PRESENT_MODE_FIFO_KHR;
  }
}

[[nodiscard]] VkExtent2D select_swapchain_extent(
    GLFWwindow* window, VkSurfaceCapabilitiesKHR const& capabilities) {
  // if this is already set (value other than uint32_t::max) then we are not
  // allowed to choose the extent

  if ((capabilities.currentExtent.width !=
       std::numeric_limits<uint32_t>::max()) ||
      (capabilities.currentExtent.height !=
       std::numeric_limits<uint32_t>::max())) {
    return capabilities.currentExtent;
  } else {
    int width, height;

    // this, unlike the window dimensions is in pixels and is the rendered to
    // area
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D target_extent = {static_cast<uint32_t>(width),
                                static_cast<uint32_t>(height)};

    target_extent.width =
        std::clamp(target_extent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    target_extent.height =
        std::clamp(target_extent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return target_extent;
  }
}

[[nodiscard]] VkSwapchainKHR create_swapchain(
    VkDevice device, VkSurfaceKHR surface, VkExtent2D const& extent,
    VkSurfaceFormatKHR surface_format, VkPresentModeKHR present_mode,
    SwapChainProperties const& properties,
    VkSharingMode accessing_queue_families_sharing_mode,
    stx::Span<uint32_t const> const& accessing_queue_families_indexes,
    VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    VkCompositeAlphaFlagBitsKHR alpha_channel_blending =
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    VkBool32 clipped = VK_TRUE) {
  VkSwapchainCreateInfoKHR create_info{};

  // number of images to have on the swap chain

  uint32_t image_count =
      properties.capabilities.maxImageCount == 0
          ?  // no limit on the number of swapchain images
          (properties.capabilities.minImageCount + 1)
          : std::clamp(properties.capabilities.minImageCount + 1,
                       properties.capabilities.minImageCount,
                       properties.capabilities.maxImageCount);

  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.imageExtent = extent;
  create_info.surface = surface;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.presentMode = present_mode;

  // number of images to use for buffering on the swapchain
  create_info.minImageCount = image_count;

  create_info.imageArrayLayers = 1;  // 2 for stereoscopic rendering
  create_info.imageUsage = image_usage;
  create_info.preTransform = properties.capabilities.currentTransform;
  create_info.compositeAlpha =
      alpha_channel_blending;  // how the alpha channel should be
                               // used for blending with other
                               // windows in the window system
  create_info.clipped = clipped;
  create_info.oldSwapchain = VK_NULL_HANDLE;

  // under normal circumstances command queues on the same queue family can
  // access data without data race issues
  //
  // VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time
  // and ownership must be explicitly transferred before using it in another
  // queue family. This option offers the best performance.
  // VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue
  // families without explicit ownership transfers.
  create_info.imageSharingMode = accessing_queue_families_sharing_mode;
  create_info.pQueueFamilyIndices = accessing_queue_families_indexes.data();
  create_info.queueFamilyIndexCount = accessing_queue_families_indexes.size();

  VkSwapchainKHR swapchain;
  VLK_MUST_SUCCEED(
      vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain),
      "Unable to create swapchain");

  return swapchain;
}

[[nodiscard]] std::vector<VkImage> get_swapchain_images(
    VkDevice device, VkSwapchainKHR swapchain) {
  uint32_t image_count;

  VLK_MUST_SUCCEED(
      vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr),
      "Unable to get swapchain images count");

  std::vector<VkImage> swapchain_images;
  swapchain_images.resize(image_count);

  VLK_MUST_SUCCEED(vkGetSwapchainImagesKHR(device, swapchain, &image_count,
                                           swapchain_images.data()),
                   "Unable to get swapchain images");

  return swapchain_images;
}

// the number of command queues to create is encapsulated in the
// `queue_priorities` size
[[nodiscard]] VkDeviceQueueCreateInfo make_command_queue_create_info(
    uint32_t queue_family_index,
    stx::Span<float const> const& queues_priorities) {
  VkDeviceQueueCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  create_info.queueFamilyIndex = queue_family_index;
  create_info.pQueuePriorities = queues_priorities.data();
  create_info.queueCount =
      queues_priorities.size();  // the number of queues we want, since multiple
                                 // queues can belong to a single family

  return create_info;
}

[[nodiscard]] VkImageView create_image_view(VkDevice device, VkImage image,
                                            VkFormat format) {
  VkImageViewCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  create_info.image = image;

  // VK_IMAGE_VIEW_TYPE_2D: 2D texture
  // VK_IMAGE_VIEW_TYPE_3D: 3D texture
  // VK_IMAGE_VIEW_TYPE_CUBE: cube map
  create_info.viewType =
      VK_IMAGE_VIEW_TYPE_2D;  // treat the image as a 2d texture
  create_info.format = format;

  // how to map the image color components
  create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  // defines what part of the image this image view represents and what this
  // image view is used for
  create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  create_info.subresourceRange.baseArrayLayer = 0;
  create_info.subresourceRange.baseMipLevel = 0;
  create_info.subresourceRange.layerCount = 1;
  create_info.subresourceRange.levelCount = 1;

  VkImageView image_view;
  VLK_MUST_SUCCEED(
      vkCreateImageView(device, &create_info, nullptr, &image_view),
      "Unable to create image view");
  return image_view;
}

[[nodiscard]] VkShaderModule create_shader_module(
    VkDevice device, stx::Span<uint32_t const> const& spirv_byte_data) {
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = spirv_byte_data.size_bytes();
  create_info.pCode = spirv_byte_data.data();

  VkShaderModule shader_module;
  VLK_MUST_SUCCEED(
      vkCreateShaderModule(device, &create_info, nullptr, &shader_module),
      "Unable to create shader module");
  return shader_module;
}

[[nodiscard]] VkPipelineShaderStageCreateInfo
make_pipeline_shader_stage_create_info(
    VkShaderModule module, char const* program_entry_point,
    VkShaderStageFlagBits pipeline_stage_flag,
    VkSpecializationInfo const* program_constants) {
  VkPipelineShaderStageCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  create_info.module = module;
  create_info.pName = program_entry_point;
  create_info.stage = pipeline_stage_flag;
  create_info.pNext = nullptr;
  create_info.pSpecializationInfo =
      program_constants;  // provide constants used within the shader

  return create_info;
}

[[nodiscard]] VkPipelineShaderStageCreateInfo
make_pipeline_shader_stage_create_info(
    VkShaderModule module, char const* program_entry_point,
    VkShaderStageFlagBits pipeline_stage_flag) {
  VkPipelineShaderStageCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  create_info.module = module;
  create_info.pName = program_entry_point;
  create_info.stage = pipeline_stage_flag;
  create_info.pNext = nullptr;
  create_info.pSpecializationInfo =
      nullptr;  // provide constants used within the shader

  return create_info;
}

[[nodiscard]] VkPipelineVertexInputStateCreateInfo
make_pipeline_vertex_input_state_create_info(
    stx::Span<VkVertexInputBindingDescription const> const&
        vertex_binding_descriptions,
    stx::Span<VkVertexInputAttributeDescription const> const&
        vertex_attribute_desciptions) {
  // Bindings: spacing between data and whether the data is per-vertex or
  // per-instance
  // Attribute descriptions: type of the attributes passed to the vertex shader,
  // which binding to load them from and at which offset
  VkPipelineVertexInputStateCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  create_info.vertexBindingDescriptionCount =
      vertex_binding_descriptions.size();
  create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
  create_info.vertexAttributeDescriptionCount =
      vertex_attribute_desciptions.size();
  create_info.pVertexAttributeDescriptions =
      vertex_attribute_desciptions.data();

  return create_info;
}

[[nodiscard]] VkPipelineInputAssemblyStateCreateInfo
make_pipeline_input_assembly_state_create_info() {
  // Bindings: spacing between data and whether the data is per-vertex or
  // per-instance
  // Attribute descriptions: type of the attributes passed to the vertex shader,
  // which binding to load them from and at which offset
  VkPipelineInputAssemblyStateCreateInfo create_info{};
  create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  create_info.topology =
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // rendering in triangle mode
  create_info.primitiveRestartEnable = VK_FALSE;

  return create_info;
}

[[nodiscard]] VkViewport make_viewport(float x, float y, float w, float h,
                                       float min_depth = 0.0f,
                                       float max_depth = 1.0f) {
  VkViewport viewport{};
  viewport.x = x;
  viewport.y = y;
  viewport.width = w;             // width of the framebuffer (swapchain image)
  viewport.height = h;            // height of the framebuffer (swapchain image)
  viewport.minDepth = min_depth;  // min depth value to use for the frame buffer
  viewport.maxDepth = max_depth;  // max depth value to use for the frame buffer

  return viewport;
}

[[nodiscard]] VkRect2D make_scissor(float x, float y, float w, float h) {
  VkRect2D scissor{};

  scissor.offset.x = x;
  scissor.offset.y = y;

  scissor.extent.width = w;
  scissor.extent.height = h;

  return scissor;
}

[[nodiscard]] VkPipelineViewportStateCreateInfo
make_pipeline_viewport_state_create_info(
    stx::Span<VkViewport const> const& viewports,
    stx::Span<VkRect2D const> const& scissors) {
  // to use multiple viewports, ensure the GPU feature is enabled during logical
  // device creation
  VkPipelineViewportStateCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  create_info.viewportCount = viewports.size();
  create_info.pViewports = viewports.data();
  create_info.scissorCount =
      scissors.size();  // scissors cut out the part to be rendered
  create_info.pScissors = scissors.data();

  return create_info;
}

[[nodiscard]] VkPipelineRasterizationStateCreateInfo
make_pipeline_rasterization_create_info(float line_width = 1.0f) {
  VkPipelineRasterizationStateCreateInfo create_info{};
  create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  create_info.depthClampEnable =
      VK_FALSE;  // ragments that are beyond the near and far planes are clamped
                 // to them as opposed to discarding them. This is useful in
                 // some special cases like shadow maps. Using this requires
                 // enabling a GPU feature.
  create_info.rasterizerDiscardEnable =
      VK_FALSE;  // if true geometry never passes through the rasterization
                 // stage thus disabling output to the framebuffer
  // VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
  // VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
  // VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
  create_info.polygonMode =
      VK_POLYGON_MODE_FILL;  // using any other one requires enabling a GPU
                             // feature
  create_info.lineWidth =
      line_width;  // any thicker than 1.0f requires enabling a GPU feature

  create_info.cullMode = VK_CULL_MODE_BACK_BIT;  // discard the back part of the
                                                 // image that isn't facing us
  create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;

  create_info.depthBiasEnable = VK_FALSE;
  create_info.depthBiasConstantFactor = 0.0f;  // mostly used for shadow mapping
  create_info.depthBiasClamp = 0.0f;
  create_info.depthBiasSlopeFactor = 0.0f;

  return create_info;
}

[[nodiscard]] VkPipelineMultisampleStateCreateInfo
make_pipeline_multisample_state_create_info() {
  VkPipelineMultisampleStateCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  create_info.sampleShadingEnable = VK_FALSE;
  create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  create_info.minSampleShading = 1.0f;
  create_info.pSampleMask = nullptr;
  create_info.alphaToCoverageEnable = VK_FALSE;
  create_info.alphaToOneEnable = VK_FALSE;

  return create_info;
}

[[nodiscard]] VkPipelineDepthStencilStateCreateInfo
make_pipeline_depth_stencil_state_create_info() {
  VkPipelineDepthStencilStateCreateInfo create_info{};
  // VLK_ENSURE(false, "Unimplemented");
  return create_info;
}

// per framebuffer
[[nodiscard]] VkPipelineColorBlendAttachmentState
make_pipeline_color_blend_attachment_state() {
  // simply overwrites the pixels in the destination
  VkPipelineColorBlendAttachmentState state{};
  state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  state.blendEnable = VK_TRUE;
  state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  state.colorBlendOp = VK_BLEND_OP_ADD;
  state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  state.alphaBlendOp = VK_BLEND_OP_ADD;
  return state;
}

// global pipeline state
[[nodiscard]] VkPipelineColorBlendStateCreateInfo
make_pipeline_color_blend_state_create_info(
    stx::Span<VkPipelineColorBlendAttachmentState const> const&
        color_frame_buffers) {
  VkPipelineColorBlendStateCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  create_info.logicOpEnable = VK_FALSE;
  create_info.logicOp = VK_LOGIC_OP_COPY;
  create_info.attachmentCount =
      color_frame_buffers.size();  // number of framebuffers
  create_info.pAttachments = color_frame_buffers.data();
  create_info.blendConstants[0] = 0.0f;
  create_info.blendConstants[1] = 0.0f;
  create_info.blendConstants[2] = 0.0f;
  create_info.blendConstants[3] = 0.0f;

  return create_info;
}

[[nodiscard]] VkPipelineDynamicStateCreateInfo make_pipeline_dynamic_state(
    stx::Span<VkDynamicState const> const& dynamic_states) {
  // This will cause the configuration of these values to be ignored and you
  // will be required to specify the data at drawing time. This struct can be
  // substituted by a nullptr later on if you don't have any dynamic state.

  VkPipelineDynamicStateCreateInfo pipeline_state{};
  pipeline_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  pipeline_state.dynamicStateCount = std::size(dynamic_states);
  pipeline_state.pDynamicStates = dynamic_states.data();

  return pipeline_state;
}

[[nodiscard]] VkPipelineLayout create_pipeline_layout(VkDevice device) {
  VkPipelineLayoutCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  create_info.setLayoutCount = 0;
  create_info.pSetLayouts = nullptr;
  create_info.pushConstantRangeCount = 0;
  create_info.pPushConstantRanges = nullptr;

  VkPipelineLayout layout;
  VLK_MUST_SUCCEED(
      vkCreatePipelineLayout(device, &create_info, nullptr, &layout),
      "Unable to create pipeline layout");

  return layout;
}

[[nodiscard]] VkAttachmentDescription make_attachment_description(
    VkFormat format) {
  // the format of the color attachment should match the format of the swap
  // chain images,
  VkAttachmentDescription attachment_description{};

  attachment_description.format = format;
  attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;  // no multi-sampling

  // The loadOp and storeOp determine what to do with the data in the attachment
  // before rendering and after rendering
  // VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the
  // attachment
  // VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at
  // the start
  // VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined;
  // we don't care about them
  attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
  // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
  // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for
  // a memory copy operation
  // descibes layout of the images
  attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  return attachment_description;
}

// subpasses are for post-processing. each subpass depends on the results of the
// previous (sub)passes, used instead of transferring data
[[nodiscard]] VkSubpassDescription make_subpass_description(
    stx::Span<VkAttachmentReference const> const& color_attachments) {
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = color_attachments.size();
  subpass.pColorAttachments =
      color_attachments.data();  // layout(location = 0) out vec4 outColor

  // pInputAttachments: Attachments that are read from a shader
  // pResolveAttachments: Attachments used for multisampling color attachments
  // pDepthStencilAttachment: Attachment for depth and stencil data
  // pPreserveAttachments: Attachments that are not used by this subpass, but
  // for which the data must be preserved
  return subpass;
}

// ????
[[nodiscard]] VkSubpassDependency make_subpass_dependency() {
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;

  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;

  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  return dependency;
}

// specify how many color and depth buffers there will be, how many samples to
// use for each of them and how their contents should be handled throughout the
// rendering operations (and the subpasses description)
[[nodiscard]] VkRenderPass create_render_pass(
    VkDevice device,
    stx::Span<VkAttachmentDescription const> const& attachment_descriptions,
    stx::Span<VkSubpassDescription const> const& subpass_descriptions,
    stx::Span<VkSubpassDependency const> const& subpass_dependencies) {
  VkRenderPassCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  create_info.attachmentCount = attachment_descriptions.size();
  create_info.pAttachments = attachment_descriptions.data();
  create_info.subpassCount = subpass_descriptions.size();
  create_info.pSubpasses = subpass_descriptions.data();

  create_info.dependencyCount = subpass_dependencies.size();
  create_info.pDependencies = subpass_dependencies.data();

  VkRenderPass render_pass;
  VLK_MUST_SUCCEED(
      vkCreateRenderPass(device, &create_info, nullptr, &render_pass),
      "Unable to create render pass");

  return render_pass;
}

[[nodiscard]] VkPipeline create_graphics_pipeline(
    VkDevice device, VkPipelineLayout layout, VkRenderPass render_pass,
    stx::Span<VkPipelineShaderStageCreateInfo const> const&
        shader_stages_create_infos,
    VkPipelineVertexInputStateCreateInfo const& vertex_input_state,
    VkPipelineInputAssemblyStateCreateInfo const& input_assembly_state,
    VkPipelineViewportStateCreateInfo const& viewport_state,
    VkPipelineRasterizationStateCreateInfo const& rasterization_state,
    VkPipelineMultisampleStateCreateInfo const& multisample_state,
    VkPipelineDepthStencilStateCreateInfo const& depth_stencil_state,
    VkPipelineColorBlendStateCreateInfo const& color_blend_state,
    VkPipelineDynamicStateCreateInfo const& dynamic_state) {
  VkGraphicsPipelineCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  create_info.pStages = shader_stages_create_infos.data();
  create_info.stageCount = shader_stages_create_infos.size();
  create_info.pVertexInputState = &vertex_input_state;
  create_info.pInputAssemblyState = &input_assembly_state;
  create_info.pViewportState = &viewport_state;
  create_info.pRasterizationState = &rasterization_state;
  create_info.pMultisampleState = &multisample_state;
  create_info.pDepthStencilState = &depth_stencil_state;
  create_info.pColorBlendState = &color_blend_state;
  create_info.pDynamicState =
      &dynamic_state;  // which of these fixed function states would change, any
                       // of the ones listed here would need to be provided at
                       // every draw/render call

  create_info.layout = layout;
  create_info.renderPass = render_pass;
  create_info.subpass =
      0;  // index of the device's subpass this graphics pipeline belongs to

  create_info.basePipelineHandle = nullptr;
  create_info.basePipelineIndex = -1;

  VkPipeline graphics_pipeline;

  VLK_MUST_SUCCEED(
      vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create_info,
                                nullptr, &graphics_pipeline),
      "Unable to create graphics pipeline");

  return graphics_pipeline;
}

// basically a collection of attachments (color, depth, stencil, etc)
[[nodiscard]] VkFramebuffer create_frame_buffer(
    VkDevice device, VkRenderPass render_pass,
    stx::Span<VkImageView const> const& attachments, VkExtent2D const& extent) {
  VkFramebufferCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  create_info.renderPass = render_pass;
  create_info.attachmentCount = attachments.size();
  create_info.pAttachments = attachments.data();
  create_info.width = extent.width;
  create_info.height = extent.height;
  create_info.layers = 1;  // our swap chain images are single images, so the
                           // number of layers is 1

  VkFramebuffer frame_buffer;
  VLK_MUST_SUCCEED(
      vkCreateFramebuffer(device, &create_info, nullptr, &frame_buffer),
      "Unable to create frame buffer");
  return frame_buffer;
}

[[nodiscard]] VkCommandPool create_command_pool(
    VkDevice device, uint32_t queue_family_index,
    bool enable_command_buffer_resetting = false) {
  VkCommandPoolCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.queueFamilyIndex = queue_family_index;
  create_info.flags = enable_command_buffer_resetting
                          ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
                          : 0;

  VkCommandPool command_pool;
  VLK_MUST_SUCCEED(
      vkCreateCommandPool(device, &create_info, nullptr, &command_pool),
      "Unable to create command pool");

  return command_pool;
}

void allocate_command_buffer(VkDevice device, VkCommandPool command_pool,
                             VkCommandBuffer& command_buffer  // NOLINT
) {
  VkCommandBufferAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.commandPool = command_pool;

  // VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution,
  // but cannot be called from other command buffers.
  // VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be
  // called from primary command buffers.
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount = 1;

  VLK_MUST_SUCCEED(
      vkAllocateCommandBuffers(device, &allocate_info, &command_buffer),
      "Unable to allocate command buffer");
}

void allocate_command_buffers(
    VkDevice device, VkCommandPool command_pool,
    stx::Span<VkCommandBuffer> const& command_buffers) {
  VkCommandBufferAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.commandPool = command_pool;

  // VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution,
  // but cannot be called from other command buffers.
  // VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be
  // called from primary command buffers.
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount = command_buffers.size();

  VLK_MUST_SUCCEED(
      vkAllocateCommandBuffers(device, &allocate_info, command_buffers.data()),
      "Unable to allocate command buffers");
}

void reset_command_buffer(VkCommandBuffer command_buffer,
                          bool release_resources = false) {
  VLK_MUST_SUCCEED(
      vkResetCommandBuffer(command_buffer,
                           release_resources
                               ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
                               : 0),
      "Unable to reset command buffer");
}

namespace cmd {

struct Recorder {
  VkCommandBuffer command_buffer_;

  Recorder begin_recording(
      VkCommandBufferUsageFlagBits usage = {},
      VkCommandBufferInheritanceInfo const* inheritance_info = nullptr) {
    VkCommandBufferBeginInfo begin_info{};

    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be
    // rerecorded right after executing it once.
    // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary
    // command buffer that will be entirely within a single render pass.
    // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be
    // resubmitted while it is also already pending execution
    begin_info.flags = usage;
    begin_info.pInheritanceInfo = inheritance_info;

    VLK_MUST_SUCCEED(vkBeginCommandBuffer(command_buffer_, &begin_info),
                     "unable to begin command buffer recording");

    return *this;
  }

  Recorder copy(VkBuffer src, uint64_t src_offset, uint64_t size, VkBuffer dst,
                uint64_t dst_offset) {
    VkBufferCopy copy_region{};
    copy_region.dstOffset = dst_offset;
    copy_region.size = size;
    copy_region.srcOffset = src_offset;
    vkCmdCopyBuffer(command_buffer_, src, dst, 1, &copy_region);
    return *this;
  }

  // TODO(lamarrr): make into multi-copy interface
  Recorder copy(VkBuffer src, uint64_t src_offset, VkImage dst,
                VkImageLayout dst_expected_layout, VkOffset3D dst_offset,
                VkExtent3D dst_extent) {
    VkBufferImageCopy copy_region{};
    copy_region.bufferOffset = src_offset;
    copy_region.bufferRowLength = 0;    // tightly-packed, no padding
    copy_region.bufferImageHeight = 0;  // tightly-packed, no padding

    copy_region.imageOffset = dst_offset;
    copy_region.imageExtent = dst_extent;

    copy_region.imageSubresource.aspectMask =
        VK_IMAGE_ASPECT_COLOR_BIT;  // we want to copy the color components of
                                    // the pixels

    // TODO(lamarrr): remove hard-coding
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;

    vkCmdCopyBufferToImage(command_buffer_, src, dst, dst_expected_layout, 1,
                           &copy_region);
    return *this;
  }

  Recorder begin_render_pass(
      VkRenderPass render_pass, VkFramebuffer framebuffer, VkRect2D render_area,
      stx::Span<VkClearValue const> const& clear_values) {
    VkRenderPassBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.renderPass = render_pass;
    begin_info.framebuffer = framebuffer;
    begin_info.renderArea = render_area;
    begin_info.clearValueCount = clear_values.size();
    begin_info.pClearValues = clear_values.data();

    // VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in
    // the primary command buffer itself and no secondary command buffers will
    // be executed. VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render
    // pass commands will be executed from secondary command buffers.

    vkCmdBeginRenderPass(command_buffer_, &begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);
    return *this;
  }

  Recorder end_render_pass() {
    vkCmdEndRenderPass(command_buffer_);
    return *this;
  }

  Recorder bind_pipeline(VkPipeline pipeline, VkPipelineBindPoint bind_point) {
    vkCmdBindPipeline(command_buffer_, bind_point, pipeline);
    return *this;
  }

  template <
      typename BufferType,
      std::enable_if_t<static_cast<bool>(BufferType::usage&
                                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
                       int> = 0>
  Recorder bind_vertex_buffer(uint32_t binding, BufferType const& buffer,
                              uint64_t buffer_offset) {
    vkCmdBindVertexBuffers(command_buffer_, binding, 1, &buffer.buffer_,
                           &buffer_offset);
    return *this;
  }

  template <
      typename BufferType,
      std::enable_if_t<static_cast<bool>(
                           BufferType::usage& VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
                       int> = 0>
  Recorder bind_index_buffer(BufferType const& buffer, uint64_t buffer_offset,
                             VkIndexType dtype) {
    vkCmdBindIndexBuffer(command_buffer_, buffer.buffer_, buffer_offset, dtype);
    return *this;
  }

  Recorder bind_descriptor_sets(
      VkPipelineLayout pipeline_layout, VkPipelineBindPoint bind_point,
      stx::Span<VkDescriptorSet const> const& descriptor_sets) {
    vkCmdBindDescriptorSets(command_buffer_, bind_point, pipeline_layout, 0,
                            descriptor_sets.size(), descriptor_sets.data(), 0,
                            nullptr);  // no dynamic offsets for now
    return *this;
  }

  Recorder draw(uint32_t vertex_count, uint32_t instance_count,
                uint32_t first_vertex, uint32_t first_instance) {
    // instanceCount: Used for instanced rendering
    // firstVertex: Used as an offset into the vertex buffer, defines the lowest
    // value of gl_VertexIndex. firstInstance: Used as an offset for instanced
    // rendering, defines the lowest value of gl_InstanceIndex.
    vkCmdDraw(command_buffer_, vertex_count, instance_count, first_vertex,
              first_instance);
    return *this;
  }

  Recorder draw_indexed(uint32_t index_count, uint32_t instance_count,
                        uint32_t first_index, uint32_t vertex_offset,
                        uint32_t first_instance) {
    // instanceCount: Used for instanced rendering
    // firstVertex: Used as an offset into the vertex buffer, defines the lowest
    // value of gl_VertexIndex. firstInstance: Used as an offset for instanced
    // rendering, defines the lowest value of gl_InstanceIndex.
    vkCmdDrawIndexed(command_buffer_, index_count, instance_count, first_index,
                     vertex_offset, first_instance);
    return *this;
  }

  Recorder set_viewports(stx::Span<VkViewport const> const& viewports) {
    vkCmdSetViewport(command_buffer_, 0, viewports.size(), viewports.data());
    return *this;
  }

  Recorder set_scissors(stx::Span<VkRect2D const> const& scissors) {
    vkCmdSetScissor(command_buffer_, 0, scissors.size(), scissors.data());
    return *this;
  }

  Recorder set_line_width(float line_width) {
    vkCmdSetLineWidth(command_buffer_, line_width);
    return *this;
  }

  Recorder end_recording() {
    VLK_MUST_SUCCEED(vkEndCommandBuffer(command_buffer_),
                     "Unable to end command buffer recording");
    return *this;
  }
};
}  // namespace cmd

// GPU-GPU synchronization primitive
[[nodiscard]] VkSemaphore create_semaphore(VkDevice device) {
  VkSemaphoreCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkSemaphore semaphore;
  VLK_MUST_SUCCEED(vkCreateSemaphore(device, &create_info, nullptr, &semaphore),
                   "Unable to create semaphore");

  return semaphore;
}

// GPU-CPU synchronization primitive
[[nodiscard]] VkFence create_fence(VkDevice device, bool make_signaled) {
  VkFenceCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  create_info.flags = make_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

  VkFence fence;

  VLK_MUST_SUCCEED(vkCreateFence(device, &create_info, nullptr, &fence),
                   "Unable to create fence");

  return fence;
}

void reset_fence(VkDevice device, VkFence fence) {
  VLK_MUST_SUCCEED(vkResetFences(device, 1, &fence), "Unable to reset fence");
}

void reset_fences(VkDevice device, stx::Span<VkFence const> const& fences) {
  VLK_MUST_SUCCEED(vkResetFences(device, fences.size(), fences.data()),
                   "Unable to reset fences");
}

void await_fence(VkDevice device, VkFence fence) {
  VLK_MUST_SUCCEED(vkWaitForFences(device, 1, &fence, true,
                                   std::numeric_limits<uint64_t>::max()),
                   "Unable to await fence");
}

// returns true if the fence didn't timeout
[[nodiscard]] bool await_fence(VkDevice device, VkFence fence,
                               std::chrono::nanoseconds timeout) {
  auto result = vkWaitForFences(device, 1, &fence, true, timeout.count());
  VLK_MUST_SUCCEED(
      result == VK_SUCCESS || result == VK_TIMEOUT ? VK_SUCCESS : result,
      "Unable to await fence");

  return result != VK_TIMEOUT;
}

void await_fences(VkDevice device, stx::Span<VkFence const> const& fences) {
  VLK_MUST_SUCCEED(vkWaitForFences(device, fences.size(), fences.data(), true,
                                   std::numeric_limits<uint64_t>::max()),
                   "Unable to await fences");
}

// returns true if the fences didn't timeout
[[nodiscard]] bool await_fences(VkDevice device,
                                stx::Span<VkFence const> const& fences,
                                std::chrono::nanoseconds timeout) {
  auto result = vkWaitForFences(device, fences.size(), fences.data(), true,
                                timeout.count());
  VLK_MUST_SUCCEED(
      result == VK_SUCCESS || result == VK_TIMEOUT ? VK_SUCCESS : result,
      "Unable to await fences");

  return result != VK_TIMEOUT;
}

void submit_commands(VkQueue command_queue, VkCommandBuffer command_buffer,
                     stx::Span<VkSemaphore const> const& await_semaphores,
                     stx::Span<VkPipelineStageFlags const> const& await_stages,
                     stx::Span<VkSemaphore const> const& notify_semaphores,
                     VkFence notify_fence) {
  VLK_ENSURE(await_semaphores.size() == await_stages.size(),
             "stages to await must have the same number of semaphores (for "
             "each of them)");

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  submit_info.waitSemaphoreCount = await_semaphores.size();
  submit_info.pWaitSemaphores = await_semaphores.data();

  submit_info.pWaitDstStageMask = await_stages.data();

  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  submit_info.signalSemaphoreCount = notify_semaphores.size();
  submit_info.pSignalSemaphores = notify_semaphores.data();

  VLK_MUST_SUCCEED(vkQueueSubmit(command_queue, 1, &submit_info, notify_fence),
                   "Unable to submit command buffer to command queue");
}

[[nodiscard]] VkResult present_to_swapchains(
    VkQueue command_queue, stx::Span<VkSemaphore const> const& await_semaphores,
    stx::Span<VkSwapchainKHR const> const& swapchains,
    stx::Span<uint32_t const> const& swapchain_image_indexes) {
  VLK_ENSURE(swapchain_image_indexes.size() == swapchains.size(),
             "swapchain and their image indices must be of the same size");

  VkPresentInfoKHR present_info{};

  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  present_info.waitSemaphoreCount = await_semaphores.size();
  present_info.pWaitSemaphores = await_semaphores.data();

  present_info.swapchainCount = swapchains.size();
  present_info.pSwapchains = swapchains.data();

  present_info.pImageIndices = swapchain_image_indexes.data();

  present_info.pResults = nullptr;

  auto present_result = vkQueuePresentKHR(command_queue, &present_info);
  VLK_ENSURE(present_result == VK_SUCCESS ||
                 present_result == VK_SUBOPTIMAL_KHR ||
                 present_result == VK_ERROR_OUT_OF_DATE_KHR,
             "Unable to present to swapchain");

  return present_result;
}

// creates buffer object but doesn't assign memory to it
VkBuffer create_buffer(VkDevice device, uint64_t byte_size,
                       VkBufferUsageFlagBits usage,
                       VkSharingMode sharing_mode) {
  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = byte_size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = sharing_mode;

  VkBuffer buffer;
  VLK_MUST_SUCCEED(vkCreateBuffer(device, &buffer_info, nullptr, &buffer),
                   "Unable to create buffer");
  return buffer;
}

// get memory requirements for a buffer based on it's type and usage mode
VkMemoryRequirements get_memory_requirements(VkDevice device, VkBuffer buffer) {
  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);
  return memory_requirements;
}

// get memory requirements for a buffer based on it's type and usage mode
VkMemoryRequirements get_memory_requirements(VkDevice device, VkImage image) {
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(device, image, &memory_requirements);
  return memory_requirements;
}

// returns index of the heap on the physical device, could be RAM, SWAP, or VRAM
stx::Option<uint32_t> find_suitable_memory_type(
    VkPhysicalDevice physical_device,
    VkMemoryRequirements const& memory_requirements,
    VkMemoryPropertyFlagBits required_properties =
        static_cast<VkMemoryPropertyFlagBits>(
            std::numeric_limits<
                std::underlying_type_t<VkMemoryPropertyFlagBits>>::max())) {
  VkPhysicalDeviceMemoryProperties memory_properties;

  vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
  // different types of memory exist within the graphics card heap memory.
  // this can affect performance.

  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
    if ((memory_requirements.memoryTypeBits & (1 << i)) &&
        ((required_properties &
          memory_properties.memoryTypes[i].propertyFlags) ==
         required_properties)) {
      return stx::Some(std::move(i));
    }
  }

  return stx::None;
}

// vkFreeMemory
VkDeviceMemory allocate_memory(VkDevice device, uint32_t heap_index,
                               uint64_t size) {
  VkMemoryAllocateInfo allocate_info{};

  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = size;
  allocate_info.memoryTypeIndex = heap_index;

  VkDeviceMemory memory;
  VLK_MUST_SUCCEED(vkAllocateMemory(device, &allocate_info, nullptr, &memory),
                   "Unable to allocate memory");

  return memory;
}

void bind_memory_to_buffer(VkDevice device, VkBuffer buffer,
                           VkDeviceMemory memory, uint64_t offset) {
  VLK_MUST_SUCCEED(vkBindBufferMemory(device, buffer, memory, offset),
                   "Unable to bind memory to buffer");
}

void bind_memory_to_image(VkDevice device, VkImage image, VkDeviceMemory memory,
                          uint64_t offset) {
  VLK_MUST_SUCCEED(vkBindImageMemory(device, image, memory, offset),
                   "Unable to bind memory to image");
}

struct MemoryMap {
  // offset of the memory address this map points to
  uint64_t offset;
  // contains the (offset+adress) and size
  stx::Span<uint8_t volatile> span;
};

MemoryMap map_memory(VkDevice device, VkDeviceMemory memory, uint64_t offset,
                     uint64_t size, VkMemoryMapFlags flags = 0) {
  void* ptr;
  VLK_MUST_SUCCEED(vkMapMemory(device, memory, offset, size, flags, &ptr),
                   "Unable to map memory");
  return MemoryMap{offset,
                   stx::Span<uint8_t>{reinterpret_cast<uint8_t*>(ptr), size}};
}

// unlike OpenGL the driver may not immediately copy the data after unmap, i.e.
// due to caching. so we need to flush our writes
void unmap_memory(VkDevice device, VkDeviceMemory memory) {
  vkUnmapMemory(device, memory);
}

// due to caching we need to flush writes to the memory map before reading again
// has size requirements for the flush range
void flush_memory_map(VkDevice device, VkDeviceMemory memory, uint64_t offset,
                      uint64_t size) {
  VkMappedMemoryRange range{};
  range.memory = memory;
  range.offset = offset;
  range.size = size;
  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

  VLK_MUST_SUCCEED(vkFlushMappedMemoryRanges(device, 1, &range),
                   "Unable to flush memory map");
}

void refresh_memory_map(VkDevice device, VkDeviceMemory memory, uint64_t offset,
                        uint64_t size) {
  VkMappedMemoryRange range{};
  range.memory = memory;
  range.offset = offset;
  range.size = size;
  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

  VLK_MUST_SUCCEED(vkInvalidateMappedMemoryRanges(device, 1, &range),
                   "Unable to re-read memory map");
}

}  // namespace vlk

// TODO(lamarrr): Go through the tutorial and comment into this code any
// subtlety/important points
