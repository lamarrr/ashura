
#pragma once

#include <algorithm>
#include <chrono>
#include <limits>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "stx/backtrace.h"
#include "stx/limits.h"
#include "stx/option.h"
#include "stx/result.h"
#include "stx/span.h"
#include "vlk/utils.h"
#include "vulkan/vulkan.h"

namespace vlk {

using namespace std::chrono_literals;

namespace vk {

template <typename T>
inline auto join_copy(stx::Span<T> a, stx::Span<T> b) {
  std::vector<std::remove_const_t<T>> x;
  x.reserve(a.size() + b.size());

  for (auto const& el : a) x.push_back(el);
  for (auto const& el : b) x.push_back(el);

  return x;
}

inline void ensure_validation_layers_supported(
    stx::Span<char const* const> layers) {
  uint32_t available_validation_layers_count;
  vkEnumerateInstanceLayerProperties(&available_validation_layers_count,
                                     nullptr);

  std::vector<VkLayerProperties> available_validation_layers(
      available_validation_layers_count);

  VLK_LOG("Available Vulkan Validation Layers:");

  vkEnumerateInstanceLayerProperties(&available_validation_layers_count,
                                     available_validation_layers.data());

  for (VkLayerProperties const& layer : available_validation_layers) {
    VLK_LOG("\t{} (spec version: {})", layer.layerName, layer.specVersion);
  }

  bool all_layers_available = true;

  for (auto const req_layer : layers) {
    if (std::find_if(available_validation_layers.begin(),
                     available_validation_layers.end(),
                     [&req_layer](VkLayerProperties const& available_layer) {
                       return std::string_view(req_layer) ==
                              std::string_view(available_layer.layerName);
                     }) == available_validation_layers.end()) {
      all_layers_available = false;
      VLK_WARN("Required validation layer `{}` is not available",
               std::string_view(req_layer));
    }
  }

  VLK_ENSURE(all_layers_available,
             "One or more required validation layers are not available");
}

// NICE-TO-HAVE(lamarrr): versioning of extensions, know which one wasn't
// available and adjust features to that
inline void ensure_extensions_supported(stx::Span<char const* const> names) {
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

  bool all_available = true;

  for (auto const name : names) {
    if (std::find_if(available_vk_extensions.begin(),
                     available_vk_extensions.end(),
                     [&name](VkExtensionProperties const& props) {
                       return std::string_view(name) ==
                              std::string_view(props.extensionName);
                     }) == available_vk_extensions.end()) {
      all_available = false;
      VLK_WARN("Required extension `{}` is not available",
               std::string_view(name));
    }
  }

  VLK_ENSURE(all_available,
             "One or more required extensions are not available");
}

inline VkBool32 VKAPI_ATTR VKAPI_CALL default_debug_callback(
    [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
    [[maybe_unused]] void* user_data) {
  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_*_BIT_EXT are bit flags that indicate if
  // the message is important enough to show

  // you can use comparisions like messageSeverity >=
  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT to see if they are
  // important or not

  std::string hint;
  if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    hint +=
        "Specification violation or possible mistake "
        "detected";
  }

  if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
    hint += std::string(hint.empty() ? "" : ", ") +
            "Potential non-optimal use of Vulkan "
            "detected";
  }

  bool const is_general =
      message_type == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  if (hint.empty()) {
    VLK_LOG_IF(is_general, "[Validation Layer Message] {}",
               callback_data->pMessage);
    VLK_WARN_IF(!is_general, "[Validation Layer Message] {}",
                callback_data->pMessage);
  } else {
    VLK_LOG_IF(is_general, "[Validation Layer Message, Hints=\"{}\"] {}", hint,
               callback_data->pMessage);
    VLK_WARN_IF(!is_general, "[Validation Layer Message, Hints=\"{}\"] {}",
                hint, callback_data->pMessage);
  }

  if (!is_general) {
    VLK_LOG("Call Stack:");
    stx::backtrace::trace(
        [](stx::backtrace::Frame frame, int) {
          VLK_LOG("\t=> {}", frame.symbol.clone().match(
                                 [](auto sym) { return sym.raw(); },
                                 []() { return std::string_view("unknown"); }));
          return false;
        },
        2);
  }

  return VK_FALSE;
}

inline VkDebugUtilsMessengerCreateInfoEXT make_debug_messenger_create_info() {
  VkDebugUtilsMessengerCreateInfoEXT create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = default_debug_callback;
  create_info.pUserData = nullptr;

  return create_info;
}

inline VkDebugUtilsMessengerEXT create_install_debug_messenger(
    VkInstance instance, VkAllocationCallbacks const* allocator,
    VkDebugUtilsMessengerCreateInfoEXT create_info =
        make_debug_messenger_create_info()) {
  VkDebugUtilsMessengerEXT debug_messenger;

  auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

  VLK_ENSURE(
      func != nullptr,
      "Unable to get process address for vkCreateDebugUtilsMessengerEXT");

  VLK_MUST_SUCCEED(func(instance, &create_info, allocator, &debug_messenger),
                   "Unable to setup debug messenger");

  return debug_messenger;
}

inline void destroy_debug_messenger(VkInstance instance,
                                    VkDebugUtilsMessengerEXT debug_messenger,
                                    VkAllocationCallbacks const* allocator) {
  auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

  VLK_ENSURE(func != nullptr, "Unable to destroy debug messenger");

  return func(instance, debug_messenger, allocator);
}

// terminology: every object created using a `create_*` requires a `vkDestroy`
// call.
// `make_*` returns plain structs that could possibly contain immutable
// view of data.

inline std::pair<VkInstance, VkDebugUtilsMessengerEXT> create_vulkan_instance(
    stx::Span<char const* const> required_extensions,
    stx::Span<char const* const> required_validation_layers,
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info,
    char const* const application_name = "Valkyrie",
    uint32_t application_version = VK_MAKE_VERSION(1, 0, 0),
    char const* const engine_name = "Valkyrie Engine",
    uint32_t engine_version = VK_MAKE_VERSION(1, 0, 0)) noexcept {
  // helps bnt not necessary
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = application_name;
  app_info.applicationVersion = application_version;
  app_info.pEngineName = engine_name;
  app_info.engineVersion = engine_version;
  app_info.apiVersion = VK_API_VERSION_1_1;
  app_info.pNext = nullptr;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.pNext = nullptr;

  static constexpr char const* debug_extensions[] = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

  // TODO(lamarrr): check that the requested extensions are available
  // debug message callback extension
  auto extensions =
      join_copy(required_extensions,
                required_validation_layers.empty()
                    ? stx::Span<char const* const>{}
                    : stx::Span<char const* const>(debug_extensions));

  create_info.enabledExtensionCount = extensions.size();
  create_info.ppEnabledExtensionNames = extensions.data();

  ensure_extensions_supported(extensions);

  if (!required_validation_layers.empty()) {
    // validation layers
    ensure_validation_layers_supported(required_validation_layers);
    create_info.enabledLayerCount = required_validation_layers.size();
    create_info.ppEnabledLayerNames = required_validation_layers.data();

    // debug messenger for when the installed debug messenger is uninstalled.
    // this helps to debug issues with vkDestroyInstance and vkCreateInstance
    // i.e. (before and after the debug messenger is installed)
    create_info.pNext = &debug_messenger_create_info;
  }

  VkInstance vulkan_instance;
  VLK_MUST_SUCCEED(vkCreateInstance(&create_info, nullptr, &vulkan_instance),
                   "Unable to create vulkan instance");

  VkDebugUtilsMessengerEXT messenger = nullptr;

  if (!required_validation_layers.empty()) {
    messenger = create_install_debug_messenger(vulkan_instance, nullptr,
                                               debug_messenger_create_info);
  }

  return std::make_pair(vulkan_instance, messenger);
}

//  to do anything on the GPU (render, draw, compute, allocate memory, create
//  texture, etc.) we use command queues
inline std::vector<VkQueueFamilyProperties> get_queue_families(
    VkPhysicalDevice device) noexcept {
  uint32_t queue_families_count;

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queue_families_properties(
      queue_families_count);

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count,
                                           queue_families_properties.data());

  return queue_families_properties;
}

inline std::vector<bool> get_command_queue_support(
    stx::Span<VkQueueFamilyProperties const> queue_families,
    VkQueueFlagBits required_command_queue) {
  std::vector<bool> supports;

  for (auto const& fam_props : queue_families) {
    supports.push_back(fam_props.queueFlags & required_command_queue);
  }

  return supports;
}

// find the device's queue family capable of supporting surface presentation
inline std::vector<bool> get_surface_presentation_command_queue_support(
    VkPhysicalDevice physical_device,
    stx::Span<VkQueueFamilyProperties const> queue_families,
    VkSurfaceKHR surface) noexcept {
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

inline VkDevice create_logical_device(
    VkPhysicalDevice physical_device,
    stx::Span<char const* const> required_extensions,
    stx::Span<char const* const> required_validation_layers,
    stx::Span<VkDeviceQueueCreateInfo const> command_queue_create_infos,
    VkAllocationCallbacks const* allocation_callback,
    VkPhysicalDeviceFeatures const& required_features = {}) noexcept {
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

inline VkQueue get_command_queue(
    VkDevice device, uint32_t queue_family_index,
    uint32_t command_queue_index_in_family) noexcept {
  VkQueue command_queue;
  vkGetDeviceQueue(device, queue_family_index, command_queue_index_in_family,
                   &command_queue);
  VLK_ENSURE(command_queue != nullptr,
             "Requested command queue not created on target device");
  return command_queue;
}

struct SwapChainProperties {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> supported_formats;
  std::vector<VkPresentModeKHR> presentation_modes;
};

inline SwapChainProperties get_swapchain_properties(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface) noexcept {
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

inline bool is_swapchain_adequate(
    SwapChainProperties const& properties) noexcept {
  // we use any available for selecting devices
  VLK_ENSURE(properties.supported_formats.size() != 0,
             "Physical Device does not support any window surface "
             "format");

  VLK_ENSURE(properties.presentation_modes.size() != 0,
             "Physical Device does not support any window surface "
             "presentation mode");

  return true;
}

inline VkExtent2D select_swapchain_extent(
    VkSurfaceCapabilitiesKHR const& capabilities,
    VkExtent2D desired_extent) noexcept {
  // this, unlike the window dimensions is in pixels and is the rendered to
  // area
  //
  //
  //
  // if {capabilities.currentExtent} is already set (value other than u32_max)
  // then we are not allowed to choose the extent and we must use the provided
  // extent. otherwise, we a range of extents will be provided that we must
  // clamp to.
  if (capabilities.currentExtent.width != stx::u32_max ||
      capabilities.currentExtent.height != stx::u32_max) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D target_extent =
        VkExtent2D{desired_extent.width, desired_extent.height};

    target_extent.width =
        std::clamp(target_extent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    target_extent.height =
        std::clamp(target_extent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return target_extent;
  }
}

// select number of images to have on the swap chain based on device
// capabilities. i.e. double buffering, triple buffering.
inline uint32_t select_swapchain_image_count(
    VkSurfaceCapabilitiesKHR const& capabilities,
    uint32_t desired_num_buffers) {
  return
      // no limit on the number of swapchain images
      capabilities.maxImageCount == 0
          ? std::clamp(desired_num_buffers, capabilities.minImageCount,
                       stx::u32_max)
          : std::clamp(desired_num_buffers, capabilities.minImageCount,
                       capabilities.maxImageCount);
}

inline std::pair<VkSwapchainKHR, VkExtent2D> create_swapchain(
    VkDevice device, VkSurfaceKHR surface, VkExtent2D extent,
    VkSurfaceFormatKHR surface_format, VkPresentModeKHR present_mode,
    SwapChainProperties const& properties,
    VkSharingMode accessing_queue_families_sharing_mode,
    stx::Span<uint32_t const> accessing_queue_families_indexes,
    VkImageUsageFlags image_usages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    VkCompositeAlphaFlagBitsKHR alpha_channel_blending =
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    VkBool32 clipped = VK_TRUE) noexcept {
  VkSwapchainCreateInfoKHR create_info{};

  uint32_t desired_num_buffers = properties.capabilities.minImageCount + 1;

  VkExtent2D selected_extent =
      select_swapchain_extent(properties.capabilities, extent);

  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.imageExtent = selected_extent;
  create_info.surface = surface;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.presentMode = present_mode;

  // number of images to use for buffering on the swapchain
  create_info.minImageCount = select_swapchain_image_count(
      properties.capabilities, desired_num_buffers);

  create_info.imageArrayLayers = 1;  // 2 for stereoscopic rendering
  create_info.imageUsage = image_usages;
  create_info.preTransform = properties.capabilities.currentTransform;
  create_info.compositeAlpha =
      alpha_channel_blending;  // how the alpha channel should be
                               // used for blending with other
                               // windows in the window system

  // clipped specifies whether the Vulkan implementation is allowed to discard
  // rendering operations that affect regions of the surface that are not
  // visible.
  // If set to VK_TRUE, the presentable images associated with the swapchain may
  // not own all of their pixels. Pixels in the presentable images that
  // correspond to regions of the target surface obscured by another window on
  // the desktop, or subject to some other clipping mechanism will have
  // undefined content when read back. Fragment shaders may not execute for
  // these pixels, and thus any side effects they would have had will not occur.
  // Setting VK_TRUE does not guarantee any clipping will occur, but allows more
  // efficient presentation methods to be used on some platforms. If set to
  // VK_FALSE, presentable images associated with the swapchain will own all of
  // the pixels they contain.
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

  return std::make_pair(swapchain, selected_extent);
}

inline std::vector<VkImage> get_swapchain_images(
    VkDevice device, VkSwapchainKHR swapchain) noexcept {
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
// this will create `queue_priorities.size()` number of command queues from
// family `queue_family_index`
constexpr VkDeviceQueueCreateInfo make_command_queue_create_info(
    uint32_t queue_family_index,
    stx::Span<float const> queues_priorities) noexcept {
  VkDeviceQueueCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  create_info.queueFamilyIndex = queue_family_index;
  create_info.pQueuePriorities = queues_priorities.data();
  create_info.queueCount =
      queues_priorities.size();  // the number of queues we want, since multiple
                                 // queues can belong to a single family

  return create_info;
}

constexpr VkComponentMapping make_default_component_mapping() noexcept {
  // how to map the image color components
  VkComponentMapping mapping{};
  mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;  // leave as-is
  mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  return mapping;
}

inline VkImageView create_image_view(
    VkDevice device, VkImage image, VkFormat format, VkImageViewType view_type,
    VkImageAspectFlagBits aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT,
    VkComponentMapping component_mapping =
        make_default_component_mapping()) noexcept {
  VkImageViewCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  create_info.image = image;

  // VK_IMAGE_VIEW_TYPE_2D: 2D texture
  // VK_IMAGE_VIEW_TYPE_3D: 3D texture
  // VK_IMAGE_VIEW_TYPE_CUBE: cube map
  create_info.viewType = view_type;  // treat the image as a 2d texture
  create_info.format = format;

  create_info.components = component_mapping;

  // defines what part of the image this image view represents and what this
  // image view is used for
  create_info.subresourceRange.aspectMask = aspect_mask;
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

inline VkSampler create_sampler(VkDevice device,
                                stx::Option<float> max_anisotropy) noexcept {
  VkSamplerCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

  // for treating the case where there are more fragments than texels
  create_info.magFilter = VK_FILTER_LINEAR;
  create_info.minFilter = VK_FILTER_LINEAR;

  // VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the
  // image dimensions.
  // VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but
  // inverts the coordinates to mirror the image when going beyond the
  // dimensions.
  // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the color of the
  // edge closest to the coordinate beyond the image dimensions.
  // VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: Like clamp to edge, but
  // instead uses the edge opposite to the closest edge.
  // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Return a solid color when sampling
  // beyond the dimensions of the image.

  // u, v, w coordinate overflow style of the textures
  // this shouldn't affect the texture if we are not sampling outside of the
  // image
  create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  // for treating the case where there are more texels than fragments
  create_info.anisotropyEnable = max_anisotropy.is_some();
  create_info.maxAnisotropy =
      max_anisotropy.is_some() ? max_anisotropy.clone().unwrap() : 0.0f;

  create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  create_info.unnormalizedCoordinates =
      VK_FALSE;  // coordinates matching the sampled image will be normalized to
                 // the (0.0 to 1.0 range) otherwise in the (0, image width or
                 // height range)

  create_info.compareEnable = VK_FALSE;
  create_info.compareOp = VK_COMPARE_OP_ALWAYS;

  // mip-mapping
  create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  create_info.mipLodBias = 0.0f;
  create_info.minLod = 0.0f;
  create_info.maxLod = 0.0f;

  VkSampler sampler;

  VLK_MUST_SUCCEED(vkCreateSampler(device, &create_info, nullptr, &sampler),
                   "Unable to create sampler");

  return sampler;
}

inline VkShaderModule create_shader_module(
    VkDevice device, stx::Span<uint32_t const> spirv_byte_data) noexcept {
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

constexpr VkPipelineShaderStageCreateInfo
make_pipeline_shader_stage_create_info(
    VkShaderModule module, char const* program_entry_point,
    VkShaderStageFlagBits pipeline_stage_flag,
    VkSpecializationInfo const* program_constants) noexcept {
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

constexpr VkPipelineShaderStageCreateInfo
make_pipeline_shader_stage_create_info(
    VkShaderModule module, char const* program_entry_point,
    VkShaderStageFlagBits pipeline_stage_flag) noexcept {
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

constexpr VkPipelineVertexInputStateCreateInfo
make_pipeline_vertex_input_state_create_info(
    stx::Span<VkVertexInputBindingDescription const>
        vertex_binding_descriptions,
    stx::Span<VkVertexInputAttributeDescription const>
        vertex_attribute_desciptions) noexcept {
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

constexpr VkPipelineInputAssemblyStateCreateInfo
make_pipeline_input_assembly_state_create_info() noexcept {
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

constexpr VkViewport make_viewport(float x, float y, float w, float h,
                                   float min_depth = 0.0f,
                                   float max_depth = 1.0f) noexcept {
  VkViewport viewport{};
  viewport.x = x;
  viewport.y = y;
  viewport.width = w;             // width of the framebuffer (swapchain image)
  viewport.height = h;            // height of the framebuffer (swapchain image)
  viewport.minDepth = min_depth;  // min depth value to use for the frame buffer
  viewport.maxDepth = max_depth;  // max depth value to use for the frame buffer

  return viewport;
}

constexpr VkRect2D make_scissor(float x, float y, float w, float h) noexcept {
  VkRect2D scissor{};

  scissor.offset.x = x;
  scissor.offset.y = y;

  scissor.extent.width = w;
  scissor.extent.height = h;

  return scissor;
}

constexpr VkPipelineViewportStateCreateInfo
make_pipeline_viewport_state_create_info(
    stx::Span<VkViewport const> viewports,
    stx::Span<VkRect2D const> scissors) noexcept {
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

constexpr VkPipelineRasterizationStateCreateInfo
make_pipeline_rasterization_create_info(VkFrontFace front_face,
                                        float line_width = 1.0f) noexcept {
  VkPipelineRasterizationStateCreateInfo create_info{};
  create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  create_info.depthClampEnable =
      VK_FALSE;  // ragments that are beyond the near and far planes are clamped
                 // to them as opposed to discarding them. This is useful in
                 // some special cases like shadow maps. Using this requires
                 // enabling a GPU feature.
  create_info.rasterizerDiscardEnable =
      VK_FALSE;  // if true, geometry never passes through the rasterization
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
  create_info.frontFace = front_face;

  create_info.depthBiasEnable = VK_FALSE;
  create_info.depthBiasConstantFactor = 0.0f;  // mostly used for shadow mapping
  create_info.depthBiasClamp = 0.0f;
  create_info.depthBiasSlopeFactor = 0.0f;

  return create_info;
}

constexpr VkPipelineMultisampleStateCreateInfo
make_pipeline_multisample_state_create_info() noexcept {
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

constexpr VkPipelineDepthStencilStateCreateInfo
make_pipeline_depth_stencil_state_create_info() noexcept {
  VkPipelineDepthStencilStateCreateInfo create_info{};
  // VLK_ENSURE(false, "Unimplemented");
  return create_info;
}

// per framebuffer
constexpr VkPipelineColorBlendAttachmentState
make_pipeline_color_blend_attachment_state() noexcept {
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
constexpr VkPipelineColorBlendStateCreateInfo
make_pipeline_color_blend_state_create_info(
    stx::Span<VkPipelineColorBlendAttachmentState const>
        color_frame_buffers) noexcept {
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

constexpr VkPipelineDynamicStateCreateInfo make_pipeline_dynamic_state(
    stx::Span<VkDynamicState const> dynamic_states) noexcept {
  // This will cause the configuration of these values to be ignored and you
  // will be required to specify the data at drawing time. This struct can be
  // substituted by a nullptr later on if you don't have any dynamic state.

  VkPipelineDynamicStateCreateInfo pipeline_state{};
  pipeline_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  pipeline_state.dynamicStateCount = std::size(dynamic_states);
  pipeline_state.pDynamicStates = dynamic_states.data();

  return pipeline_state;
}

inline VkPipelineLayout create_pipeline_layout(
    VkDevice device,
    stx::Span<VkDescriptorSetLayout const> descriptor_sets_layout = {},
    stx::Span<VkPushConstantRange const> constant_ranges = {}) noexcept {
  VkPipelineLayoutCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  create_info.setLayoutCount = descriptor_sets_layout.size();
  create_info.pSetLayouts = descriptor_sets_layout.data();
  create_info.pushConstantRangeCount = constant_ranges.size();
  create_info.pPushConstantRanges = constant_ranges.data();

  VkPipelineLayout layout;
  VLK_MUST_SUCCEED(
      vkCreatePipelineLayout(device, &create_info, nullptr, &layout),
      "Unable to create pipeline layout");

  return layout;
}

constexpr VkAttachmentDescription make_attachment_description(
    VkFormat format) noexcept {
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
constexpr VkSubpassDescription make_subpass_description(
    stx::Span<VkAttachmentReference const> color_attachments) noexcept {
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
constexpr VkSubpassDependency make_subpass_dependency() noexcept {
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
inline VkRenderPass create_render_pass(
    VkDevice device,
    stx::Span<VkAttachmentDescription const> attachment_descriptions,
    stx::Span<VkSubpassDescription const> subpass_descriptions,
    stx::Span<VkSubpassDependency const> subpass_dependencies) noexcept {
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

inline VkPipeline create_graphics_pipeline(
    VkDevice device, VkPipelineLayout layout, VkRenderPass render_pass,
    stx::Span<VkPipelineShaderStageCreateInfo const> shader_stages_create_infos,
    VkPipelineVertexInputStateCreateInfo const& vertex_input_state,
    VkPipelineInputAssemblyStateCreateInfo const& input_assembly_state,
    VkPipelineViewportStateCreateInfo const& viewport_state,
    VkPipelineRasterizationStateCreateInfo const& rasterization_state,
    VkPipelineMultisampleStateCreateInfo const& multisample_state,
    VkPipelineDepthStencilStateCreateInfo const& depth_stencil_state,
    VkPipelineColorBlendStateCreateInfo const& color_blend_state,
    VkPipelineDynamicStateCreateInfo const& dynamic_state) noexcept {
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
inline VkFramebuffer create_frame_buffer(
    VkDevice device, VkRenderPass render_pass,
    stx::Span<VkImageView const> attachments, VkExtent2D extent) noexcept {
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

inline VkCommandPool create_command_pool(
    VkDevice device, uint32_t queue_family_index,
    bool enable_command_buffer_resetting = false) noexcept {
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

inline void allocate_command_buffer(VkDevice device, VkCommandPool command_pool,
                                    VkCommandBuffer& command_buffer  // NOLINT
                                    ) noexcept {
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

inline void allocate_command_buffers(
    VkDevice device, VkCommandPool command_pool,
    stx::Span<VkCommandBuffer> command_buffers) noexcept {
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

inline void reset_command_buffer(VkCommandBuffer command_buffer,
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

  Recorder begin_recording(VkCommandBufferUsageFlagBits usage = {},
                           VkCommandBufferInheritanceInfo const*
                               inheritance_info = nullptr) noexcept {
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
                uint64_t dst_offset) noexcept {
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
                VkExtent3D dst_extent) noexcept {
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
      stx::Span<VkClearValue const> clear_values) noexcept {
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

  Recorder end_render_pass() noexcept {
    vkCmdEndRenderPass(command_buffer_);
    return *this;
  }

  Recorder bind_pipeline(VkPipeline pipeline,
                         VkPipelineBindPoint bind_point) noexcept {
    vkCmdBindPipeline(command_buffer_, bind_point, pipeline);
    return *this;
  }

  Recorder bind_pipeline_barrier(
      VkPipelineStageFlagBits src_stages = {},
      VkPipelineStageFlagBits dst_stages = {},
      stx::Span<VkMemoryBarrier const> memory_barriers = {},
      stx::Span<VkBufferMemoryBarrier const> buffer_memory_barriers = {},
      stx::Span<VkImageMemoryBarrier const> iamge_memory_barriers =
          {}) noexcept {
    // TODO(lamarrr): don't
    // 0 or VK_DEPENDENCY_BY_REGION_BIT. VK_DEPENDENCY_BY_REGION_BIT the barrier
    // into a per-region condition. That means that the implementation is
    // allowed to already begin reading from the parts of a resource that were
    // written so far.
    VkDependencyFlags dependency = 0;

    vkCmdPipelineBarrier(
        command_buffer_, src_stages, dst_stages, dependency,
        memory_barriers.size(), memory_barriers.data(),
        buffer_memory_barriers.size(), buffer_memory_barriers.data(),
        iamge_memory_barriers.size(), iamge_memory_barriers.data());
    return *this;
  }

  template <
      typename BufferType,
      std::enable_if_t<static_cast<bool>(BufferType::usage&
                                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
                       int> = 0>
  Recorder bind_vertex_buffer(uint32_t binding, BufferType const& buffer,
                              uint64_t buffer_offset) noexcept {
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
                             VkIndexType dtype) noexcept {
    vkCmdBindIndexBuffer(command_buffer_, buffer.buffer_, buffer_offset, dtype);
    return *this;
  }

  Recorder bind_descriptor_sets(
      VkPipelineLayout pipeline_layout, VkPipelineBindPoint bind_point,
      stx::Span<VkDescriptorSet const> descriptor_sets) noexcept {
    vkCmdBindDescriptorSets(command_buffer_, bind_point, pipeline_layout, 0,
                            descriptor_sets.size(), descriptor_sets.data(), 0,
                            nullptr);  // no dynamic offsets for now
    return *this;
  }

  Recorder draw(uint32_t vertex_count, uint32_t instance_count,
                uint32_t first_vertex, uint32_t first_instance) noexcept {
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
                        uint32_t first_instance) noexcept {
    // instanceCount: Used for instanced rendering
    // firstVertex: Used as an offset into the vertex buffer, defines the lowest
    // value of gl_VertexIndex. firstInstance: Used as an offset for instanced
    // rendering, defines the lowest value of gl_InstanceIndex.
    vkCmdDrawIndexed(command_buffer_, index_count, instance_count, first_index,
                     vertex_offset, first_instance);
    return *this;
  }

  Recorder set_viewports(stx::Span<VkViewport const> viewports) noexcept {
    vkCmdSetViewport(command_buffer_, 0, viewports.size(), viewports.data());
    return *this;
  }

  Recorder set_scissors(stx::Span<VkRect2D const> scissors) noexcept {
    vkCmdSetScissor(command_buffer_, 0, scissors.size(), scissors.data());
    return *this;
  }

  Recorder set_line_width(float line_width) noexcept {
    vkCmdSetLineWidth(command_buffer_, line_width);
    return *this;
  }

  Recorder end_recording() noexcept {
    VLK_MUST_SUCCEED(vkEndCommandBuffer(command_buffer_),
                     "Unable to end command buffer recording");
    return *this;
  }
};
}  // namespace cmd

// GPU-GPU synchronization primitive, cheap
inline VkSemaphore create_semaphore(VkDevice device) noexcept {
  VkSemaphoreCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkSemaphore semaphore;
  VLK_MUST_SUCCEED(vkCreateSemaphore(device, &create_info, nullptr, &semaphore),
                   "Unable to create semaphore");

  return semaphore;
}

// GPU-CPU synchronization primitive, expensive
inline VkFence create_fence(VkDevice device,
                            VkFenceCreateFlagBits make_signaled) noexcept {
  VkFenceCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  create_info.flags = make_signaled;

  VkFence fence;

  VLK_MUST_SUCCEED(vkCreateFence(device, &create_info, nullptr, &fence),
                   "Unable to create fence");

  return fence;
}

inline void reset_fences(VkDevice device,
                         stx::Span<VkFence const> fences) noexcept {
  VLK_MUST_SUCCEED(vkResetFences(device, fences.size(), fences.data()),
                   "Unable to reset fences");
}

inline bool await_fences(VkDevice device, stx::Span<VkFence const> fences) {
  VLK_MUST_SUCCEED(
      vkWaitForFences(
          device, fences.size(), fences.data(), true,
          std::chrono::duration_cast<std::chrono::nanoseconds>(1min).count()),
      "Unable to await fences");
}

inline void submit_commands(VkQueue command_queue,
                            VkCommandBuffer command_buffer,
                            stx::Span<VkSemaphore const> await_semaphores,
                            stx::Span<VkPipelineStageFlags const> await_stages,
                            stx::Span<VkSemaphore const> notify_semaphores,
                            VkFence notify_fence) noexcept {
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

inline std::pair<uint32_t, VkResult> acquire_next_swapchain_image(
    VkDevice device, VkSwapchainKHR swapchain, VkSemaphore signal_semaphore,
    VkFence signal_fence) {
  uint32_t index = 0;

  auto result = vkAcquireNextImageKHR(
      device, swapchain,
      std::chrono::duration_cast<std::chrono::nanoseconds>(
        1min)
          .count(),
      signal_semaphore, signal_fence, &index);
  VLK_ENSURE(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
                 result == VK_ERROR_OUT_OF_DATE_KHR,
             "Unable to acquire next image");

  return std::make_pair(index, result);
}

inline VkResult present(
    VkQueue command_queue, stx::Span<VkSemaphore const> await_semaphores,
    stx::Span<VkSwapchainKHR const> swapchains,
    stx::Span<uint32_t const> swapchain_image_indexes) noexcept {
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

  auto result = vkQueuePresentKHR(command_queue, &present_info);
  VLK_ENSURE(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
                 result == VK_ERROR_OUT_OF_DATE_KHR,
             "Unable to present to swapchain");

  return result;
}

// creates buffer object but doesn't assign memory to it
inline VkBuffer create_buffer(VkDevice device, uint64_t byte_size,
                              VkBufferUsageFlagBits usage,
                              VkSharingMode sharing_mode) noexcept {
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

// creates image but doesn't assign memory to it
// different image layouts are suitable for different image operations
inline VkImage create_image(VkDevice device, VkImageType type,
                            VkExtent3D extent, VkImageUsageFlagBits usage,
                            VkSharingMode sharing_mode, VkFormat format,
                            VkImageLayout initial_layout) noexcept {
  VkImageCreateInfo image_info{};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.usage = usage;
  image_info.imageType = type;
  image_info.extent = extent;
  image_info.sharingMode = sharing_mode;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.format = format;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.initialLayout = initial_layout;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.flags = 0;

  VkImage image;
  VLK_MUST_SUCCEED(vkCreateImage(device, &image_info, nullptr, &image),
                   "Unable to create image");
  return image;
}

// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Optimal for presentation
// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Optimal as attachment for writing
// colors from the fragment shader VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: Optimal
// as source in a transfer operation, like vkCmdCopyImageToBuffer
// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Optimal as destination in a transfer
// operation, like vkCmdCopyBufferToImage
// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: Optimal for sampling from a shader

// establishes synchronization of the state of the image's memory (state
// transitions that must occur between each operation) i.e. making sure that an
// image was written to before it is read. They can also be used to transition
// the image's layouts.
constexpr VkImageMemoryBarrier make_image_memory_barrier(
    VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
    VkAccessFlags src_access_flags, VkAccessFlags dst_access_flags) noexcept {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex =
      VK_QUEUE_FAMILY_IGNORED;  // not transferring ownership of the image
  barrier.dstQueueFamilyIndex =
      VK_QUEUE_FAMILY_IGNORED;  // not transferring ownership of the image
  barrier.image = image;
  barrier.subresourceRange.aspectMask =
      VK_IMAGE_ASPECT_COLOR_BIT;  // part of the image
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  barrier.srcAccessMask = src_access_flags;
  barrier.dstAccessMask = dst_access_flags;

  return barrier;
}

// get memory requirements for a buffer based on it's type, usage mode, and
// other properties
inline VkMemoryRequirements get_memory_requirements(VkDevice device,
                                                    VkBuffer buffer) noexcept {
  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);
  return memory_requirements;
}

// get memory requirements for an image based on it's type, usage mode, and
// other properties
inline VkMemoryRequirements get_memory_requirements(VkDevice device,
                                                    VkImage image) noexcept {
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(device, image, &memory_requirements);
  return memory_requirements;
}

// returns index of the heap on the physical device, could be RAM, SWAP, or VRAM
inline stx::Option<uint32_t> find_suitable_memory_type(
    VkPhysicalDevice physical_device,
    VkMemoryRequirements const& memory_requirements,
    VkMemoryPropertyFlagBits required_properties =
        VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM) noexcept {
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
inline VkDeviceMemory allocate_memory(VkDevice device, uint32_t heap_index,
                                      uint64_t size) noexcept {
  VkMemoryAllocateInfo allocate_info{};

  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = size;
  allocate_info.memoryTypeIndex = heap_index;

  VkDeviceMemory memory;
  VLK_MUST_SUCCEED(vkAllocateMemory(device, &allocate_info, nullptr, &memory),
                   "Unable to allocate memory");

  return memory;
}

inline void bind_memory_to_buffer(VkDevice device, VkBuffer buffer,
                                  VkDeviceMemory memory,
                                  uint64_t offset) noexcept {
  VLK_MUST_SUCCEED(vkBindBufferMemory(device, buffer, memory, offset),
                   "Unable to bind memory to buffer");
}

inline void bind_memory_to_image(VkDevice device, VkImage image,
                                 VkDeviceMemory memory,
                                 uint64_t offset) noexcept {
  VLK_MUST_SUCCEED(vkBindImageMemory(device, image, memory, offset),
                   "Unable to bind memory to image");
}

struct MemoryMap {
  // offset of the memory address this map points to
  uint64_t offset;
  // contains the (offset+adress) and size
  stx::Span<uint8_t volatile> span;
};

inline MemoryMap map_memory(VkDevice device, VkDeviceMemory memory,
                            uint64_t offset, uint64_t size,
                            VkMemoryMapFlags flags = 0) noexcept {
  void* ptr;
  VLK_MUST_SUCCEED(vkMapMemory(device, memory, offset, size, flags, &ptr),
                   "Unable to map memory");
  return MemoryMap{offset,
                   stx::Span<uint8_t>{reinterpret_cast<uint8_t*>(ptr), size}};
}

// unlike OpenGL the driver may not immediately copy the data after unmap, i.e.
// due to caching. so we need to flush our writes
inline void unmap_memory(VkDevice device, VkDeviceMemory memory) noexcept {
  vkUnmapMemory(device, memory);
}

// due to caching we need to flush writes to the memory map before reading again
// has size requirements for the flush range
inline void flush_memory_map(VkDevice device, VkDeviceMemory memory,
                             uint64_t offset, uint64_t size) noexcept {
  VkMappedMemoryRange range{};
  range.memory = memory;
  range.offset = offset;
  range.size = size;
  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

  VLK_MUST_SUCCEED(vkFlushMappedMemoryRanges(device, 1, &range),
                   "Unable to flush memory map");
}

inline void refresh_memory_map(VkDevice device, VkDeviceMemory memory,
                               uint64_t offset, uint64_t size) noexcept {
  VkMappedMemoryRange range{};
  range.memory = memory;
  range.offset = offset;
  range.size = size;
  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

  VLK_MUST_SUCCEED(vkInvalidateMappedMemoryRanges(device, 1, &range),
                   "Unable to re-read memory map");
}

constexpr VkDescriptorSetLayoutBinding make_descriptor_set_layout_binding(
    uint32_t binding,
    uint32_t descriptor_count  // number of objects being described starting
                               // from {binding}
    ,
    VkDescriptorType descriptor_type, VkShaderStageFlagBits shader_stages,
    VkSampler const* sampler = nullptr) noexcept {
  VkDescriptorSetLayoutBinding dsl_binding{};
  dsl_binding.binding = binding;
  dsl_binding.descriptorType = descriptor_type;
  dsl_binding.descriptorCount = descriptor_count;
  dsl_binding.pImmutableSamplers = sampler;
  dsl_binding.stageFlags = shader_stages;

  return dsl_binding;
}

// descriptor sets define the input data for the uniforms (or samplers)
inline VkDescriptorSetLayout create_descriptor_set_layout(
    VkDevice device, stx::Span<VkDescriptorSetLayoutBinding const> bindings,
    VkDescriptorSetLayoutCreateFlagBits flags = {}) noexcept {
  VkDescriptorSetLayoutCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  create_info.bindingCount = bindings.size();
  create_info.pBindings = bindings.data();
  create_info.pNext = nullptr;
  create_info.flags = flags;

  VkDescriptorSetLayout ds_layout;
  VLK_MUST_SUCCEED(
      vkCreateDescriptorSetLayout(device, &create_info, nullptr, &ds_layout),
      "Unable to create descriptor set layout");

  return ds_layout;
}

inline VkDescriptorPool create_descriptor_pool(
    VkDevice device, uint32_t max_descriptor_sets,
    stx::Span<VkDescriptorPoolSize const> pool_sizing) noexcept {
  // create pool capable of holding different types of data with varying number
  // of descriptors
  VkDescriptorPoolCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  create_info.poolSizeCount = pool_sizing.size();
  create_info.pPoolSizes = pool_sizing.data();

  create_info.maxSets =
      max_descriptor_sets;  /// desc sets is
                            // a set with similar properties (can be by type and
                            // are not necessarily unique as the name might
                            // imply)

  VkDescriptorPool descriptor_pool;
  VLK_MUST_SUCCEED(
      vkCreateDescriptorPool(device, &create_info, nullptr, &descriptor_pool),
      "Unable to create descriptor pool");

  return descriptor_pool;
}

// each descriptor set represents a descriptor for a certain buffer type i.e.
// DESCRIPTOR_TYPE_UNIFORM_BUFFER
inline void allocate_descriptor_sets(
    VkDevice device, VkDescriptorPool descriptor_pool,
    stx::Span<VkDescriptorSetLayout const> layouts,
    stx::Span<VkDescriptorSet> descriptor_sets) noexcept {
  VLK_ENSURE(layouts.size() == descriptor_sets.size(),
             "descriptor sets and layouts sizes must match");

  VkDescriptorSetAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  info.descriptorPool = descriptor_pool;
  info.descriptorSetCount = layouts.size();
  info.pSetLayouts = layouts.data();

  VLK_MUST_SUCCEED(
      vkAllocateDescriptorSets(device, &info, descriptor_sets.data()),
      "Unable to create descriptor sets");
}

// descriptor set writer interface, can write multiple objects of the same time
// type in one pass (images, buffers, texels, etc.)
struct DescriptorSetProxy {
  VkDevice device;
  VkDescriptorSet descriptor_set;
  VkDescriptorType descriptor_type;
  uint32_t binding;

  DescriptorSetProxy bind_buffers(
      stx::Span<VkDescriptorBufferInfo const> buffers) noexcept {
    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = binding;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = descriptor_type;
    descriptor_write.descriptorCount = buffers.size();

    descriptor_write.pBufferInfo = buffers.data();

    vkUpdateDescriptorSets(device, 1, &descriptor_write, 0, nullptr);

    return *this;
  }

  DescriptorSetProxy bind_images(
      stx::Span<VkDescriptorImageInfo const> images) noexcept {
    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = binding;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = descriptor_type;
    descriptor_write.descriptorCount = images.size();

    descriptor_write.pImageInfo = images.data();

    vkUpdateDescriptorSets(device, 1, &descriptor_write, 0, nullptr);

    return *this;
  }

  // copy and write

  DescriptorSetProxy copy_image();  // descriptor_write.pImageInfo

  DescriptorSetProxy write_texel();  // descriptor_write.pTexelView
};

constexpr std::string_view format(VkFormat format) {
  switch (format) {
    VLK_ERRNUM_CASE(VK_FORMAT_UNDEFINED)
    VLK_ERRNUM_CASE(VK_FORMAT_R4G4_UNORM_PACK8)
    VLK_ERRNUM_CASE(VK_FORMAT_R4G4B4A4_UNORM_PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_B4G4R4A4_UNORM_PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_R5G6B5_UNORM_PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_B5G6R5_UNORM_PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_R5G5B5A1_UNORM_PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_B5G5R5A1_UNORM_PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_A1R5G5B5_UNORM_PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_R8_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R8_SNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R8_USCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R8_SSCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R8_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R8_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R8_SRGB)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8_SNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8_USCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8_SSCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8_SRGB)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8_SNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8_USCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8_SSCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8_SRGB)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8_SNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8_USCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8_SSCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8_SRGB)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_SNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_USCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_SSCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_SRGB)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_SNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_USCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_SSCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_SRGB)
    VLK_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_UNORM_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_SNORM_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_USCALED_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_SSCALED_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_UINT_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_SINT_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_SRGB_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_UNORM_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_SNORM_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_USCALED_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_SSCALED_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_UINT_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_SINT_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_UNORM_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_SNORM_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_USCALED_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_SSCALED_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_UINT_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_SINT_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_R16_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R16_SNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R16_USCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R16_SSCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R16_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16_SNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16_USCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16_SSCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16_SNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16_USCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16_SSCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_SNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_USCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_SSCALED)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32G32_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32G32_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32G32_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32G32B32_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32G32B32_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32G32B32_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32G32B32A32_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32G32B32A32_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R32G32B32A32_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64G64_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64G64_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64G64_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64G64B64_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64G64B64_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64G64B64_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64G64B64A64_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64G64B64A64_SINT)
    VLK_ERRNUM_CASE(VK_FORMAT_R64G64B64A64_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_B10G11R11_UFLOAT_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_D16_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_X8_D24_UNORM_PACK32)
    VLK_ERRNUM_CASE(VK_FORMAT_D32_SFLOAT)
    VLK_ERRNUM_CASE(VK_FORMAT_S8_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_D16_UNORM_S8_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_D24_UNORM_S8_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_D32_SFLOAT_S8_UINT)
    VLK_ERRNUM_CASE(VK_FORMAT_BC1_RGB_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC1_RGB_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC1_RGBA_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC1_RGBA_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC2_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC2_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC3_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC3_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC4_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC4_SNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC5_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC5_SNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC6H_UFLOAT_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC6H_SFLOAT_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC7_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_BC7_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_EAC_R11_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_EAC_R11_SNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_EAC_R11G11_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_EAC_R11G11_SNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_4x4_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_4x4_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_5x4_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_5x4_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_5x5_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_5x5_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_6x5_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_6x5_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_6x6_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_6x6_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_8x5_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_8x5_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_8x6_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_8x6_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_8x8_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_8x8_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_10x5_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_10x5_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_10x6_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_10x6_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_10x8_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_10x8_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_10x10_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_10x10_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_12x10_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_12x10_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_12x12_UNORM_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_ASTC_12x12_SRGB_BLOCK)
    VLK_ERRNUM_CASE(VK_FORMAT_G8B8G8R8_422_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_B8G8R8G8_422_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_R10X6_UNORM_PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_R10X6G10X6_UNORM_2PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_R12X4_UNORM_PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_R12X4G12X4_UNORM_2PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16)
    VLK_ERRNUM_CASE(VK_FORMAT_G16B16G16R16_422_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_B16G16R16G16_422_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM)
    VLK_ERRNUM_CASE(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG)
    VLK_ERRNUM_CASE(VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG)
    VLK_ERRNUM_CASE(VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG)
    VLK_ERRNUM_CASE(VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG)
    VLK_ERRNUM_CASE(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG)
    VLK_ERRNUM_CASE(VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG)
    VLK_ERRNUM_CASE(VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG)
    VLK_ERRNUM_CASE(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG)
    default:
      return "Unidentified Format Enum";
  }
}

constexpr std::string_view format(VkResult error) noexcept {
  switch (error) {
    VLK_ERRNUM_CASE(VK_SUCCESS)
    VLK_ERRNUM_CASE(VK_NOT_READY)
    VLK_ERRNUM_CASE(VK_TIMEOUT)
    VLK_ERRNUM_CASE(VK_EVENT_SET)
    VLK_ERRNUM_CASE(VK_EVENT_RESET)
    VLK_ERRNUM_CASE(VK_INCOMPLETE)
    VLK_ERRNUM_CASE(VK_ERROR_OUT_OF_HOST_MEMORY)
    VLK_ERRNUM_CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY)
    VLK_ERRNUM_CASE(VK_ERROR_INITIALIZATION_FAILED)
    VLK_ERRNUM_CASE(VK_ERROR_DEVICE_LOST)
    VLK_ERRNUM_CASE(VK_ERROR_MEMORY_MAP_FAILED)
    VLK_ERRNUM_CASE(VK_ERROR_LAYER_NOT_PRESENT)
    VLK_ERRNUM_CASE(VK_ERROR_EXTENSION_NOT_PRESENT)
    VLK_ERRNUM_CASE(VK_ERROR_FEATURE_NOT_PRESENT)
    VLK_ERRNUM_CASE(VK_ERROR_INCOMPATIBLE_DRIVER)
    VLK_ERRNUM_CASE(VK_ERROR_TOO_MANY_OBJECTS)
    VLK_ERRNUM_CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
    VLK_ERRNUM_CASE(VK_ERROR_FRAGMENTED_POOL)
    // VLK_ERRNUM_CASE( VK_ERROR_UNKNOWN)

    // Provided by VK_VERSION_1_1
    VLK_ERRNUM_CASE(VK_ERROR_OUT_OF_POOL_MEMORY)
    VLK_ERRNUM_CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE)

    // Provided by VK_VERSION_1_2
    VLK_ERRNUM_CASE(VK_ERROR_FRAGMENTATION_EXT)
    // VLK_ERRNUM_CASE( VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)

    // Provided by VK_KHR_surface
    VLK_ERRNUM_CASE(VK_ERROR_SURFACE_LOST_KHR)
    VLK_ERRNUM_CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)

    // Provided by VK_KHR_swapchain
    VLK_ERRNUM_CASE(VK_SUBOPTIMAL_KHR)
    VLK_ERRNUM_CASE(VK_ERROR_OUT_OF_DATE_KHR)

    // Provided by VK_KHR_display_swapchain
    VLK_ERRNUM_CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)

    // Provided by VK_EXT_debug_report
    VLK_ERRNUM_CASE(VK_ERROR_VALIDATION_FAILED_EXT)

    // Provided by VK_NV_glsl_shader
    VLK_ERRNUM_CASE(VK_ERROR_INVALID_SHADER_NV)

    // Provided by VK_EXT_image_drm_format_modifier
    // VLK_ERRNUM_CASE( VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)

    // Provided by VK_EXT_global_priority
    VLK_ERRNUM_CASE(VK_ERROR_NOT_PERMITTED_EXT)

    // Provided by VK_EXT_full_screen_exclusive
    // VLK_ERRNUM_CASE( VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)

    // Provided by VK_KHR_deferred_host_operations
    // VLK_ERRNUM_CASE( VK_THREAD_IDLE_KHR)
    // VLK_ERRNUM_CASE( VK_THREAD_DONE_KHR)
    // VLK_ERRNUM_CASE( VK_OPERATION_DEFERRED_KHR)
    // VLK_ERRNUM_CASE( VK_OPERATION_NOT_DEFERRED_KHR)

    // Provided by VK_EXT_pipeline_creation_cache_control
    // VLK_ERRNUM_CASE( VK_PIPELINE_COMPILE_REQUIRED_EXT)

    VLK_ERRNUM_CASE(VK_RESULT_MAX_ENUM)

    default:
      return "Unidentified Error Enum";
  }
}

constexpr std::string_view format(VkPhysicalDeviceType type) {
  switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      return "CPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      return "dGPU";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      return "iGPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      return "vGPU";
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
      return "other";
    default:
      return "unidentified device type";
  }
}

constexpr std::string_view format(VkColorSpaceKHR color_space) {
  switch (color_space) {
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_DCI_P3_LINEAR_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_BT709_LINEAR_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_BT709_NONLINEAR_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_BT2020_LINEAR_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_HDR10_ST2084_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_DOLBYVISION_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_HDR10_HLG_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_PASS_THROUGH_EXT)
    VLK_ERRNUM_CASE(VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT)
    default:
      return "unidentified color space";
  }
}

}  // namespace vk
}  // namespace vlk

// TODO(lamarrr): must return a span of any type. would be converted to bytes
// and sent
inline stx::SpanReport operator>>(stx::ReportQuery,
                                  VkResult const& result) noexcept {
  return stx::SpanReport(vlk::vk::format(result));
}

inline stx::SpanReport operator>>(stx::ReportQuery,
                                  VkFormat const& format) noexcept {
  return stx::SpanReport(vlk::vk::format(format));
}
