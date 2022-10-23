#pragma once

#include <algorithm>
#include <chrono>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "asura/primitives.h"
#include "asura/utils.h"
#include "stx/backtrace.h"
#include "stx/limits.h"
#include "stx/option.h"
#include "stx/result.h"
#include "stx/span.h"
#include "stx/vec.h"
#include "vulkan/vulkan.h"

namespace asr {

using namespace std::chrono_literals;

namespace vk {

template <typename T>
inline auto join_copy(stx::Span<T> a, stx::Span<T> b) {
  stx::Vec<std::remove_const_t<T>> x{stx::os_allocator};
  x.reserve(a.size() + b.size()).unwrap();

  for (auto const& el : a) x.push_inplace(el).unwrap();
  for (auto const& el : b) x.push_inplace(el).unwrap();

  return x;
}

inline void ensure_validation_layers_supported(
    stx::Span<char const* const> layers) {
  uint32_t available_validation_layers_count;
  vkEnumerateInstanceLayerProperties(&available_validation_layers_count,
                                     nullptr);

  stx::Vec<VkLayerProperties> available_validation_layers(stx::os_allocator);

  available_validation_layers.resize(available_validation_layers_count)
      .unwrap();

  ASR_LOG("Available Vulkan Validation Layers:");

  vkEnumerateInstanceLayerProperties(&available_validation_layers_count,
                                     available_validation_layers.data());

  for (VkLayerProperties const& layer : available_validation_layers) {
    ASR_LOG("\t{} (spec version: {})", layer.layerName, layer.specVersion);
  }

  bool all_layers_available = true;

  for (char const* req_layer : layers) {
    if (available_validation_layers.span()
            .which([&req_layer](VkLayerProperties const& available_layer) {
              return std::string_view(req_layer) ==
                     std::string_view(available_layer.layerName);
            })
            .is_empty()) {
      all_layers_available = false;
      ASR_WARN("Required validation layer `{}` is not available",
               std::string_view(req_layer));
    }
  }

  ASR_ENSURE(all_layers_available,
             "One or more required validation layers are not available");
}

// NICE-TO-HAVE(lamarrr): versioning of extensions, know which one wasn't
// available and adjust features to that
inline void ensure_extensions_supported(stx::Span<char const* const> names) {
  uint32_t available_vk_extensions_count = 0;
  vkEnumerateInstanceExtensionProperties(
      nullptr, &available_vk_extensions_count, nullptr);

  stx::Vec<VkExtensionProperties> available_vk_extensions(stx::os_allocator);

  available_vk_extensions.resize(available_vk_extensions_count).unwrap();

  vkEnumerateInstanceExtensionProperties(
      nullptr, &available_vk_extensions_count, available_vk_extensions.data());

  ASR_LOG("Available Vulkan Extensions:");
  for (auto extension : available_vk_extensions) {
    ASR_LOG("\t{},  spec version: {}", extension.extensionName,
            extension.specVersion);
  }

  bool all_available = true;

  for (auto const name : names) {
    if (available_vk_extensions.span()
            .which([&name](VkExtensionProperties const& props) {
              return std::string_view(name) ==
                     std::string_view(props.extensionName);
            })
            .is_empty()) {
      all_available = false;
      ASR_WARN("Required extension `{}` is not available",
               std::string_view(name));
    }
  }

  ASR_ENSURE(all_available,
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
    ASR_LOG_IF(is_general, "[Validation Layer Message] {}",
               callback_data->pMessage);
    ASR_WARN_IF(!is_general, "[Validation Layer Message] {}",
                callback_data->pMessage);
  } else {
    ASR_LOG_IF(is_general, "[Validation Layer Message, Hints=\"{}\"] {}", hint,
               callback_data->pMessage);
    ASR_WARN_IF(!is_general, "[Validation Layer Message, Hints=\"{}\"] {}",
                hint, callback_data->pMessage);
  }

  if (!is_general) {
    ASR_LOG("Call Stack:");
    stx::backtrace::trace(
        stx::fn::make_static([](stx::backtrace::Frame frame, int) {
          ASR_LOG("\t=> {}", frame.symbol.copy().match(
                                 [](auto sym) { return sym.raw(); },
                                 []() { return std::string_view("unknown"); }));
          return false;
        }),
        2);
  }

  return VK_FALSE;
}

inline VkDebugUtilsMessengerCreateInfoEXT make_debug_messenger_create_info() {
  VkDebugUtilsMessengerCreateInfoEXT create_info{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = default_debug_callback,
      .pUserData = nullptr};

  return create_info;
}

inline VkDebugUtilsMessengerEXT create_install_debug_messenger(
    VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT create_info =
                             make_debug_messenger_create_info()) {
  VkDebugUtilsMessengerEXT debug_messenger;

  auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

  ASR_ENSURE(
      func != nullptr,
      "Unable to get process address for vkCreateDebugUtilsMessengerEXT");

  ASR_MUST_SUCCEED(func(instance, &create_info, nullptr, &debug_messenger),
                   "Unable to setup debug messenger");

  return debug_messenger;
}

inline void destroy_debug_messenger(VkInstance instance,
                                    VkDebugUtilsMessengerEXT debug_messenger) {
  auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

  ASR_ENSURE(func != nullptr, "Unable to destroy debug messenger");

  return func(instance, debug_messenger, nullptr);
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
    uint32_t engine_version = VK_MAKE_VERSION(1, 0, 0)) {
  // helps bnt not necessary
  VkApplicationInfo app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .pApplicationName = application_name,
                             .applicationVersion = application_version,
                             .pEngineName = engine_name,
                             .engineVersion = engine_version,
                             .apiVersion = VK_API_VERSION_1_1,
                             .pNext = nullptr};

  VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
      .pNext = nullptr};

  static constexpr char const* DEBUG_EXTENSIONS[] = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

  // TODO(lamarrr): check that the requested extensions are available
  // debug message callback extension
  auto extensions =
      join_copy(required_extensions,
                required_validation_layers.is_empty()
                    ? stx::Span<char const* const>{}
                    : stx::Span<char const* const>(DEBUG_EXTENSIONS));

  create_info.enabledExtensionCount = extensions.size();
  create_info.ppEnabledExtensionNames = extensions.data();

  ensure_extensions_supported(extensions);

  if (!required_validation_layers.is_empty()) {
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
  ASR_MUST_SUCCEED(vkCreateInstance(&create_info, nullptr, &vulkan_instance),
                   "Unable to create vulkan instance");

  VkDebugUtilsMessengerEXT messenger = nullptr;

  if (!required_validation_layers.is_empty()) {
    messenger = create_install_debug_messenger(vulkan_instance,
                                               debug_messenger_create_info);
  }

  return std::make_pair(vulkan_instance, messenger);
}

//  to do anything on the GPU (render, draw, compute, allocate memory, create
//  texture, etc.) we use command queues
inline stx::Vec<VkQueueFamilyProperties> get_queue_families(
    VkPhysicalDevice device) {
  uint32_t queue_families_count;

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count,
                                           nullptr);

  stx::Vec<VkQueueFamilyProperties> queue_families_properties(
      stx::os_allocator);

  queue_families_properties.resize(queue_families_count).unwrap();

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count,
                                           queue_families_properties.data());

  return queue_families_properties;
}

inline stx::Vec<bool> get_command_queue_support(
    stx::Span<VkQueueFamilyProperties const> queue_families,
    VkQueueFlagBits required_command_queue) {
  stx::Vec<bool> supports{stx::os_allocator};

  for (auto const& fam_props : queue_families) {
    supports.push(fam_props.queueFlags & required_command_queue).unwrap();
  }

  return supports;
}

// find the device's queue family capable of supporting surface presentation
inline stx::Vec<bool> get_surface_presentation_command_queue_support(
    VkPhysicalDevice phy_device,
    stx::Span<VkQueueFamilyProperties const> queue_families,
    VkSurfaceKHR surface) {
  stx::Vec<bool> supports{stx::os_allocator};

  for (size_t i = 0; i < queue_families.size(); i++) {
    VkBool32 surface_presentation_supported;
    ASR_MUST_SUCCEED(
        vkGetPhysicalDeviceSurfaceSupportKHR(phy_device, i, surface,
                                             &surface_presentation_supported),
        "Unable to query physical device' surface support");
    supports.push_inplace(surface_presentation_supported == VK_TRUE).unwrap();
  }

  return supports;
}

inline VkDevice create_logical_device(
    VkPhysicalDevice phy_device,
    stx::Span<char const* const> required_extensions,
    stx::Span<char const* const> required_validation_layers,
    stx::Span<VkDeviceQueueCreateInfo const> command_queue_create_infos,
    VkPhysicalDeviceFeatures const& required_features = {}) {
  VkDeviceCreateInfo device_create_info{
      .pQueueCreateInfos = command_queue_create_infos.data(),
      .queueCreateInfoCount =
          static_cast<uint32_t>(command_queue_create_infos.size()),
      .pEnabledFeatures = &required_features};

  uint32_t available_extensions_count;
  ASR_MUST_SUCCEED(
      vkEnumerateDeviceExtensionProperties(
          phy_device, nullptr, &available_extensions_count, nullptr),
      "Unable to get physical device extensions");

  // device specific extensions
  stx::Vec<VkExtensionProperties> available_device_extensions{
      stx::os_allocator};

  available_device_extensions.resize(available_extensions_count).unwrap();

  ASR_MUST_SUCCEED(vkEnumerateDeviceExtensionProperties(
                       phy_device, nullptr, &available_extensions_count,
                       available_device_extensions.data()),
                   "Unable to get physical device extensions");

  ASR_LOG("Required Device Extensions: ");
  required_extensions.for_each([](char const* ext) { ASR_LOG("\t{}", ext); });

  ASR_LOG("Available Device Extensions: ");
  available_device_extensions.span().for_each([](VkExtensionProperties ext) {
    ASR_LOG("\t{} (spec version: {})", ext.extensionName, ext.specVersion);
  });

  ASR_ENSURE(required_extensions.is_all([&](char const* ext) {
    return !available_device_extensions.span()
                .which([=](VkExtensionProperties a_ext) {
                  return std::string_view(ext) ==
                         std::string_view(a_ext.extensionName);
                })
                .is_empty();
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

  ASR_ENSURE(vkCreateDevice(phy_device, &device_create_info, nullptr,
                            &logical_device) == VK_SUCCESS,
             "Unable to Create Physical Device");

  return logical_device;
}

inline VkQueue get_command_queue(VkDevice device, uint32_t queue_family_index,
                                 uint32_t command_queue_index_in_family) {
  VkQueue command_queue;
  vkGetDeviceQueue(device, queue_family_index, command_queue_index_in_family,
                   &command_queue);
  ASR_ENSURE(command_queue != nullptr,
             "Requested command queue not created on target device");
  return command_queue;
}

struct SwapChainProperties {
  VkSurfaceCapabilitiesKHR capabilities;
  stx::Vec<VkSurfaceFormatKHR> supported_formats{stx::os_allocator};
  stx::Vec<VkPresentModeKHR> presentation_modes{stx::os_allocator};
};

inline SwapChainProperties get_swapchain_properties(VkPhysicalDevice phy_device,
                                                    VkSurfaceKHR surface) {
  SwapChainProperties details{};

  ASR_MUST_SUCCEED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                       phy_device, surface, &details.capabilities),
                   "Unable to get physical device' surface capabilities");

  uint32_t supported_surface_formats_count = 0;

  ASR_MUST_SUCCEED(
      vkGetPhysicalDeviceSurfaceFormatsKHR(
          phy_device, surface, &supported_surface_formats_count, nullptr),
      "Unable to get physical device' surface format");

  details.supported_formats.resize(supported_surface_formats_count).unwrap();

  ASR_MUST_SUCCEED(vkGetPhysicalDeviceSurfaceFormatsKHR(
                       phy_device, surface, &supported_surface_formats_count,
                       details.supported_formats.data()),
                   "Unable to get physical device' surface format");

  uint32_t surface_presentation_modes_count;
  ASR_MUST_SUCCEED(
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          phy_device, surface, &surface_presentation_modes_count, nullptr),
      "Unable to get physical device' surface presentation mode");

  details.presentation_modes.resize(surface_presentation_modes_count).unwrap();
  ASR_MUST_SUCCEED(vkGetPhysicalDeviceSurfacePresentModesKHR(
                       phy_device, surface, &surface_presentation_modes_count,
                       details.presentation_modes.data()),
                   "Unable to get physical device' surface presentation mode");

  return details;
}

inline bool is_swapchain_adequate(SwapChainProperties const& properties) {
  // we use any available for selecting devices
  ASR_ENSURE(!properties.supported_formats.is_empty(),
             "Physical Device does not support any window surface "
             "format");

  ASR_ENSURE(!properties.presentation_modes.is_empty(),
             "Physical Device does not support any window surface "
             "presentation mode");

  return true;
}

inline VkExtent2D select_swapchain_extent(
    VkSurfaceCapabilitiesKHR const& capabilities, VkExtent2D desired_extent) {
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
    VkBool32 clipped = VK_TRUE) {
  uint32_t desired_num_buffers =
      std::max(properties.capabilities.minImageCount + 1,
               properties.capabilities.maxImageCount);

  VkExtent2D selected_extent =
      select_swapchain_extent(properties.capabilities, extent);

  VkSwapchainCreateInfoKHR create_info{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .imageExtent = selected_extent,
      .surface = surface,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .presentMode = present_mode,

      // number of images to use for buffering on the swapchain
      .minImageCount = select_swapchain_image_count(properties.capabilities,
                                                    desired_num_buffers),
      .imageArrayLayers = 1,  // 2 for stereoscopic rendering
      .imageUsage = image_usages,
      .preTransform = properties.capabilities.currentTransform,
      .compositeAlpha =
          alpha_channel_blending,  // how the alpha channel should be
                                   // used for blending with other
                                   // windows in the window system
      // clipped specifies whether the Vulkan implementation is allowed to
      // discard rendering operations that affect regions of the surface that
      // are not visible. If set to VK_TRUE, the presentable images associated
      // with the swapchain may not own all of their pixels. Pixels in the
      // presentable images that correspond to regions of the target surface
      // obscured by another window on the desktop, or subject to some other
      // clipping mechanism will have undefined content when read back. Fragment
      // shaders may not execute for these pixels, and thus any side effects
      // they would have had will not occur. Setting VK_TRUE does not guarantee
      // any clipping will occur, but allows more efficient presentation methods
      // to be used on some platforms. If set to VK_FALSE, presentable images
      // associated with the swapchain will own all of the pixels they contain.
      .clipped = clipped,
      .oldSwapchain = VK_NULL_HANDLE,
      // under normal circumstances command queues on the same queue family can
      // access data without data race issues
      //
      // VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a
      // time and ownership must be explicitly transferred before using it in
      // another queue family. This option offers the best performance.
      // VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue
      // families without explicit ownership transfers.
      .imageSharingMode = accessing_queue_families_sharing_mode,
      .pQueueFamilyIndices = accessing_queue_families_indexes.data(),
      .queueFamilyIndexCount =
          static_cast<uint32_t>(accessing_queue_families_indexes.size())};

  VkSwapchainKHR swapchain;
  ASR_MUST_SUCCEED(
      vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain),
      "Unable to create swapchain");

  return std::make_pair(swapchain, selected_extent);
}

inline stx::Vec<VkImage> get_swapchain_images(VkDevice device,
                                              VkSwapchainKHR swapchain) {
  uint32_t image_count;

  ASR_MUST_SUCCEED(
      vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr),
      "Unable to get swapchain images count");

  stx::Vec<VkImage> swapchain_images{stx::os_allocator};
  swapchain_images.resize(image_count).unwrap();

  ASR_MUST_SUCCEED(vkGetSwapchainImagesKHR(device, swapchain, &image_count,
                                           swapchain_images.data()),
                   "Unable to get swapchain images");

  return swapchain_images;
}

// the number of command queues to create is encapsulated in the
// `queue_priorities` size
// this will create `queue_priorities.size()` number of command queues from
// family `queue_family_index`
constexpr VkDeviceQueueCreateInfo make_command_queue_create_info(
    uint32_t queue_family_index, stx::Span<float const> queues_priorities) {
  VkDeviceQueueCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queue_family_index,
      .pQueuePriorities = queues_priorities.data(),
      .queueCount = static_cast<uint32_t>(
          queues_priorities
              .size())  // the number of queues we want, since multiple
                        // queues can belong to a single family
  };
  return create_info;
}

constexpr VkComponentMapping make_default_component_mapping() {
  // how to map the image color components
  VkComponentMapping mapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,  // leave as-is
                             .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                             .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                             .a = VK_COMPONENT_SWIZZLE_IDENTITY};

  return mapping;
}

inline VkImageView create_image_view(VkDevice device, VkImage image,
                                     VkFormat format, VkImageViewType view_type,
                                     VkImageAspectFlagBits aspect_mask,
                                     VkComponentMapping component_mapping) {
  VkImageViewCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      // VK_IMAGE_VIEW_TYPE_2D: 2D texture
      // VK_IMAGE_VIEW_TYPE_3D: 3D texture
      // VK_IMAGE_VIEW_TYPE_CUBE: cube map
      .viewType = view_type,  // treat the image as a 2d texture
      .format = format,
      .components = component_mapping,
      // defines what part of the image this image view represents and what this
      // image view is used for
      .subresourceRange.aspectMask = aspect_mask,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.layerCount = 1,
      .subresourceRange.levelCount = 1};

  VkImageView image_view;
  ASR_MUST_SUCCEED(
      vkCreateImageView(device, &create_info, nullptr, &image_view),
      "Unable to create image view");
  return image_view;
}

inline VkSampler create_sampler(VkDevice device,
                                stx::Option<float> max_anisotropy) {
  VkSamplerCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      // for treating the case where there are more fragments than texels
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      // VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond
      // the image dimensions. VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like
      // repeat, but inverts the coordinates to mirror the image when going
      // beyond the dimensions. VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the
      // color of the edge closest to the coordinate beyond the image
      // dimensions. VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: Like clamp to
      // edge, but instead uses the edge opposite to the closest edge.
      // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Return a solid color when
      // sampling beyond the dimensions of the image.
      //
      // u, v, w coordinate overflow style of the textures
      // this shouldn't affect the texture if we are not sampling outside of the
      // image
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      // for treating the case where there are more texels than fragments
      .anisotropyEnable = max_anisotropy.is_some(),
      .maxAnisotropy =
          max_anisotropy.is_some() ? max_anisotropy.copy().unwrap() : 0.0f,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates =
          VK_FALSE,  // coordinates matching the sampled image will be
                     // normalized to the (0.0 to 1.0 range) otherwise in the
                     // (0, image width or height range)
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      // mip-mapping
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .mipLodBias = 0.0f,
      .minLod = 0.0f,
      .maxLod = 0.0f};

  VkSampler sampler;

  ASR_MUST_SUCCEED(vkCreateSampler(device, &create_info, nullptr, &sampler),
                   "Unable to create sampler");

  return sampler;
}

inline VkShaderModule create_shader_module(
    VkDevice device, stx::Span<uint32_t const> spirv_byte_data) {
  VkShaderModuleCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = spirv_byte_data.size_bytes(),
      .pCode = spirv_byte_data.data()};

  VkShaderModule shader_module;
  ASR_MUST_SUCCEED(
      vkCreateShaderModule(device, &create_info, nullptr, &shader_module),
      "Unable to create shader module");
  return shader_module;
}

constexpr VkPipelineShaderStageCreateInfo
make_pipeline_shader_stage_create_info(
    VkShaderModule module, char const* program_entry_point,
    VkShaderStageFlagBits pipeline_stage_flag,
    VkSpecializationInfo const* program_constants) {
  VkPipelineShaderStageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .module = module,
      .pName = program_entry_point,
      .stage = pipeline_stage_flag,
      .pNext = nullptr,
      .pSpecializationInfo =
          program_constants};  // provide constants used within the shader

  return create_info;
}

constexpr VkPipelineShaderStageCreateInfo
make_pipeline_shader_stage_create_info(
    VkShaderModule module, char const* program_entry_point,
    VkShaderStageFlagBits pipeline_stage_flag) {
  VkPipelineShaderStageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .module = module,
      .pName = program_entry_point,
      .stage = pipeline_stage_flag,
      .pNext = nullptr,
      .pSpecializationInfo =
          nullptr  // provide constants used within the shader
  };

  return create_info;
}

constexpr VkPipelineVertexInputStateCreateInfo
make_pipeline_vertex_input_state_create_info(
    stx::Span<VkVertexInputBindingDescription const>
        vertex_binding_descriptions,
    stx::Span<VkVertexInputAttributeDescription const>
        vertex_attribute_desciptions) {
  // Bindings: spacing between data and whether the data is per-vertex or
  // per-instance
  // Attribute descriptions: type of the attributes passed to the vertex shader,
  // which binding to load them from and at which offset
  VkPipelineVertexInputStateCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount =
          static_cast<uint32_t>(vertex_binding_descriptions.size()),
      .pVertexBindingDescriptions = vertex_binding_descriptions.data(),
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(vertex_attribute_desciptions.size()),
      .pVertexAttributeDescriptions = vertex_attribute_desciptions.data()};

  return create_info;
}

constexpr VkPipelineInputAssemblyStateCreateInfo
make_pipeline_input_assembly_state_create_info() {
  // Bindings: spacing between data and whether the data is per-vertex or
  // per-instance
  // Attribute descriptions: type of the attributes passed to the vertex shader,
  // which binding to load them from and at which offset
  VkPipelineInputAssemblyStateCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology =
          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,  // rendering in triangle mode
      .primitiveRestartEnable = VK_FALSE};

  return create_info;
}

/*
constexpr VkViewport make_viewport(float x, float y, float w, float h,
                                   float min_depth = 0.0f,
                                   float max_depth = 1.0f) {
  VkViewport viewport{
      .x = x,
      .y = y,
      .width = w,             // width of the framebuffer (swapchain image)
      .height = h,            // height of the framebuffer (swapchain image)
      .minDepth = min_depth,  // min depth value to use for the frame buffer
      .maxDepth = max_depth   // max depth value to use for the frame buffer
  };
  return viewport;
}
*/

constexpr VkPipelineViewportStateCreateInfo
make_pipeline_viewport_state_create_info(stx::Span<VkViewport const> viewports,
                                         stx::Span<VkRect2D const> scissors) {
  // to use multiple viewports, ensure the GPU feature is enabled during logical
  // device creation
  VkPipelineViewportStateCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = static_cast<uint32_t>(viewports.size()),
      .pViewports = viewports.data(),
      .scissorCount = static_cast<uint32_t>(
          scissors.size()),  // scissors cut out the part to be rendered
      .pScissors = scissors.data()};

  return create_info;
}

constexpr VkPipelineRasterizationStateCreateInfo
make_pipeline_rasterization_create_info(VkFrontFace front_face,
                                        float line_width = 1.0f) {
  VkPipelineRasterizationStateCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable =
          VK_FALSE,  // ragments that are beyond the near and far planes are
                     // clamped to them as opposed to discarding them. This is
                     // useful in some special cases like shadow maps. Using
                     // this requires enabling a GPU feature.
      .rasterizerDiscardEnable =
          VK_FALSE,  // if true, geometry never passes through the rasterization
                     // stage thus disabling output to the framebuffer
      // VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
      // VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
      // VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
      .polygonMode = VK_POLYGON_MODE_FILL,  // using any other one requires
                                            // enabling a GPU feature
      .lineWidth =
          line_width,  // any thicker than 1.0f requires enabling a GPU feature
      .cullMode = VK_CULL_MODE_BACK_BIT,  // discard the back part of the
                                          // image that isn't facing us
      .frontFace = front_face,
      .depthBiasEnable = VK_FALSE,
      .depthBiasConstantFactor = 0.0f,  // mostly used for shadow mapping
      .depthBiasClamp = 0.0f,
      .depthBiasSlopeFactor = 0.0f};

  return create_info;
}

constexpr VkPipelineMultisampleStateCreateInfo
make_pipeline_multisample_state_create_info() {
  VkPipelineMultisampleStateCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable = VK_FALSE,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .minSampleShading = 1.0f,
      .pSampleMask = nullptr,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE};

  return create_info;
}

constexpr VkPipelineDepthStencilStateCreateInfo
make_pipeline_depth_stencil_state_create_info() {
  VkPipelineDepthStencilStateCreateInfo create_info{};
  ASR_ENSURE(false, "Unimplemented");
  return create_info;
}

// per framebuffer
constexpr VkPipelineColorBlendAttachmentState
make_pipeline_color_blend_attachment_state() {
  // simply overwrites the pixels in the destination
  VkPipelineColorBlendAttachmentState state{
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD};
  return state;
}

// global pipeline state
constexpr VkPipelineColorBlendStateCreateInfo
make_pipeline_color_blend_state_create_info(
    stx::Span<VkPipelineColorBlendAttachmentState const> color_frame_buffers) {
  VkPipelineColorBlendStateCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = static_cast<uint32_t>(
          color_frame_buffers.size()),  // number of framebuffers
      .pAttachments = color_frame_buffers.data(),
      .blendConstants[0] = 0.0f,
      .blendConstants[1] = 0.0f,
      .blendConstants[2] = 0.0f,
      .blendConstants[3] = 0.0f};

  return create_info;
}

constexpr VkPipelineDynamicStateCreateInfo make_pipeline_dynamic_state(
    stx::Span<VkDynamicState const> dynamic_states) {
  // This will cause the configuration of these values to be ignored and you
  // will be required to specify the data at drawing time. This struct can be
  // substituted by a nullptr later on if you don't have any dynamic state.

  VkPipelineDynamicStateCreateInfo pipeline_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data()};

  return pipeline_state;
}

inline VkPipelineLayout create_pipeline_layout(
    VkDevice device,
    stx::Span<VkDescriptorSetLayout const> descriptor_sets_layout,
    stx::Span<VkPushConstantRange const> constant_ranges) {
  VkPipelineLayoutCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = static_cast<uint32_t>(descriptor_sets_layout.size()),
      .pSetLayouts = descriptor_sets_layout.data(),
      .pushConstantRangeCount = static_cast<uint32_t>(constant_ranges.size()),
      .pPushConstantRanges = constant_ranges.data()};

  VkPipelineLayout layout;
  ASR_MUST_SUCCEED(
      vkCreatePipelineLayout(device, &create_info, nullptr, &layout),
      "Unable to create pipeline layout");

  return layout;
}

constexpr VkAttachmentDescription make_attachment_description(VkFormat format) {
  // the format of the color attachment should match the format of the swap
  // chain images,
  VkAttachmentDescription attachment_description{
      .format = format,
      .samples = VK_SAMPLE_COUNT_1_BIT,  // no multi-sampling
      // The loadOp and storeOp determine what to do with the data in the
      // attachment before rendering and after rendering
      // VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the
      // attachment
      // VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at
      // the start
      // VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined;
      // we don't care about them
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color
      // attachment VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in
      // the swap chain VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used
      // as destination for a memory copy operation descibes layout of the
      // images
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

  return attachment_description;
}

// subpasses are for post-processing. each subpass depends on the results of the
// previous (sub)passes, used instead of transferring data
constexpr VkSubpassDescription make_subpass_description(
    stx::Span<VkAttachmentReference const> color_attachments) {
  VkSubpassDescription subpass{
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = static_cast<uint32_t>(color_attachments.size()),
      .pColorAttachments =
          color_attachments.data()  // layout(location = 0) out vec4 outColor
  };

  // pInputAttachments: Attachments that are read from a shader
  // pResolveAttachments: Attachments used for multisampling color attachments
  // pDepthStencilAttachment: Attachment for depth and stencil data
  // pPreserveAttachments: Attachments that are not used by this subpass, but
  // for which the data must be preserved
  return subpass;
}

// ????
constexpr VkSubpassDependency make_subpass_dependency() {
  VkSubpassDependency dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};

  return dependency;
}

// specify how many color and depth buffers there will be, how many samples to
// use for each of them and how their contents should be handled throughout the
// rendering operations (and the subpasses description)
inline VkRenderPass create_render_pass(
    VkDevice device,
    stx::Span<VkAttachmentDescription const> attachment_descriptions,
    stx::Span<VkSubpassDescription const> subpass_descriptions,
    stx::Span<VkSubpassDependency const> subpass_dependencies) {
  VkRenderPassCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = static_cast<uint32_t>(attachment_descriptions.size()),
      .pAttachments = attachment_descriptions.data(),
      .subpassCount = static_cast<uint32_t>(subpass_descriptions.size()),
      .pSubpasses = subpass_descriptions.data(),
      .dependencyCount = static_cast<uint32_t>(subpass_dependencies.size()),
      .pDependencies = subpass_dependencies.data()};

  VkRenderPass render_pass;
  ASR_MUST_SUCCEED(
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
    VkPipelineDynamicStateCreateInfo const& dynamic_state) {
  VkGraphicsPipelineCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pStages = shader_stages_create_infos.data(),
      .stageCount = static_cast<uint32_t>(shader_stages_create_infos.size()),
      .pVertexInputState = &vertex_input_state,
      .pInputAssemblyState = &input_assembly_state,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterization_state,
      .pMultisampleState = &multisample_state,
      .pDepthStencilState = &depth_stencil_state,
      .pColorBlendState = &color_blend_state,
      .pDynamicState =
          &dynamic_state,  // which of these fixed function states would change,
                           // any of the ones listed here would need to be
                           // provided at every draw/render call
      .layout = layout,
      .renderPass = render_pass,
      .subpass =
          0,  // index of the device's subpass this graphics pipeline belongs to
      .basePipelineHandle = nullptr,
      .basePipelineIndex = -1};

  VkPipeline graphics_pipeline;

  ASR_MUST_SUCCEED(
      vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create_info,
                                nullptr, &graphics_pipeline),
      "Unable to create graphics pipeline");

  return graphics_pipeline;
}

// basically a collection of attachments (color, depth, stencil, etc)
inline VkFramebuffer create_frame_buffer(
    VkDevice device, VkRenderPass render_pass,
    stx::Span<VkImageView const> attachments, VkExtent2D extent) {
  VkFramebufferCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = render_pass,
      .attachmentCount = static_cast<uint32_t>(attachments.size()),
      .pAttachments = attachments.data(),
      .width = extent.width,
      .height = extent.height,
      .layers = 1};  // our swap chain images are single images, so the
                     // number of layers is 1

  VkFramebuffer frame_buffer;
  ASR_MUST_SUCCEED(
      vkCreateFramebuffer(device, &create_info, nullptr, &frame_buffer),
      "Unable to create frame buffer");
  return frame_buffer;
}

inline VkCommandPool create_command_pool(
    VkDevice device, uint32_t queue_family_index,
    bool enable_command_buffer_resetting = false) {
  VkCommandPoolCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = queue_family_index,
      .flags = enable_command_buffer_resetting
                   ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
                   : static_cast<VkCommandPoolCreateFlagBits>(0)};

  VkCommandPool command_pool;
  ASR_MUST_SUCCEED(
      vkCreateCommandPool(device, &create_info, nullptr, &command_pool),
      "Unable to create command pool");

  return command_pool;
}

inline void allocate_command_buffer(VkDevice device, VkCommandPool command_pool,
                                    VkCommandBuffer& command_buffer  // NOLINT
) {
  VkCommandBufferAllocateInfo allocate_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command_pool,
      // VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for
      // execution, but cannot be called from other command buffers.
      // VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but
      // can be called from primary command buffers.
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

  ASR_MUST_SUCCEED(
      vkAllocateCommandBuffers(device, &allocate_info, &command_buffer),
      "Unable to allocate command buffer");
}

inline void allocate_command_buffers(
    VkDevice device, VkCommandPool command_pool,
    stx::Span<VkCommandBuffer> command_buffers) {
  VkCommandBufferAllocateInfo allocate_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command_pool,
      // VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for
      // execution, but cannot be called from other command buffers.
      // VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but
      // can be called from primary command buffers.
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32_t>(command_buffers.size())};

  ASR_MUST_SUCCEED(
      vkAllocateCommandBuffers(device, &allocate_info, command_buffers.data()),
      "Unable to allocate command buffers");
}

inline void reset_command_buffer(VkCommandBuffer command_buffer,
                                 bool release_resources = false) {
  ASR_MUST_SUCCEED(
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
    VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will
        // be rerecorded right after executing it once.
        // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary
        // command buffer that will be entirely within a single render pass.
        // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can
        // be resubmitted while it is also already pending execution
        .flags = usage,
        .pInheritanceInfo = inheritance_info};

    ASR_MUST_SUCCEED(vkBeginCommandBuffer(command_buffer_, &begin_info),
                     "unable to begin command buffer recording");

    return *this;
  }

  Recorder copy(VkBuffer src, uint64_t src_offset, uint64_t size, VkBuffer dst,
                uint64_t dst_offset) {
    VkBufferCopy copy_region{
        .dstOffset = dst_offset, .size = size, .srcOffset = src_offset};
    vkCmdCopyBuffer(command_buffer_, src, dst, 1, &copy_region);
    return *this;
  }

  // TODO(lamarrr): make into multi-copy interface
  Recorder copy(VkBuffer src, uint64_t src_offset, VkImage dst,
                VkImageLayout dst_expected_layout, VkOffset3D dst_offset,
                VkExtent3D dst_extent) {
    VkBufferImageCopy copy_region{
        .bufferOffset = src_offset,
        .bufferRowLength = 0,    // tightly-packed, no padding
        .bufferImageHeight = 0,  // tightly-packed, no padding
        .imageOffset = dst_offset,
        .imageExtent = dst_extent,
        .imageSubresource.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT,  // we want to copy the color components
                                        // of the pixels
        // TODO(lamarrr): remove hard-coding
        .imageSubresource.mipLevel = 0,
        .imageSubresource.baseArrayLayer = 0,
        .imageSubresource.layerCount = 1};

    vkCmdCopyBufferToImage(command_buffer_, src, dst, dst_expected_layout, 1,
                           &copy_region);
    return *this;
  }

  Recorder begin_render_pass(VkRenderPass render_pass,
                             VkFramebuffer framebuffer, VkRect2D render_area,
                             stx::Span<VkClearValue const> clear_values) {
    VkRenderPassBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass,
        .framebuffer = framebuffer,
        .renderArea = render_area,
        .clearValueCount = static_cast<uint32_t>(clear_values.size()),
        .pClearValues = clear_values.data()};

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

  Recorder bind_pipeline_barrier(
      VkPipelineStageFlagBits src_stages = {},
      VkPipelineStageFlagBits dst_stages = {},
      stx::Span<VkMemoryBarrier const> memory_barriers = {},
      stx::Span<VkBufferMemoryBarrier const> buffer_memory_barriers = {},
      stx::Span<VkImageMemoryBarrier const> iamge_memory_barriers = {}) {
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

  Recorder bind_descriptor_sets(
      VkPipelineLayout pipeline_layout, VkPipelineBindPoint bind_point,
      stx::Span<VkDescriptorSet const> descriptor_sets) {
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

  Recorder set_viewports(stx::Span<VkViewport const> viewports) {
    vkCmdSetViewport(command_buffer_, 0, viewports.size(), viewports.data());
    return *this;
  }

  Recorder set_scissors(stx::Span<VkRect2D const> scissors) {
    vkCmdSetScissor(command_buffer_, 0, scissors.size(), scissors.data());
    return *this;
  }

  Recorder set_line_width(float line_width) {
    vkCmdSetLineWidth(command_buffer_, line_width);
    return *this;
  }

  Recorder end_recording() {
    ASR_MUST_SUCCEED(vkEndCommandBuffer(command_buffer_),
                     "Unable to end command buffer recording");
    return *this;
  }
};
}  // namespace cmd

// GPU-GPU synchronization primitive, cheap
inline VkSemaphore create_semaphore(VkDevice device) {
  VkSemaphoreCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  VkSemaphore semaphore;
  ASR_MUST_SUCCEED(vkCreateSemaphore(device, &create_info, nullptr, &semaphore),
                   "Unable to create semaphore");

  return semaphore;
}

// GPU-CPU synchronization primitive, expensive
inline VkFence create_fence(VkDevice device,
                            VkFenceCreateFlagBits make_signaled) {
  VkFenceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                .flags = make_signaled};

  VkFence fence;

  ASR_MUST_SUCCEED(vkCreateFence(device, &create_info, nullptr, &fence),
                   "Unable to create fence");

  return fence;
}

inline void reset_fences(VkDevice device, stx::Span<VkFence const> fences) {
  ASR_MUST_SUCCEED(vkResetFences(device, fences.size(), fences.data()),
                   "Unable to reset fences");
}

inline void await_fences(VkDevice device, stx::Span<VkFence const> fences) {
  ASR_MUST_SUCCEED(
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
                            VkFence notify_fence) {
  ASR_ENSURE(await_semaphores.size() == await_stages.size(),
             "stages to await must have the same number of semaphores (for "
             "each of them)");

  VkSubmitInfo submit_info{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = static_cast<uint32_t>(await_semaphores.size()),
      .pWaitSemaphores = await_semaphores.data(),
      .pWaitDstStageMask = await_stages.data(),
      .commandBufferCount = 1,
      .pCommandBuffers = &command_buffer,
      .signalSemaphoreCount = static_cast<uint32_t>(notify_semaphores.size()),
      .pSignalSemaphores = notify_semaphores.data()};

  ASR_MUST_SUCCEED(vkQueueSubmit(command_queue, 1, &submit_info, notify_fence),
                   "Unable to submit command buffer to command queue");
}

inline std::pair<uint32_t, VkResult> acquire_next_swapchain_image(
    VkDevice device, VkSwapchainKHR swapchain, VkSemaphore signal_semaphore,
    VkFence signal_fence) {
  uint32_t index = 0;

  auto result = vkAcquireNextImageKHR(
      device, swapchain,
      std::chrono::duration_cast<std::chrono::nanoseconds>(1min).count(),
      signal_semaphore, signal_fence, &index);
  ASR_ENSURE(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
                 result == VK_ERROR_OUT_OF_DATE_KHR,
             "Unable to acquire next image");

  return std::make_pair(index, result);
}

inline VkResult present(VkQueue command_queue,
                        stx::Span<VkSemaphore const> await_semaphores,
                        stx::Span<VkSwapchainKHR const> swapchains,
                        stx::Span<uint32_t const> swapchain_image_indexes) {
  ASR_ENSURE(swapchain_image_indexes.size() == swapchains.size(),
             "swapchain and their image indices must be of the same size");

  VkPresentInfoKHR present_info{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = static_cast<uint32_t>(await_semaphores.size()),
      .pWaitSemaphores = await_semaphores.data(),
      .swapchainCount = static_cast<uint32_t>(swapchains.size()),
      .pSwapchains = swapchains.data(),
      .pImageIndices = swapchain_image_indexes.data(),
      .pResults = nullptr};

  auto result = vkQueuePresentKHR(command_queue, &present_info);
  ASR_ENSURE(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
                 result == VK_ERROR_OUT_OF_DATE_KHR,
             "Unable to present to swapchain");

  return result;
}

// creates buffer object but doesn't assign memory to it
inline VkBuffer create_buffer(VkDevice device, uint64_t byte_size,
                              VkBufferUsageFlagBits usage,
                              VkSharingMode sharing_mode) {
  VkBufferCreateInfo buffer_info{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .size = byte_size,
                                 .usage = usage,
                                 .sharingMode = sharing_mode};

  VkBuffer buffer;
  ASR_MUST_SUCCEED(vkCreateBuffer(device, &buffer_info, nullptr, &buffer),
                   "Unable to create buffer");
  return buffer;
}

// creates image but doesn't assign memory to it
// different image layouts are suitable for different image operations
inline VkImage create_image(VkDevice device, VkImageType type,
                            VkExtent3D extent, VkImageUsageFlagBits usage,
                            VkSharingMode sharing_mode, VkFormat format,
                            VkImageLayout initial_layout) {
  VkImageCreateInfo image_info{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                               .usage = usage,
                               .imageType = type,
                               .extent = extent,
                               .sharingMode = sharing_mode,
                               .mipLevels = 1,
                               .arrayLayers = 1,
                               .format = format,
                               .tiling = VK_IMAGE_TILING_OPTIMAL,
                               .initialLayout = initial_layout,
                               .samples = VK_SAMPLE_COUNT_1_BIT,
                               .flags = 0};

  VkImage image;
  ASR_MUST_SUCCEED(vkCreateImage(device, &image_info, nullptr, &image),
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
    VkAccessFlags src_access_flags, VkAccessFlags dst_access_flags) {
  VkImageMemoryBarrier barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex =
          VK_QUEUE_FAMILY_IGNORED,  // not transferring ownership of the image
      .dstQueueFamilyIndex =
          VK_QUEUE_FAMILY_IGNORED,  // not transferring ownership of the image
      .image = image,
      .subresourceRange.aspectMask =
          VK_IMAGE_ASPECT_COLOR_BIT,  // part of the image
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.levelCount = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount = 1,
      .srcAccessMask = src_access_flags,
      .dstAccessMask = dst_access_flags};

  return barrier;
}

/*
// get memory requirements for a buffer based on it's type, usage mode, and
// other properties
inline VkMemoryRequirements get_memory_requirements(VkDevice device,
                                                    VkBuffer buffer)  {
  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);
  return memory_requirements;
}

// get memory requirements for an image based on it's type, usage mode, and
// other properties
inline VkMemoryRequirements get_memory_requirements(VkDevice device,
                                                    VkImage image)  {
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(device, image, &memory_requirements);
  return memory_requirements;
}

// returns index of the heap on the physical device, could be RAM, SWAP, or VRAM
inline stx::Option<uint32_t> find_suitable_memory_type(
    VkPhysicalDevice phy_device,
    VkMemoryRequirements const& memory_requirements,
    VkMemoryPropertyFlagBits required_properties =
        VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM)  {
  VkPhysicalDeviceMemoryProperties memory_properties;

  vkGetPhysicalDeviceMemoryProperties(phy_device, &memory_properties);
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
                                      uint64_t size)  {
  VkMemoryAllocateInfo allocate_info{};

  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = size;
  allocate_info.memoryTypeIndex = heap_index;

  VkDeviceMemory memory;
  ASR_MUST_SUCCEED(vkAllocateMemory(device, &allocate_info, nullptr, &memory),
                   "Unable to allocate memory");

  return memory;
}

inline void bind_memory_to_buffer(VkDevice device, VkBuffer buffer,
                                  VkDeviceMemory memory,
                                  uint64_t offset)  {
  ASR_MUST_SUCCEED(vkBindBufferMemory(device, buffer, memory, offset),
                   "Unable to bind memory to buffer");
}

inline void bind_memory_to_image(VkDevice device, VkImage image,
                                 VkDeviceMemory memory,
                                 uint64_t offset)  {
  ASR_MUST_SUCCEED(vkBindImageMemory(device, image, memory, offset),
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
                            VkMemoryMapFlags flags = 0)  {
  void* ptr;
  ASR_MUST_SUCCEED(vkMapMemory(device, memory, offset, size, flags, &ptr),
                   "Unable to map memory");
  return MemoryMap{offset,
                   stx::Span<uint8_t>{reinterpret_cast<uint8_t*>(ptr), size}};
}

// unlike OpenGL the driver may not immediately copy the data after unmap, i.e.
// due to caching. so we need to flush our writes
inline void unmap_memory(VkDevice device, VkDeviceMemory memory)  {
  vkUnmapMemory(device, memory);
}

// due to caching we need to flush writes to the memory map before reading again
// has size requirements for the flush range
inline void flush_memory_map(VkDevice device, VkDeviceMemory memory,
                             uint64_t offset, uint64_t size)  {
  VkMappedMemoryRange range{};
  range.memory = memory;
  range.offset = offset;
  range.size = size;
  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

  ASR_MUST_SUCCEED(vkFlushMappedMemoryRanges(device, 1, &range),
                   "Unable to flush memory map");
}

inline void refresh_memory_map(VkDevice device, VkDeviceMemory memory,
                               uint64_t offset, uint64_t size)  {
  VkMappedMemoryRange range{};
  range.memory = memory;
  range.offset = offset;
  range.size = size;
  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

  ASR_MUST_SUCCEED(vkInvalidateMappedMemoryRanges(device, 1, &range),
                   "Unable to re-read memory map");
}
*/

constexpr VkDescriptorSetLayoutBinding make_descriptor_set_layout_binding(
    uint32_t binding,
    uint32_t descriptor_count  // number of objects being described starting
                               // from {binding}
    ,
    VkDescriptorType descriptor_type, VkShaderStageFlagBits shader_stages,
    VkSampler const* sampler) {
  VkDescriptorSetLayoutBinding dsl_binding{.binding = binding,
                                           .descriptorType = descriptor_type,
                                           .descriptorCount = descriptor_count,
                                           .pImmutableSamplers = sampler,
                                           .stageFlags = shader_stages};

  return dsl_binding;
}

// descriptor sets define the input data for the uniforms (or samplers)
inline VkDescriptorSetLayout create_descriptor_set_layout(
    VkDevice device, stx::Span<VkDescriptorSetLayoutBinding const> bindings,
    VkDescriptorSetLayoutCreateFlagBits flags = {}) {
  VkDescriptorSetLayoutCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data(),
      .pNext = nullptr,
      .flags = flags};

  VkDescriptorSetLayout ds_layout;
  ASR_MUST_SUCCEED(
      vkCreateDescriptorSetLayout(device, &create_info, nullptr, &ds_layout),
      "Unable to create descriptor set layout");

  return ds_layout;
}

inline VkDescriptorPool create_descriptor_pool(
    VkDevice device, uint32_t max_descriptor_sets,
    stx::Span<VkDescriptorPoolSize const> pool_sizing) {
  // create pool capable of holding different types of data with varying number
  // of descriptors
  VkDescriptorPoolCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .poolSizeCount = static_cast<uint32_t>(pool_sizing.size()),
      .pPoolSizes = pool_sizing.data(),
      .maxSets = max_descriptor_sets  /// desc sets is
                                      // a set with similar properties (can be
                                      // by type and are not necessarily unique
                                      // as the name might imply)
  };

  VkDescriptorPool descriptor_pool;
  ASR_MUST_SUCCEED(
      vkCreateDescriptorPool(device, &create_info, nullptr, &descriptor_pool),
      "Unable to create descriptor pool");

  return descriptor_pool;
}

// each descriptor set represents a descriptor for a certain buffer type i.e.
// DESCRIPTOR_TYPE_UNIFORM_BUFFER
inline void allocate_descriptor_sets(
    VkDevice device, VkDescriptorPool descriptor_pool,
    stx::Span<VkDescriptorSetLayout const> layouts,
    stx::Span<VkDescriptorSet> descriptor_sets) {
  ASR_ENSURE(layouts.size() == descriptor_sets.size(),
             "descriptor sets and layouts sizes must match");

  VkDescriptorSetAllocateInfo info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptor_pool,
      .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data()};

  ASR_MUST_SUCCEED(
      vkAllocateDescriptorSets(device, &info, descriptor_sets.data()),
      "Unable to create descriptor sets");
}

// descriptor set writer interface, can write multiple objects of the same time
// type in one pass (images, buffers, texels, etc.)
/*
struct DescriptorSetProxy {
  VkDevice device;
  VkDescriptorSet descriptor_set;
  VkDescriptorType descriptor_type;
  uint32_t binding;

  DescriptorSetProxy bind_buffers(
      stx::Span<VkDescriptorBufferInfo const> buffers) {
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
      stx::Span<VkDescriptorImageInfo const> images) {
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
*/

constexpr std::string_view format(VkFormat format) {
  switch (format) {
    ASR_ERRNUM_CASE(VK_FORMAT_UNDEFINED)
    ASR_ERRNUM_CASE(VK_FORMAT_R4G4_UNORM_PACK8)
    ASR_ERRNUM_CASE(VK_FORMAT_R4G4B4A4_UNORM_PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_B4G4R4A4_UNORM_PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_R5G6B5_UNORM_PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_B5G6R5_UNORM_PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_R5G5B5A1_UNORM_PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_B5G5R5A1_UNORM_PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_A1R5G5B5_UNORM_PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_R8_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R8_SNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R8_USCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R8_SSCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R8_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R8_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R8_SRGB)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8_SNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8_USCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8_SSCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8_SRGB)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8_SNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8_USCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8_SSCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8_SRGB)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8_SNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8_USCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8_SSCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8_SRGB)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_SNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_USCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_SSCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R8G8B8A8_SRGB)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_SNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_USCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_SSCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8A8_SRGB)
    ASR_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_UNORM_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_SNORM_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_USCALED_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_SSCALED_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_UINT_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_SINT_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A8B8G8R8_SRGB_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_UNORM_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_SNORM_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_USCALED_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_SSCALED_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_UINT_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2R10G10B10_SINT_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_UNORM_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_SNORM_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_USCALED_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_SSCALED_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_UINT_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_A2B10G10R10_SINT_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_R16_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R16_SNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R16_USCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R16_SSCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R16_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16_SNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16_USCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16_SSCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16_SNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16_USCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16_SSCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_SNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_USCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_SSCALED)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R16G16B16A16_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32G32_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32G32_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32G32_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32G32B32_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32G32B32_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32G32B32_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32G32B32A32_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32G32B32A32_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R32G32B32A32_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64G64_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64G64_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64G64_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64G64B64_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64G64B64_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64G64B64_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64G64B64A64_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64G64B64A64_SINT)
    ASR_ERRNUM_CASE(VK_FORMAT_R64G64B64A64_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_B10G11R11_UFLOAT_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_D16_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_X8_D24_UNORM_PACK32)
    ASR_ERRNUM_CASE(VK_FORMAT_D32_SFLOAT)
    ASR_ERRNUM_CASE(VK_FORMAT_S8_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_D16_UNORM_S8_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_D24_UNORM_S8_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_D32_SFLOAT_S8_UINT)
    ASR_ERRNUM_CASE(VK_FORMAT_BC1_RGB_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC1_RGB_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC1_RGBA_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC1_RGBA_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC2_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC2_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC3_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC3_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC4_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC4_SNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC5_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC5_SNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC6H_UFLOAT_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC6H_SFLOAT_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC7_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_BC7_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_EAC_R11_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_EAC_R11_SNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_EAC_R11G11_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_EAC_R11G11_SNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_4x4_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_4x4_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_5x4_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_5x4_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_5x5_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_5x5_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_6x5_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_6x5_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_6x6_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_6x6_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_8x5_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_8x5_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_8x6_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_8x6_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_8x8_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_8x8_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_10x5_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_10x5_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_10x6_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_10x6_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_10x8_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_10x8_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_10x10_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_10x10_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_12x10_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_12x10_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_12x12_UNORM_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_ASTC_12x12_SRGB_BLOCK)
    ASR_ERRNUM_CASE(VK_FORMAT_G8B8G8R8_422_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_B8G8R8G8_422_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_R10X6_UNORM_PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_R10X6G10X6_UNORM_2PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_R12X4_UNORM_PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_R12X4G12X4_UNORM_2PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16)
    ASR_ERRNUM_CASE(VK_FORMAT_G16B16G16R16_422_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_B16G16R16G16_422_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM)
    ASR_ERRNUM_CASE(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG)
    ASR_ERRNUM_CASE(VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG)
    ASR_ERRNUM_CASE(VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG)
    ASR_ERRNUM_CASE(VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG)
    ASR_ERRNUM_CASE(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG)
    ASR_ERRNUM_CASE(VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG)
    ASR_ERRNUM_CASE(VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG)
    ASR_ERRNUM_CASE(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG)
    default:
      return "Unidentified Format Enum";
  }
}

constexpr std::string_view format(VkResult error) {
  switch (error) {
    ASR_ERRNUM_CASE(VK_SUCCESS)
    ASR_ERRNUM_CASE(VK_NOT_READY)
    ASR_ERRNUM_CASE(VK_TIMEOUT)
    ASR_ERRNUM_CASE(VK_EVENT_SET)
    ASR_ERRNUM_CASE(VK_EVENT_RESET)
    ASR_ERRNUM_CASE(VK_INCOMPLETE)
    ASR_ERRNUM_CASE(VK_ERROR_OUT_OF_HOST_MEMORY)
    ASR_ERRNUM_CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY)
    ASR_ERRNUM_CASE(VK_ERROR_INITIALIZATION_FAILED)
    ASR_ERRNUM_CASE(VK_ERROR_DEVICE_LOST)
    ASR_ERRNUM_CASE(VK_ERROR_MEMORY_MAP_FAILED)
    ASR_ERRNUM_CASE(VK_ERROR_LAYER_NOT_PRESENT)
    ASR_ERRNUM_CASE(VK_ERROR_EXTENSION_NOT_PRESENT)
    ASR_ERRNUM_CASE(VK_ERROR_FEATURE_NOT_PRESENT)
    ASR_ERRNUM_CASE(VK_ERROR_INCOMPATIBLE_DRIVER)
    ASR_ERRNUM_CASE(VK_ERROR_TOO_MANY_OBJECTS)
    ASR_ERRNUM_CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
    ASR_ERRNUM_CASE(VK_ERROR_FRAGMENTED_POOL)
    // ASR_ERRNUM_CASE( VK_ERROR_UNKNOWN)

    // Provided by VK_VERSION_1_1
    ASR_ERRNUM_CASE(VK_ERROR_OUT_OF_POOL_MEMORY)
    ASR_ERRNUM_CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE)

    // Provided by VK_VERSION_1_2
    ASR_ERRNUM_CASE(VK_ERROR_FRAGMENTATION_EXT)
    // ASR_ERRNUM_CASE( VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)

    // Provided by VK_KHR_surface
    ASR_ERRNUM_CASE(VK_ERROR_SURFACE_LOST_KHR)
    ASR_ERRNUM_CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)

    // Provided by VK_KHR_swapchain
    ASR_ERRNUM_CASE(VK_SUBOPTIMAL_KHR)
    ASR_ERRNUM_CASE(VK_ERROR_OUT_OF_DATE_KHR)

    // Provided by VK_KHR_display_swapchain
    ASR_ERRNUM_CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)

    // Provided by VK_EXT_debug_report
    ASR_ERRNUM_CASE(VK_ERROR_VALIDATION_FAILED_EXT)

    // Provided by VK_NV_glsl_shader
    ASR_ERRNUM_CASE(VK_ERROR_INVALID_SHADER_NV)

    // Provided by VK_EXT_image_drm_format_modifier
    // ASR_ERRNUM_CASE( VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)

    // Provided by VK_EXT_global_priority
    ASR_ERRNUM_CASE(VK_ERROR_NOT_PERMITTED_EXT)

    // Provided by VK_EXT_full_screen_exclusive
    // ASR_ERRNUM_CASE( VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)

    // Provided by VK_KHR_deferred_host_operations
    // ASR_ERRNUM_CASE( VK_THREAD_IDLE_KHR)
    // ASR_ERRNUM_CASE( VK_THREAD_DONE_KHR)
    // ASR_ERRNUM_CASE( VK_OPERATION_DEFERRED_KHR)
    // ASR_ERRNUM_CASE( VK_OPERATION_NOT_DEFERRED_KHR)

    // Provided by VK_EXT_pipeline_creation_cache_control
    // ASR_ERRNUM_CASE( VK_PIPELINE_COMPILE_REQUIRED_EXT)

    ASR_ERRNUM_CASE(VK_RESULT_MAX_ENUM)

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
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_DCI_P3_LINEAR_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_BT709_LINEAR_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_BT709_NONLINEAR_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_BT2020_LINEAR_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_HDR10_ST2084_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_DOLBYVISION_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_HDR10_HLG_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_PASS_THROUGH_EXT)
    ASR_ERRNUM_CASE(VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT)
    default:
      return "unidentified color space";
  }
}

}  // namespace vk

namespace vkh {
struct Instance {
  VkInstance instance = VK_NULL_HANDLE;
  stx::Option<VkDebugUtilsMessengerEXT> debug_messenger = stx::None;

  Instance(VkInstance ainstance,
           stx::Option<VkDebugUtilsMessengerEXT> adebug_messenger)
      : instance{ainstance}, debug_messenger{std::move(adebug_messenger)} {}

  STX_MAKE_PINNED(Instance)

  ~Instance() {
    if (debug_messenger.is_some()) {
      vk::destroy_debug_messenger(instance, debug_messenger.value());
    }
    vkDestroyInstance(instance, nullptr);
  }
};

struct PhyDeviceInfo {
  VkPhysicalDevice phy_device = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties properties{};
  VkPhysicalDeviceFeatures features{};
  stx::Vec<VkQueueFamilyProperties> family_properties{stx::os_allocator};
  stx::Rc<Instance*> instance;

  PhyDeviceInfo copy() const {
    stx::Vec<VkQueueFamilyProperties> nfamily_properties{stx::os_allocator};

    for (auto& prop : family_properties) {
      nfamily_properties.push_inplace(prop).unwrap();
    }

    return PhyDeviceInfo{phy_device, properties, features,
                         std::move(nfamily_properties), instance.share()};
  }

  bool has_geometry_shader() const { return features.geometryShader; }

  bool has_transfer_command_queue_family() const {
    return family_properties.span().is_any(
        [](VkQueueFamilyProperties const& prop) -> bool {
          return prop.queueFlags & VK_QUEUE_TRANSFER_BIT;
        });
  }

  bool has_graphics_command_queue_family() const {
    return family_properties.span().is_any(
        [](VkQueueFamilyProperties const& prop) -> bool {
          return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        });
  }
};

inline stx::Vec<PhyDeviceInfo> get_all_devices(
    stx::Rc<Instance*> const& instance) {
  uint32_t devices_count = 0;

  ASR_MUST_SUCCEED(vkEnumeratePhysicalDevices(instance.handle->instance,
                                              &devices_count, nullptr),
                   "Unable to get physical devices");

  ASR_ENSURE(devices_count != 0, "No Physical Device Found");

  stx::Vec<VkPhysicalDevice> phy_devices{stx::os_allocator};

  phy_devices.resize(devices_count).unwrap();

  ASR_MUST_SUCCEED(
      vkEnumeratePhysicalDevices(instance.handle->instance, &devices_count,
                                 phy_devices.data()),
      "Unable to get physical devices");

  stx::Vec<PhyDeviceInfo> devices{stx::os_allocator};

  for (VkPhysicalDevice device : phy_devices) {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;

    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);

    devices
        .push(PhyDeviceInfo{.phy_device = device,
                            .properties = device_properties,
                            .features = device_features,
                            .family_properties = vk::get_queue_families(device),
                            .instance = instance.share()})
        .unwrap();
  }

  return devices;
}

inline std::string format(PhyDeviceInfo const& device) {
  auto const& properties = device.properties;
  return fmt::format("Device(name: '{}', ID: {}, type: {}) ",
                     properties.deviceName, properties.deviceID,
                     ::asr::vk::format(properties.deviceType));
}

struct CommandQueueFamilyInfo {
  // automatically destroyed once the device is destroyed
  uint32_t index = 0;
  stx::Rc<PhyDeviceInfo*> phy_device;
};

struct Device;

struct CommandQueueInfo {
  // automatically destroyed once the device is destroyed
  VkQueue queue = VK_NULL_HANDLE;
  uint32_t create_index = 0;
  float priority = 0.0f;
  CommandQueueFamilyInfo family;
};

struct CommandQueue {
  CommandQueueInfo info;
  stx::Rc<Device*> device;
};

struct Device {
  VkDevice device = VK_NULL_HANDLE;
  stx::Rc<PhyDeviceInfo*> phy_device;
  stx::Vec<CommandQueueInfo> command_queues{stx::os_allocator};

  Device(VkDevice adevice, stx::Rc<PhyDeviceInfo*> aphy_device,
         stx::Vec<CommandQueueInfo> acommand_queues)
      : device{adevice},
        phy_device{std::move(aphy_device)},
        command_queues{std::move(acommand_queues)} {}

  STX_MAKE_PINNED(Device)

  ~Device() { vkDestroyDevice(device, nullptr); }
};

inline stx::Rc<Instance*> create_instance(
    char const* app_name, uint32_t app_version, char const* engine_name,
    uint32_t engine_version,
    stx::Span<char const* const> required_extensions = {},
    stx::Span<char const* const> validation_layers = {}) {
  auto [instance, messenger] = vk::create_vulkan_instance(
      required_extensions, validation_layers,
      vk::make_debug_messenger_create_info(), app_name, app_version,
      engine_name, engine_version);

  return stx::rc::make_inplace<Instance>(
             stx::os_allocator, instance,
             messenger == VK_NULL_HANDLE ? stx::None
                                         : stx::make_some(std::move(messenger)))
      .unwrap();
}

// can also be used for transfer
inline stx::Option<CommandQueueFamilyInfo> get_graphics_command_queue(
    stx::Rc<PhyDeviceInfo*> const& phy_device) {
  auto pos = std::find_if(phy_device.handle->family_properties.begin(),
                          phy_device.handle->family_properties.end(),
                          [](VkQueueFamilyProperties const& prop) -> bool {
                            return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                          });

  if (pos == phy_device.handle->family_properties.end()) {
    return stx::None;
  }

  return stx::Some(CommandQueueFamilyInfo{
      .index = static_cast<uint32_t>(
          pos - phy_device.handle->family_properties.begin()),
      .phy_device = phy_device.share()});
}

inline stx::Rc<Device*> create_device(
    stx::Rc<PhyDeviceInfo*> const& phy_device,
    stx::Span<VkDeviceQueueCreateInfo const> command_queue_create_info,
    stx::Span<char const* const> required_extensions = {},
    stx::Span<char const* const> required_validation_layers = {},
    VkPhysicalDeviceFeatures required_features = {}) {
  VkDevice device = vk::create_logical_device(
      phy_device.handle->phy_device, required_extensions,
      required_validation_layers, command_queue_create_info, required_features);

  stx::Vec<CommandQueueInfo> command_queues{stx::os_allocator};

  for (size_t i = 0; i < command_queue_create_info.size(); i++) {
    auto create_info = command_queue_create_info[i];
    auto command_queue_family_index = create_info.queueFamilyIndex;
    auto queue_count = create_info.queueCount;
    ASR_ENSURE(command_queue_family_index <
               phy_device.handle->family_properties.size());

    for (uint32_t queue_index_in_family = 0;
         queue_index_in_family < queue_count; queue_index_in_family++) {
      float priority = create_info.pQueuePriorities[i];
      VkQueue command_queue = vk::get_command_queue(
          device, command_queue_family_index, queue_index_in_family);

      command_queues
          .push(CommandQueueInfo{
              .queue = command_queue,
              .create_index = static_cast<uint32_t>(i),
              .priority = priority,
              .family =
                  CommandQueueFamilyInfo{.index = command_queue_family_index,
                                         .phy_device = phy_device.share()},
          })
          .unwrap();
    }
  }

  return stx::rc::make_inplace<Device>(stx::os_allocator, device,
                                       phy_device.share(),
                                       std::move(command_queues))
      .unwrap();
}

inline stx::Option<CommandQueue> get_command_queue(
    stx::Rc<Device*> const& device, CommandQueueFamilyInfo const& family,
    uint32_t command_queue_create_index) {
  // We shouldn't have to perform checks?
  ASR_ENSURE(device.handle->phy_device.handle->phy_device ==
             family.phy_device.handle->phy_device);

  stx::Span queue_s = device.handle->command_queues.span().which(
      [&](CommandQueueInfo const& info) {
        return info.family.index == family.index &&
               info.create_index == command_queue_create_index;
      });

  if (queue_s.is_empty()) return stx::None;

  auto& queue = queue_s[0];

  return stx::Some(CommandQueue{
      .info = CommandQueueInfo{.queue = queue.queue,
                               .create_index = queue.create_index,
                               .priority = queue.priority,
                               .family =
                                   CommandQueueFamilyInfo{
                                       .index = queue.family.index,
                                       .phy_device =
                                           queue.family.phy_device.share()}},
      .device = device.share()});
}

/*
struct AllocatorHandle {
  VmaAllocator allocator = nullptr;
  Device device;

  ASR_DISABLE_COPY(AllocatorHandle)
ASR_DISABLE_MOVE(AllocatorHandle)

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
        device.handle->phy_device.info.properties.apiVersion;
    info.device = device.handle->device;
    info.physicalDevice = device.handle->phy_device.info.phy_device;
    info.instance = device.handle->phy_device.info.instance.handle->instance;

    auto handle = std::shared_ptr<AllocatorHandle>(new AllocatorHandle{});

    handle->device = device;

    ASR_MUST_SUCCEED(vmaCreateAllocator(&info, &handle->allocator),
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

  ASR_DISABLE_COPY(ImageHandle)
ASR_DISABLE_MOVE(ImageHandle)

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
    ASR_ENSURE(extent.visible());

    VkImageCreateInfo info{};

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

    ASR_MUST_SUCCEED(result, "Unable to create image on device");
  }

  std::shared_ptr<ImageHandle> handle;
};

struct ImageViewHandle {
  VkImageView view = nullptr;
  Image image;

  ASR_DISABLE_COPY(ImageViewHandle)
ASR_DISABLE_MOVE(ImageViewHandle)

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

*/

}  // namespace vkh
}  // namespace asr
