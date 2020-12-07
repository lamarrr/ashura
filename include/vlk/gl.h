
#pragma once

#include <algorithm>
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
    VLK_LOG("\t" << extension.extensionName
                 << ", spec version: " << extension.specVersion);
  }

  create_info.enabledExtensionCount = required_extensions.size();
  create_info.ppEnabledExtensionNames = required_extensions.data();

  // debug message callback extension

#if VLK_DEBUG
  // validation layers
  ensure_validation_layers_supported(required_validation_layers);
  create_info.enabledLayerCount = std::size(required_validation_layers);
  create_info.ppEnabledLayerNames = required_validation_layers.data();

  // debug messenger for when the installed debug messenger is uninstalled
  create_info.pNext =
      reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT const*>(
          default_debug_messenger_create_info);

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

    VLK_LOG("\t" << name_physical_device(device_properties)
                 << " (geometry shader: " << device_features.geometryShader
                 << ")");
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
                [](char const* ext) { VLK_LOG("\t" << ext); });

  VLK_LOG("Available Device Extensions: ");
  std::for_each(available_device_extensions.begin(),
                available_device_extensions.end(),
                [](VkExtensionProperties ext) {
                  VLK_LOG("\t" << ext.extensionName
                               << "(spec version: " << ext.specVersion << ")");
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
    VkSpecializationInfo const& program_constants = {}) {
  VkPipelineShaderStageCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  create_info.module = module;
  create_info.pName = program_entry_point;
  create_info.stage = pipeline_stage_flag;
  create_info.pNext = nullptr;
  create_info.pSpecializationInfo =
      &program_constants;  // provide constants used within the shader

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

// specify how many color and depth buffers there will be, how many samples to
// use for each of them and how their contents should be handled throughout the
// rendering operations (and the subpasses description)
[[nodiscard]] VkRenderPass create_render_pass(
    VkDevice device,
    stx::Span<VkAttachmentDescription const> const& attachment_descriptions,
    stx::Span<VkSubpassDescription const> const& subpass_descriptions) {
  VkRenderPassCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  create_info.attachmentCount = attachment_descriptions.size();
  create_info.pAttachments = attachment_descriptions.data();
  create_info.subpassCount = subpass_descriptions.size();
  create_info.pSubpasses = subpass_descriptions.data();
  VkRenderPass render_pass;
  VLK_MUST_SUCCEED(
      vkCreateRenderPass(device, &create_info, nullptr, &render_pass),
      "Unable to create render pass");

  return render_pass;
}

}  // namespace vlk
