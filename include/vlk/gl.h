
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

using DevicePropFt = std::tuple<VkPhysicalDevice, VkPhysicalDeviceProperties,
                                VkPhysicalDeviceFeatures>;

static auto create_vk_instance(
    VkDebugUtilsMessengerCreateInfoEXT* default_debug_messenger_create_info,
    stx::Span<char const* const> required_validation_layers)
    -> stx::Result<VkInstance, VkResult> {
  // helps bnt not necessary
  VkApplicationInfo app_info;
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Valkyrie";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Valkyrie Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  // get list of extensions required for vulkan interfacing with the window
  // system
  uint32_t glfw_req_extensions_count = 0;
  char const** glfw_req_extensions_names;

  glfw_req_extensions_names =
      glfwGetRequiredInstanceExtensions(&glfw_req_extensions_count);

  VLK_LOG("Required GLFW Extensions:");
  for (size_t i = 0; i < glfw_req_extensions_count; i++) {
    VLK_LOG("\t" << glfw_req_extensions_names[i]);
  }

  uint32_t available_vk_extensions_count = 0;
  vkEnumerateInstanceExtensionProperties(
      nullptr, &available_vk_extensions_count, nullptr);

  std::vector<VkExtensionProperties> available_vk_extensions(
      available_vk_extensions_count);
  vkEnumerateInstanceExtensionProperties(
      nullptr, &available_vk_extensions_count, available_vk_extensions.data());

  std::vector<char const*> required_extensions;
  // TODO(lamarrr): deduction guides
  for (auto extension : stx::Span<char const*>(glfw_req_extensions_names,
                                               glfw_req_extensions_count)) {
    required_extensions.push_back(extension);
  }

#if VLK_DEBUG
  required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

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
  *default_debug_messenger_create_info = make_debug_messenger_create_info();
  create_info.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(
      default_debug_messenger_create_info);

#endif

  VkInstance vk_instance;
  VkResult result = vkCreateInstance(&create_info, nullptr, &vk_instance);

  if (result != VK_SUCCESS) {
    return stx::Err(std::move(result));
  }

  return stx::Ok(std::move(vk_instance));
}

constexpr VkBool32 device_gt_eq(DevicePropFt const& a, DevicePropFt const& b) {
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

constexpr VkBool32 device_lt(DevicePropFt const& a, DevicePropFt const& b) {
  return !device_gt_eq(a, b);
}

static std::string name_physical_device(VkPhysicalDeviceProperties properties) {
  std::string name = properties.deviceName;

  name += "(id: " + std::to_string(properties.deviceID) + ", type: ";

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

  return std::move(name);
}

static std::vector<DevicePropFt> get_physical_devices(VkInstance vk_instance) {
  uint32_t devices_count = 0;

  vkEnumeratePhysicalDevices(vk_instance, &devices_count, nullptr);

  VLK_ENSURE(devices_count != 0, "No Physical Device Found");

  std::vector<VkPhysicalDevice> physical_devices(devices_count);
  vkEnumeratePhysicalDevices(vk_instance, &devices_count,
                             physical_devices.data());

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

  return std::move(device_prop_ft);
}

// selects GPU, in the following preference order: dGPU => vGPU => iGPU => CPU
static DevicePropFt most_suitable_physical_device(
    VkInstance vk_instance, stx::Span<DevicePropFt> physical_devices,
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
std::vector<VkQueueFamilyProperties> get_queue_families(
    VkPhysicalDevice device) {
  uint32_t queue_families_count;

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queue_families_properties(
      queue_families_count);

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count,
                                           queue_families_properties.data());

  return std::move(queue_families_properties);
}

stx::Option<uint32_t> find_queue_family(
    VkPhysicalDevice device, stx::Span<VkQueueFamilyProperties> queue_families,
    VkQueueFlagBits required_queue_families) {
  auto req_queue_family =
      std::find_if(queue_families.begin(), queue_families.end(),
                   [=](VkQueueFamilyProperties const& fam_props) -> bool {
                     return fam_props.queueFlags & required_queue_families;
                   });

  if (req_queue_family == queue_families.end()) return stx::None;

  return stx::Some(
      static_cast<uint32_t>(req_queue_family - queue_families.begin()));
}

VkDevice create_logical_device(
    VkPhysicalDevice physical_device, uint32_t queue_family_index,
    stx::Span<char const* const> required_extensions,
    stx::Span<char const* const> required_validation_layers,
    VkAllocationCallbacks const* allocation_callback) {
  VkDeviceQueueCreateInfo queue_create_info{};
  queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = queue_family_index;
  queue_create_info.queueCount = 1;  // we only need one queue => GRAPHICS queue

  float queue_priority = 1.0f;
  // influence the scheduling of command buffer execution
  queue_create_info.pQueuePriorities = &queue_priority;

  // required features
  VkPhysicalDeviceFeatures device_features{};

  VkDeviceCreateInfo device_create_info{};
  device_create_info.pQueueCreateInfos = &queue_create_info;
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.pEnabledFeatures = &device_features;

  uint32_t available_extensions_count;
  vkEnumerateDeviceExtensionProperties(physical_device, nullptr,
                                       &available_extensions_count, nullptr);

  // device specific extensions
  std::vector<VkExtensionProperties> available_device_extensions(
      available_extensions_count);

  vkEnumerateDeviceExtensionProperties(physical_device, nullptr,
                                       &available_extensions_count,
                                       available_device_extensions.data());

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
  device_create_info.enabledLayerCount = required_validation_layers.size();
  device_create_info.ppEnabledLayerNames = required_validation_layers.data();

  device_create_info.enabledExtensionCount = required_extensions.size();
  device_create_info.ppEnabledExtensionNames = required_extensions.data();

  VkDevice logical_device;

  VLK_ENSURE(vkCreateDevice(physical_device, &device_create_info,
                            allocation_callback, &logical_device) == VK_SUCCESS,
             "Unable to Create Physical Device");

  return logical_device;
}

struct SwapChainProperties {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> supported_formats;
  std::vector<VkPresentModeKHR> presentation_modes;
};

SwapChainProperties get_swapchain_properties(VkPhysicalDevice physical_device,
                                             VkSurfaceKHR surface) {
  SwapChainProperties details{};

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                            &details.capabilities);

  uint32_t supported_surface_formats_count = 0;

  vkGetPhysicalDeviceSurfaceFormatsKHR(
      physical_device, surface, &supported_surface_formats_count, nullptr);

  details.supported_formats.resize(supported_surface_formats_count);

  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                       &supported_surface_formats_count,
                                       details.supported_formats.data());

  uint32_t surface_presentation_modes_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device, surface, &surface_presentation_modes_count, nullptr);

  details.presentation_modes.resize(surface_presentation_modes_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                            &surface_presentation_modes_count,
                                            details.presentation_modes.data());

  return std::move(details);
}

bool is_swapchain_adequate(SwapChainProperties const& details) {
  // we use any available for selecting devices
  VLK_ENSURE(details.supported_formats.size() != 0,
             "Physical Device does not support any window surface "
             "format");

  VLK_ENSURE(details.presentation_modes.size() != 0,
             "Physical Device does not support any window surface "
             "presentation mode");

  return true;
}

// choose a specific surface format available on the GPU
VkSurfaceFormatKHR select_surface_formats(
    stx::Span<VkSurfaceFormatKHR const> formats) {
  VLK_ENSURE(formats.size() > 0, "No window surface format gotten as arg");
  auto It_format = std::find_if(
      formats.begin(), formats.end(), [](VkSurfaceFormatKHR const& format) {
        return format.format == VK_FORMAT_R8G8B8_SRGB &&
               format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      });

  return (It_format != formats.end()) ? *It_format : formats[0];
}

VkPresentModeKHR select_surface_presentation_mode(
    stx::Span<VkPresentModeKHR const> available_presentation_modes) {
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

  auto mode = std::find(available_presentation_modes.begin(),
                        available_presentation_modes.end(),
                        VK_PRESENT_MODE_MAILBOX_KHR);
  if (mode != available_presentation_modes.end()) return *mode;
  VLK_WARN("Device does not support the Mailbox surface presentation mode");

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D select_swapchain_extent(
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
