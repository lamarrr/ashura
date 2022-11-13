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

#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "stx/backtrace.h"
#include "stx/limits.h"
#include "stx/option.h"
#include "stx/result.h"
#include "stx/span.h"
#include "stx/vec.h"
#include "vulkan/vulkan.h"

#if STX_CFG(COMPILER, CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreorder-init-list"
#endif

#define ASR_VK_CHECK(...)                      \
  ASR_ENSURE((__VA_ARGS__) == VK_SUCCESS,      \
             "Vulkan Operation (" #__VA_ARGS__ \
             ")  failed (VK_SUCCESS not returned)")

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
  u32 available_validation_layers_count;
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
  u32 available_vk_extensions_count = 0;
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
    u32 application_version = VK_MAKE_VERSION(1, 0, 0),
    char const* const engine_name = "Valkyrie Engine",
    u32 engine_version = VK_MAKE_VERSION(1, 0, 0)) {
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
  u32 queue_families_count;

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

  for (usize i = 0; i < queue_families.size(); i++) {
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
  u32 available_extensions_count;
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

  VkDeviceCreateInfo device_create_info{
      .pQueueCreateInfos = command_queue_create_infos.data(),
      .queueCreateInfoCount =
          static_cast<u32>(command_queue_create_infos.size()),
      .pEnabledFeatures = &required_features,
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .ppEnabledLayerNames = required_validation_layers.data(),
      .enabledLayerCount = static_cast<u32>(required_validation_layers.size()),
      .ppEnabledExtensionNames = required_extensions.data(),
      .enabledExtensionCount = static_cast<u32>(required_extensions.size())};

  VkDevice logical_device;

  ASR_VK_CHECK(vkCreateDevice(phy_device, &device_create_info, nullptr,
                              &logical_device));

  return logical_device;
}

inline VkQueue get_command_queue(VkDevice device, u32 queue_family_index,
                                 u32 command_queue_index_in_family) {
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

  u32 supported_surface_formats_count = 0;

  ASR_MUST_SUCCEED(
      vkGetPhysicalDeviceSurfaceFormatsKHR(
          phy_device, surface, &supported_surface_formats_count, nullptr),
      "Unable to get physical device' surface format");

  details.supported_formats.resize(supported_surface_formats_count).unwrap();

  ASR_MUST_SUCCEED(vkGetPhysicalDeviceSurfaceFormatsKHR(
                       phy_device, surface, &supported_surface_formats_count,
                       details.supported_formats.data()),
                   "Unable to get physical device' surface format");

  u32 surface_presentation_modes_count;
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
inline u32 select_swapchain_image_count(
    VkSurfaceCapabilitiesKHR const& capabilities, u32 desired_num_buffers) {
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
    stx::Span<u32 const> accessing_queue_families_indexes,
    VkImageUsageFlags image_usages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    VkCompositeAlphaFlagBitsKHR alpha_channel_blending =
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    VkBool32 clipped = VK_TRUE) {
  u32 desired_num_buffers = std::max(properties.capabilities.minImageCount + 1,
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
          static_cast<u32>(accessing_queue_families_indexes.size())};

  VkSwapchainKHR swapchain;
  ASR_MUST_SUCCEED(
      vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain),
      "Unable to create swapchain");

  return std::make_pair(swapchain, selected_extent);
}

inline stx::Vec<VkImage> get_swapchain_images(VkDevice device,
                                              VkSwapchainKHR swapchain) {
  u32 image_count;

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
    u32 queue_family_index, stx::Span<f32 const> queues_priorities) {
  VkDeviceQueueCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queue_family_index,
      .pQueuePriorities = queues_priorities.data(),
      .queueCount = static_cast<u32>(
          queues_priorities
              .size())  // the number of queues we want, since multiple
                        // queues can belong to a single family
  };
  return create_info;
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
      .subresourceRange = VkImageSubresourceRange{.aspectMask = aspect_mask,
                                                  .baseArrayLayer = 0,
                                                  .baseMipLevel = 0,
                                                  .layerCount = 1,
                                                  .levelCount = 1}};

  VkImageView image_view;
  ASR_MUST_SUCCEED(
      vkCreateImageView(device, &create_info, nullptr, &image_view),
      "Unable to create image view");
  return image_view;
}

inline VkSampler create_sampler(VkDevice device,
                                stx::Option<f32> max_anisotropy) {
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
          static_cast<u32>(vertex_binding_descriptions.size()),
      .pVertexBindingDescriptions = vertex_binding_descriptions.data(),
      .vertexAttributeDescriptionCount =
          static_cast<u32>(vertex_attribute_desciptions.size()),
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

constexpr VkPipelineViewportStateCreateInfo
make_pipeline_viewport_state_create_info(stx::Span<VkViewport const> viewports,
                                         stx::Span<VkRect2D const> scissors) {
  // to use multiple viewports, ensure the GPU feature is enabled during logical
  // device creation
  VkPipelineViewportStateCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = static_cast<u32>(viewports.size()),
      .pViewports = viewports.data(),
      .scissorCount = static_cast<u32>(
          scissors.size()),  // scissors cut out the part to be rendered
      .pScissors = scissors.data()};

  return create_info;
}

constexpr VkPipelineRasterizationStateCreateInfo
make_pipeline_rasterization_create_info(VkFrontFace front_face,
                                        f32 line_width = 1.0f) {
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
      .attachmentCount = static_cast<u32>(
          color_frame_buffers.size()),  // number of framebuffers
      .pAttachments = color_frame_buffers.data(),
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};

  return create_info;
}

constexpr VkPipelineDynamicStateCreateInfo make_pipeline_dynamic_state(
    stx::Span<VkDynamicState const> dynamic_states) {
  // This will cause the configuration of these values to be ignored and you
  // will be required to specify the data at drawing time. This struct can be
  // substituted by a nullptr later on if you don't have any dynamic state.

  VkPipelineDynamicStateCreateInfo pipeline_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<u32>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data()};

  return pipeline_state;
}

inline VkPipelineLayout create_pipeline_layout(
    VkDevice device,
    stx::Span<VkDescriptorSetLayout const> descriptor_sets_layout,
    stx::Span<VkPushConstantRange const> constant_ranges) {
  VkPipelineLayoutCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = static_cast<u32>(descriptor_sets_layout.size()),
      .pSetLayouts = descriptor_sets_layout.data(),
      .pushConstantRangeCount = static_cast<u32>(constant_ranges.size()),
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
      .colorAttachmentCount = static_cast<u32>(color_attachments.size()),
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
      .attachmentCount = static_cast<u32>(attachment_descriptions.size()),
      .pAttachments = attachment_descriptions.data(),
      .subpassCount = static_cast<u32>(subpass_descriptions.size()),
      .pSubpasses = subpass_descriptions.data(),
      .dependencyCount = static_cast<u32>(subpass_dependencies.size()),
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
      .stageCount = static_cast<u32>(shader_stages_create_infos.size()),
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
      .attachmentCount = static_cast<u32>(attachments.size()),
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
    VkDevice device, u32 queue_family_index,
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
      .commandBufferCount = static_cast<u32>(command_buffers.size())};

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
      .waitSemaphoreCount = static_cast<u32>(await_semaphores.size()),
      .pWaitSemaphores = await_semaphores.data(),
      .pWaitDstStageMask = await_stages.data(),
      .commandBufferCount = 1,
      .pCommandBuffers = &command_buffer,
      .signalSemaphoreCount = static_cast<u32>(notify_semaphores.size()),
      .pSignalSemaphores = notify_semaphores.data()};

  ASR_MUST_SUCCEED(vkQueueSubmit(command_queue, 1, &submit_info, notify_fence),
                   "Unable to submit command buffer to command queue");
}

inline std::pair<u32, VkResult> acquire_next_swapchain_image(
    VkDevice device, VkSwapchainKHR swapchain, VkSemaphore signal_semaphore,
    VkFence signal_fence) {
  u32 index = 0;

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
                        stx::Span<u32 const> swapchain_image_indexes) {
  ASR_ENSURE(swapchain_image_indexes.size() == swapchains.size(),
             "swapchain and their image indices must be of the same size");

  VkPresentInfoKHR present_info{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = static_cast<u32>(await_semaphores.size()),
      .pWaitSemaphores = await_semaphores.data(),
      .swapchainCount = static_cast<u32>(swapchains.size()),
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
inline VkBuffer create_buffer(VkDevice device, u64 byte_size,
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
      .subresourceRange =
          VkImageSubresourceRange{
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,  // part of the image
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1},
      .srcAccessMask = src_access_flags,
      .dstAccessMask = dst_access_flags};

  return barrier;
}

// get memory requirements for an image based on it's type, usage mode, and
// other properties
inline VkMemoryRequirements get_memory_requirements(VkDevice device,
                                                    VkImage image) {
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(device, image, &memory_requirements);
  return memory_requirements;
}

// returns index of the heap on the physical device, could be RAM, SWAP, or VRAM
inline stx::Option<u32> find_suitable_memory_type(
    VkMemoryRequirements const& memory_requirements,
    VkPhysicalDeviceMemoryProperties const& memory_properties,
    VkMemoryPropertyFlagBits required_properties) {
  // different types of memory exist within the graphics card heap memory.
  // this can affect performance.

  for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
    if ((memory_requirements.memoryTypeBits & (1 << i)) &&
        ((required_properties &
          memory_properties.memoryTypes[i].propertyFlags) ==
         required_properties)) {
      return stx::Some(std::move(i));
    }
  }

  return stx::None;
}

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
  VkPhysicalDeviceMemoryProperties memory_properties{};
  stx::Vec<VkQueueFamilyProperties> family_properties{stx::os_allocator};
  stx::Rc<Instance*> instance;

  PhyDeviceInfo copy() const {
    stx::Vec<VkQueueFamilyProperties> nfamily_properties{stx::os_allocator};

    for (auto& prop : family_properties) {
      nfamily_properties.push_inplace(prop).unwrap();
    }

    return PhyDeviceInfo{phy_device,
                         properties,
                         features,
                         memory_properties,
                         std::move(nfamily_properties),
                         instance.share()};
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
  u32 devices_count = 0;

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
    VkPhysicalDeviceMemoryProperties memory_properties;

    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);
    vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

    devices
        .push(PhyDeviceInfo{.phy_device = device,
                            .properties = device_properties,
                            .features = device_features,
                            .memory_properties = memory_properties,
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
  u32 index = 0;
  stx::Rc<PhyDeviceInfo*> phy_device;
};

struct Device;

struct CommandQueueInfo {
  // automatically destroyed once the device is destroyed
  VkQueue queue = VK_NULL_HANDLE;
  u32 create_index = 0;
  f32 priority = 0.0f;
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
    char const* app_name, u32 app_version, char const* engine_name,
    u32 engine_version, stx::Span<char const* const> required_extensions = {},
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
      .index =
          static_cast<u32>(pos - phy_device.handle->family_properties.begin()),
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

  for (usize i = 0; i < command_queue_create_info.size(); i++) {
    auto create_info = command_queue_create_info[i];
    auto command_queue_family_index = create_info.queueFamilyIndex;
    auto queue_count = create_info.queueCount;
    ASR_ENSURE(command_queue_family_index <
               phy_device.handle->family_properties.size());

    for (u32 queue_index_in_family = 0; queue_index_in_family < queue_count;
         queue_index_in_family++) {
      f32 priority = create_info.pQueuePriorities[i];
      VkQueue command_queue = vk::get_command_queue(
          device, command_queue_family_index, queue_index_in_family);

      command_queues
          .push(CommandQueueInfo{
              .queue = command_queue,
              .create_index = static_cast<u32>(i),
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
    u32 command_queue_create_index) {
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

struct Buffer {
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkBuffer buffer = VK_NULL_HANDLE;
  stx::Rc<Device*> device;

  Buffer(VkDeviceMemory amemory, VkBuffer abuffer, stx::Rc<Device*> adevice)
      : memory{amemory}, buffer{abuffer}, device{std::move(adevice)} {}

  STX_DISABLE_COPY(Buffer)
  STX_DISABLE_MOVE(Buffer)

  ~Buffer() {
    vkFreeMemory(device.handle->device, memory, nullptr);
    vkDestroyBuffer(device.handle->device, buffer, nullptr);
  }
};

std::pair<VkBuffer, VkDeviceMemory> create_buffer_with_memory(
    VkDevice dev, CommandQueueFamilyInfo const& graphics_command_queue,
    VkPhysicalDeviceMemoryProperties const& memory_properties,
    usize size_bytes) {
  u32 queue_families[] = {graphics_command_queue.index};

  VkBufferCreateInfo create_info{
      .flags = 0,
      .pNext = nullptr,
      .pQueueFamilyIndices = queue_families,
      .queueFamilyIndexCount = 1,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .size = size_bytes,
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  };

  VkBuffer buffer;

  ASR_VK_CHECK(vkCreateBuffer(dev, &create_info, nullptr, &buffer));

  VkDeviceMemory memory;

  VkMemoryRequirements requirements;

  vkGetBufferMemoryRequirements(dev, buffer, &requirements);

  u32 memory_type_index =
      vk::find_suitable_memory_type(requirements, memory_properties,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
          .unwrap();

  VkMemoryAllocateInfo alloc_info{
      .allocationSize = requirements.size,
      .memoryTypeIndex = memory_type_index,
      .pNext = nullptr,
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

  ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

  ASR_VK_CHECK(vkBindBufferMemory(dev, buffer, memory, 0));

  return std::make_pair(buffer, memory);
}

stx::Rc<Buffer*> upload_vertices(
    stx::Rc<Device*> const& device,
    CommandQueueFamilyInfo const& graphics_command_queue,
    VkPhysicalDeviceMemoryProperties const& memory_properties,
    stx::Span<vec3 const> vertices) {
  VkDevice dev = device.handle->device;

  auto [buffer, memory] = create_buffer_with_memory(
      dev, graphics_command_queue, memory_properties, vertices.size_bytes());

  void* memory_map;
  ASR_VK_CHECK(vkMapMemory(dev, memory, 0, VK_WHOLE_SIZE, 0, &memory_map));

  memcpy(memory_map, vertices.data(), vertices.size_bytes());

  VkMappedMemoryRange range{.memory = memory,
                            .offset = 0,
                            .pNext = nullptr,
                            .size = VK_WHOLE_SIZE,
                            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};

  ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));

  vkUnmapMemory(dev, memory);

  return stx::rc::make_inplace<Buffer>(stx::os_allocator, memory, buffer,
                                       device.share())
      .unwrap();
}

stx::Rc<Buffer*> upload_indices(
    stx::Rc<Device*> const& device,
    CommandQueueFamilyInfo const& graphics_command_queue,
    VkPhysicalDeviceMemoryProperties const& memory_properties,
    stx::Span<u32 const> indices) {
  VkDevice dev = device.handle->device;

  auto [buffer, memory] = create_buffer_with_memory(
      dev, graphics_command_queue, memory_properties, indices.size_bytes());

  void* memory_map;
  ASR_VK_CHECK(vkMapMemory(dev, memory, 0, VK_WHOLE_SIZE, 0, &memory_map));

  memcpy(memory_map, indices.data(), indices.size_bytes());

  VkMappedMemoryRange range{.memory = memory,
                            .offset = 0,
                            .pNext = nullptr,
                            .size = VK_WHOLE_SIZE,
                            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};

  ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));

  vkUnmapMemory(dev, memory);

  return stx::rc::make_inplace<Buffer>(stx::os_allocator, memory, buffer,
                                       device.share())
      .unwrap();
}

struct Image {
  VkImage image = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  stx::Rc<CommandQueue*> queue;

  Image(VkImage aimage, VkImageView aview, VkDeviceMemory amemory,
        stx::Rc<CommandQueue*> aqueue)
      : image{aimage},
        view{aview},
        memory{amemory},
        queue{std::move(aqueue)} {};

  STX_DISABLE_COPY(Image)
  STX_DISABLE_MOVE(Image)

  ~Image() {
    VkDevice dev = queue.handle->device.handle->device;
    vkFreeMemory(dev, memory, nullptr);
    vkDestroyImageView(dev, view, nullptr);
    vkDestroyImage(dev, image, nullptr);
  }
};

// R | G | B | A
stx::Rc<Image*> upload_rgba_image(stx::Rc<CommandQueue*> const& queue,
                                  u32 width, u32 height,
                                  stx::Span<u32 const> data) {
  ASR_ENSURE(data.size_bytes() == width * height * 4);

  VkDevice dev = queue.handle->device.handle->device;

  VkImageCreateInfo create_info{
      .arrayLayers = 1,
      .extent = VkExtent3D{.depth = 1, .height = height, .width = width},
      .flags = 0,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .imageType = VK_IMAGE_TYPE_2D,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .mipLevels = 1,
      .pNext = nullptr,
      .pQueueFamilyIndices = &queue.handle->info.family.index,
      .queueFamilyIndexCount = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_SAMPLED_BIT};

  VkImage image;

  ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

  VkMemoryRequirements requirements;

  vkGetImageMemoryRequirements(dev, image, &requirements);

  u32 memory_type_index =
      vk::find_suitable_memory_type(
          requirements,
          queue.handle->device.handle->phy_device.handle->memory_properties,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
          .unwrap();

  VkMemoryAllocateInfo alloc_info{
      .allocationSize = requirements.size,
      .memoryTypeIndex = memory_type_index,
      .pNext = nullptr,
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

  VkDeviceMemory memory;

  ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

  ASR_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

  void* memory_map;

  ASR_VK_CHECK(vkMapMemory(dev, memory, 0, VK_WHOLE_SIZE, 0, &memory_map));

  memcpy(memory_map, data.data(), data.size_bytes());

  VkMappedMemoryRange range{.memory = memory,
                            .offset = 0,
                            .pNext = nullptr,
                            .size = VK_WHOLE_SIZE,
                            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};

  ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));

  vkUnmapMemory(dev, memory);

  VkImageViewCreateInfo view_create_info{
      .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
      .flags = 0,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .image = image,
      .pNext = nullptr,
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseArrayLayer = 0,
                                  .baseMipLevel = 0,
                                  .layerCount = 1,
                                  .levelCount = 1},
      .viewType = VK_IMAGE_VIEW_TYPE_2D};

  VkImageView view;

  ASR_ENSURE(vkCreateImageView(dev, &view_create_info, nullptr, &view));

  return stx::rc::make_inplace<Image>(stx::os_allocator, image, view, memory,
                                      queue.share())
      .unwrap();
}

struct ImageSampler {
  VkSampler sampler = VK_NULL_HANDLE;
  stx::Rc<Image*> image;

  ImageSampler(VkSampler asampler, stx::Rc<Image*> aimage)
      : sampler{asampler}, image{std::move(aimage)} {}

  STX_DISABLE_COPY(ImageSampler)
  STX_DISABLE_MOVE(ImageSampler)

  ~ImageSampler() {
    vkDestroySampler(image.handle->queue.handle->device.handle->device, sampler,
                     nullptr);
  }
};

stx::Rc<ImageSampler*> create_sampler(stx::Rc<Image*> const& image) {
  VkSamplerCreateInfo create_info{
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .anisotropyEnable = VK_TRUE,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .flags = 0,
      .magFilter = VK_FILTER_LINEAR,
      .maxAnisotropy = image.handle->queue.handle->device.handle->phy_device
                           .handle->properties.limits.maxSamplerAnisotropy,
      .maxLod = 0.0f,
      .minFilter = VK_FILTER_LINEAR,
      .minLod = 0.0f,
      .mipLodBias = 0.0f,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .pNext = nullptr,
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .unnormalizedCoordinates = VK_FALSE};

  VkSampler sampler;
  ASR_VK_CHECK(
      vkCreateSampler(image.handle->queue.handle->device.handle->device,
                      &create_info, nullptr, &sampler));

  return stx::rc::make_inplace<ImageSampler>(stx::os_allocator, sampler,
                                             image.share())
      .unwrap();
}

enum class DescriptorType : u8 { Buffer, Sampler };

struct DescriptorBinding {
  DescriptorType type = DescriptorType::Buffer;
  // only valid if type is DescriptorType::Buffer
  usize size = 0;
  void const* data = nullptr;

  template <typename BufferType>
  static DescriptorBinding uniform(BufferType const* value) {
    return DescriptorBinding{.type = DescriptorType::Buffer,
                             .size = sizeof(BufferType),
                             .data = value};
  }

  static DescriptorBinding sampler(ImageSampler const* sampler) {
    return DescriptorBinding{
        .type = DescriptorType::Sampler, .size = 0, .data = sampler};
  }
};

struct DescriptorSet {
  stx::Vec<DescriptorBinding> bindings{stx::os_allocator};

  explicit DescriptorSet(std::initializer_list<DescriptorBinding> abindings) {
    for (auto const& binding : abindings) {
      bindings.push_inplace(binding).unwrap();
    }
  }
};

inline std::pair<stx::Vec<VkDescriptorSetLayout>, stx::Vec<VkDescriptorSet>>
prepare_descriptor_sets(VkDevice dev, VkDescriptorPool descriptor_pool,
                        VkShaderStageFlagBits shader_stage,
                        stx::Span<DescriptorSet const> sets) {
  stx::Vec<VkDescriptorSetLayout> layouts{stx::os_allocator};

  for (DescriptorSet const& set : sets) {
    stx::Vec<VkDescriptorSetLayoutBinding> bindings{stx::os_allocator};
    u32 binding_index = 0;

    for (DescriptorBinding const& binding : set.bindings) {
      VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

      switch (binding.type) {
        case DescriptorType::Buffer:
          type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
          break;
        case DescriptorType::Sampler:
          type = VK_DESCRIPTOR_TYPE_SAMPLER;
          break;
        default:
          ASR_ENSURE(false);
      }

      bindings
          .push(VkDescriptorSetLayoutBinding{
              .binding = binding_index,
              .descriptorCount = 1,
              .descriptorType = type,
              .pImmutableSamplers = nullptr,
              .stageFlags = (VkShaderStageFlags)shader_stage})
          .unwrap();

      binding_index++;
    }

    VkDescriptorSetLayoutCreateInfo layout_create_info{
        .bindingCount = static_cast<u32>(bindings.size()),
        .flags = 0,
        .pBindings = bindings.data(),
        .pNext = nullptr,
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

    VkDescriptorSetLayout layout;

    ASR_VK_CHECK(vkCreateDescriptorSetLayout(dev, &layout_create_info, nullptr,
                                             &layout));

    layouts.push_inplace(layout).unwrap();
  }

  stx::Vec<VkDescriptorSet> descriptor_sets{stx::os_allocator};

  VkDescriptorSetAllocateInfo descriptor_set_alloc_info{
      .descriptorPool = descriptor_pool,
      .descriptorSetCount = static_cast<u32>(layouts.size()),
      .pNext = nullptr,
      .pSetLayouts = layouts.data(),
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};

  descriptor_sets.reserve(layouts.size()).unwrap();

  ASR_VK_CHECK(vkAllocateDescriptorSets(dev, &descriptor_set_alloc_info,
                                        descriptor_sets.data()));

  return std::make_pair(std::move(layouts), std::move(descriptor_sets));
}

struct DescriptorSetsSpec {
  stx::Span<DescriptorSet> vertex_shader;
  stx::Span<DescriptorSet> fragment_shader;
};

// NOTE: descriptor binding values lifetime must be longer than the
// ShaderProgram's
struct ShaderProgram {
  STX_DISABLE_COPY(ShaderProgram)
  STX_DISABLE_MOVE(ShaderProgram)

  VkShaderModule vertex_shader = VK_NULL_HANDLE;
  VkShaderModule fragment_shader = VK_NULL_HANDLE;

  stx::Vec<VkDescriptorSetLayout> vertex_shader_descriptor_set_layouts{
      stx::os_allocator};
  stx::Vec<VkDescriptorSetLayout> fragment_shader_descriptor_set_layouts{
      stx::os_allocator};
  stx::Vec<VkDescriptorSet> vertex_shader_descriptor_sets{stx::os_allocator};
  stx::Vec<VkDescriptorSet> fragment_shader_descriptor_sets{stx::os_allocator};

  VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

  stx::Vec<std::pair<VkBuffer, VkDeviceMemory>> uniform_buffers{
      stx::os_allocator};

  stx::Vec<std::tuple<VkDeviceMemory, DescriptorBinding>>
      uniform_buffer_memory_mappings{stx::os_allocator};

  stx::Rc<CommandQueue*> queue;

  stx::Vec<DescriptorSet> vertex_shader_descriptor_set_spec{stx::os_allocator};
  stx::Vec<DescriptorSet> fragment_shader_descriptor_set_spec{
      stx::os_allocator};

  ShaderProgram(stx::Rc<CommandQueue*> const& aqueue,
                stx::Span<u32 const> fragment_shader_code,
                stx::Span<u32 const> vertex_shader_code,
                DescriptorSetsSpec spec)
      : queue{aqueue.share()} {
    VkDevice dev = queue.handle->device.handle->device;

    {
      VkShaderModuleCreateInfo create_info{
          .codeSize = vertex_shader_code.size_bytes(),
          .flags = 0,
          .pCode = vertex_shader_code.data(),
          .pNext = nullptr,
          .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};

      ASR_VK_CHECK(
          vkCreateShaderModule(dev, &create_info, nullptr, &vertex_shader));
    }

    {
      VkShaderModuleCreateInfo create_info{
          .codeSize = fragment_shader_code.size_bytes(),
          .flags = 0,
          .pCode = fragment_shader_code.data(),
          .pNext = nullptr,
          .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};

      ASR_VK_CHECK(
          vkCreateShaderModule(dev, &create_info, nullptr, &fragment_shader));
    }

    VkDescriptorPoolSize pool_sizes[] = {
        {.descriptorCount = 20, .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
        {.descriptorCount = 10, .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE}};

    VkDescriptorPoolCreateInfo pool_create_info{
        .flags = 0,
        .maxSets = 10,
        .pNext = nullptr,
        .poolSizeCount = std::size(pool_sizes),
        .pPoolSizes = pool_sizes,
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};

    ASR_VK_CHECK(vkCreateDescriptorPool(dev, &pool_create_info, nullptr,
                                        &descriptor_pool));

    ____spec_descriptor_sets(spec);
    ____prepare_descriptor_sets();
  }

  ~ShaderProgram() {
    VkDevice dev = queue.handle->device.handle->device;

    vkDestroyShaderModule(dev, vertex_shader, nullptr);
    vkDestroyShaderModule(dev, fragment_shader, nullptr);

    for (VkDescriptorSetLayout layout : vertex_shader_descriptor_set_layouts) {
      vkDestroyDescriptorSetLayout(dev, layout, nullptr);
    }

    for (VkDescriptorSetLayout layout :
         fragment_shader_descriptor_set_layouts) {
      vkDestroyDescriptorSetLayout(dev, layout, nullptr);
    }

    ASR_VK_CHECK(vkFreeDescriptorSets(dev, descriptor_pool,
                                      vertex_shader_descriptor_sets.size(),
                                      vertex_shader_descriptor_sets.data()));
    ASR_VK_CHECK(vkFreeDescriptorSets(dev, descriptor_pool,
                                      fragment_shader_descriptor_sets.size(),
                                      fragment_shader_descriptor_sets.data()));

    vkDestroyDescriptorPool(dev, descriptor_pool, nullptr);

    for (auto [buffer, memory] : uniform_buffers) {
      vkFreeMemory(dev, memory, nullptr);
      vkDestroyBuffer(dev, buffer, nullptr);
    }
  }

  void ____spec_descriptor_sets(DescriptorSetsSpec spec) {
    for (DescriptorSet& set : spec.vertex_shader)
      vertex_shader_descriptor_set_spec.push_inplace(std::move(set)).unwrap();

    for (DescriptorSet& set : spec.fragment_shader)
      fragment_shader_descriptor_set_spec.push_inplace(std::move(set)).unwrap();
  }

  void ____prepare_descriptor_sets() {
    VkDevice dev = queue.handle->device.handle->device;

    auto [vert_descset_layouts, vert_descsets] = prepare_descriptor_sets(
        dev, descriptor_pool, VK_SHADER_STAGE_VERTEX_BIT,
        vertex_shader_descriptor_set_spec);

    vertex_shader_descriptor_set_layouts = std::move(vert_descset_layouts);
    vertex_shader_descriptor_sets = std::move(vert_descsets);

    auto [frag_descset_layouts, frag_descsets] = prepare_descriptor_sets(
        dev, descriptor_pool, VK_SHADER_STAGE_FRAGMENT_BIT,
        fragment_shader_descriptor_set_spec);

    fragment_shader_descriptor_set_layouts = std::move(frag_descset_layouts);
    fragment_shader_descriptor_sets = std::move(frag_descsets);

    auto pass = [&](stx::Span<DescriptorSet const> specs,
                    stx::Span<VkDescriptorSet const> descriptor_sets) {
      u32 iset = 0;
      for (DescriptorSet const& set : specs) {
        u32 ibinding = 0;
        for (DescriptorBinding const& binding : set.bindings) {
          if (binding.type == DescriptorType::Buffer) {
            auto [buffer, memory] = create_buffer_with_memory(
                dev, queue.handle->info.family,
                queue.handle->device.handle->phy_device.handle
                    ->memory_properties,
                binding.size);

            uniform_buffers.push_inplace(buffer, memory).unwrap();

            uniform_buffer_memory_mappings.push_inplace(memory, binding)
                .unwrap();

            VkDescriptorBufferInfo buffer_info{
                .buffer = buffer, .offset = 0, .range = VK_WHOLE_SIZE};

            VkWriteDescriptorSet writes[] = {
                {.descriptorCount = 1,
                 .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                 .dstArrayElement = 0,
                 .dstBinding = ibinding,
                 .dstSet = descriptor_sets[iset],
                 .pBufferInfo = &buffer_info,
                 .pImageInfo = nullptr,
                 .pNext = nullptr,
                 .pTexelBufferView = nullptr,
                 .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET}};

            vkUpdateDescriptorSets(dev, 1, writes, 0, nullptr);

          } else if (binding.type == DescriptorType::Sampler) {
            ImageSampler const* sampler =
                static_cast<ImageSampler const*>(binding.data);

            VkDescriptorImageInfo image_info{
                .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .imageView = sampler->image.handle->view,
                .sampler = sampler->sampler};

            VkWriteDescriptorSet writes[] = {
                {.descriptorCount = 1,
                 .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                 .dstArrayElement = 0,
                 .dstBinding = ibinding,
                 .dstSet = descriptor_sets[iset],
                 .pBufferInfo = nullptr,
                 .pImageInfo = &image_info,
                 .pNext = nullptr,
                 .pTexelBufferView = nullptr,
                 .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET}};

            vkUpdateDescriptorSets(dev, 1, writes, 0, nullptr);
          }

          ibinding++;
        }

        iset++;
      }
    };

    pass(vertex_shader_descriptor_set_spec, vertex_shader_descriptor_sets);
    pass(fragment_shader_descriptor_set_spec, fragment_shader_descriptor_sets);
  }

  void flush_uniform_buffers() {
    VkDevice dev = queue.handle->device.handle->device;

    for (auto [memory, binding] : uniform_buffer_memory_mappings) {
      switch (binding.type) {
        case DescriptorType::Buffer: {
          void* memory_map;
          ASR_VK_CHECK(
              vkMapMemory(dev, memory, 0, VK_WHOLE_SIZE, 0, &memory_map));

          memcpy(memory_map, binding.data, binding.size);

          VkMappedMemoryRange range{
              .memory = memory,
              .offset = 0,
              .pNext = nullptr,
              .size = VK_WHOLE_SIZE,
              .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};

          ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));

          vkUnmapMemory(dev, memory);
        } break;
        default:
          ASR_ENSURE(false);
      }
    }
  }
};

struct Pipeline {
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkRenderPass render_pass = VK_NULL_HANDLE;
  stx::Rc<ShaderProgram*> program;

  Pipeline(stx::Rc<ShaderProgram*> aprogram,
           stx::Span<VkVertexInputAttributeDescription const> vertex_input_attr,
           usize vertex_input_size, VkFormat swapchain_format,
           VkSampleCountFlags sample_count)
      : program{std::move(aprogram)} {
    VkDevice dev = program.handle->queue.handle->device.handle->device;

    VkPipelineShaderStageCreateInfo vert_shader_stage{
        .flags = 0,
        .module = program.handle->vertex_shader,
        .pName = "main",
        .pNext = nullptr,
        .pSpecializationInfo = nullptr,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};

    VkPipelineShaderStageCreateInfo frag_shader_stage{
        .flags = 0,
        .module = program.handle->fragment_shader,
        .pName = "main",
        .pNext = nullptr,
        .pSpecializationInfo = nullptr,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};

    VkPipelineShaderStageCreateInfo stages[] = {vert_shader_stage,
                                                frag_shader_stage};

    VkPipelineLayoutCreateInfo layout_create_info{
        .flags = 0,
        .pNext = nullptr,
        .pPushConstantRanges = nullptr,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .setLayoutCount = 0,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    VkPipelineLayout layout;

    ASR_VK_CHECK(
        vkCreatePipelineLayout(dev, &layout_create_info, nullptr, &layout));

    VkPipelineColorBlendAttachmentState color_blend_attachment_states[]{
        {.alphaBlendOp = VK_BLEND_OP_ADD,
         .blendEnable = VK_TRUE,
         .colorBlendOp = VK_BLEND_OP_ADD,
         .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
         .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
         .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
         .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
         .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA}};

    VkPipelineColorBlendStateCreateInfo color_blend_state{
        .attachmentCount = 1,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
        .flags = 0,
        .logicOp = VK_LOGIC_OP_COPY,
        .logicOpEnable = VK_FALSE,
        .pAttachments = color_blend_attachment_states,
        .pNext = nullptr,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
        .back = VkStencilOpState{.compareMask = 0,
                                 .compareOp = VK_COMPARE_OP_NEVER,
                                 .depthFailOp = VK_STENCIL_OP_KEEP,
                                 .failOp = VK_STENCIL_OP_KEEP,
                                 .passOp = VK_STENCIL_OP_KEEP,
                                 .reference = 0,
                                 .writeMask = 0},
        .depthBoundsTestEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .flags = 0,
        .front = VkStencilOpState{.compareMask = 0,
                                  .compareOp = VK_COMPARE_OP_NEVER,
                                  .depthFailOp = VK_STENCIL_OP_KEEP,
                                  .failOp = VK_STENCIL_OP_KEEP,
                                  .passOp = VK_STENCIL_OP_KEEP,
                                  .reference = 0,
                                  .writeMask = 0},
        .maxDepthBounds = 1.0f,
        .minDepthBounds = 0.0f,
        .pNext = nullptr,
        .stencilTestEnable = VK_FALSE,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
        .flags = 0,
        .pNext = nullptr,
        .primitiveRestartEnable = VK_FALSE,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

    VkPipelineMultisampleStateCreateInfo multisample_state{
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
        .flags = 0,
        .minSampleShading = 1.0f,
        .pNext = nullptr,
        .pSampleMask = nullptr,
        .rasterizationSamples = (VkSampleCountFlagBits)sample_count,
        .sampleShadingEnable = VK_FALSE,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

    VkPipelineRasterizationStateCreateInfo rasterization_state{
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .depthBiasClamp = 0.0f,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasEnable = VK_FALSE,
        .depthBiasSlopeFactor = 0.0f,
        .depthClampEnable = VK_FALSE,
        .flags = 0,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f,
        .pNext = nullptr,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .rasterizerDiscardEnable = VK_FALSE,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

    /*
        VkPipelineTessellationStateCreateInfo tesselation_state{
            .flags,
            .patchControlPoints,
            .pNext = nullptr,
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
    */

    VkVertexInputBindingDescription vertex_binding_descriptions[] = {
        {.binding = 0,
         .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
         .stride = vertex_input_size}};

    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .flags = 0,
        .pNext = nullptr,
        .pVertexAttributeDescriptions = vertex_input_attr.data(),
        .pVertexBindingDescriptions = vertex_binding_descriptions,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexAttributeDescriptionCount = vertex_input_attr.size(),
        .vertexBindingDescriptionCount =
            std::size(vertex_binding_descriptions)};

    VkViewport viewport{.height = 1080,
                        .maxDepth = 1.0f,
                        .minDepth = 0.0f,
                        .width = 1920,
                        .x = 0.0f,
                        .y = 0.0f};

    VkRect2D scissor{.extent = VkExtent2D{.height = 1080, .width = 1920},
                     .offset = VkOffset2D{.x = 0, .y = 0}};

    VkPipelineViewportStateCreateInfo viewport_state{
        .flags = 0,
        .pNext = nullptr,
        .pScissors = &scissor,
        .pViewports = &viewport,
        .scissorCount = 1,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1};

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_SCISSOR,
                                       VK_DYNAMIC_STATE_VIEWPORT};

    VkPipelineDynamicStateCreateInfo dynamic_state{
        .dynamicStateCount = std::size(dynamic_states),
        .flags = 0,
        .pDynamicStates = dynamic_states,
        .pNext = nullptr,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};

    VkAttachmentDescription color_attachment{
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .flags = 0,
        .format = swapchain_format,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .samples = (VkSampleCountFlagBits)sample_count,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE};

    VkAttachmentDescription depth_attachment{
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .flags = 0,
        .format = findDepthFormat(),
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .samples = (VkSampleCountFlagBits)sample_count,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE};

    VkAttachmentDescription color_attachment_resolve{
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .flags = 0,
        .format = swapchain_format,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE};

    VkAttachmentDescription attachments[] = {color_attachment, depth_attachment,
                                             color_attachment_resolve};

    VkAttachmentReference color_attachment_reference{
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depth_attachment_reference{
        .attachment = 1, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference color_attachment_resolve_reference{
        .attachment = 2, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{
        .colorAttachmentCount = 1,
        .flags = 0,
        .inputAttachmentCount = 0,
        .pColorAttachments = &color_attachment_reference,
        .pDepthStencilAttachment = &depth_attachment_reference,
        .pInputAttachments = nullptr,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pPreserveAttachments = nullptr,
        .preserveAttachmentCount = 0,
        .pResolveAttachments = &color_attachment_resolve_reference};

    VkSubpassDependency dependency{
        .dependencyFlags = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstSubpass = 0,
        .srcAccessMask = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcSubpass = VK_SUBPASS_EXTERNAL};

    VkRenderPassCreateInfo render_pass_create_info{
        .attachmentCount = std::size(attachments),
        .dependencyCount = 1,
        .flags = 0,
        .pAttachments = attachments,
        .pDependencies = &dependency,
        .pNext = nullptr,
        .pSubpasses = &subpass,
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .subpassCount = 1};

    ASR_VK_CHECK(vkCreateRenderPass(dev, &render_pass_create_info, nullptr,
                                    &render_pass));

    VkGraphicsPipelineCreateInfo create_info{
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
        .flags = 0,
        .layout = layout,
        .pColorBlendState = &color_blend_state,
        .pDepthStencilState = &depth_stencil_state,
        .pDynamicState = &dynamic_state,
        .pInputAssemblyState = &input_assembly_state,
        .pMultisampleState = &multisample_state,
        .pNext = nullptr,
        .pRasterizationState = &rasterization_state,
        .pStages = stages,
        .pTessellationState = nullptr,
        .pVertexInputState = &vertex_input_state,
        .pViewportState = &viewport_state,
        .renderPass = render_pass,
        .stageCount = std::size(stages),
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .subpass = 0};

    ASR_VK_CHECK(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &create_info,
                                           nullptr, &pipeline));
  }

  // void bind() {
  // vkCmdSetViewport
  // vkCmdSetScissor
  // }

  ~Pipeline() {
    vkDestroyPipeline(program.handle->queue.handle->device.handle->device,
                      pipeline, nullptr);
    vkDestroyRenderPass(program.handle->queue.handle->device.handle->device,
                        render_pass, nullptr);
  }
};

inline VkSampleCountFlagBits get_max_sample_count(PhyDeviceInfo const& device) {
  VkSampleCountFlags counts =
      device.properties.limits.framebufferColorSampleCounts &
      device.properties.limits.framebufferDepthSampleCounts;

  if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
  if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
  if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
  if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
  if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
  if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

  return VK_SAMPLE_COUNT_1_BIT;
}

inline void create_msaa_color_resource(stx::Rc<CommandQueue*> const& queue,
                                       VkFormat swapchain_format,
                                       VkExtent2D swapchain_extent,
                                       VkSampleCountFlags sample_count) {
  VkDevice dev = queue.handle->device.handle->device;

  VkImageCreateInfo create_info{
      .arrayLayers = 1,
      .extent = VkExtent3D{.depth = 1,
                           .height = swapchain_extent.height,
                           .width = swapchain_extent.width},
      .flags = 0,
      .format = swapchain_format,
      .imageType = VK_IMAGE_TYPE_2D,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .mipLevels = 1,
      .pNext = nullptr,
      .pQueueFamilyIndices = &queue.handle->info.family.index,
      .queueFamilyIndexCount = 1,
      .samples = (VkSampleCountFlagBits)sample_count,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};

  VkImage image;

  ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

  VkMemoryRequirements requirements;

  vkGetImageMemoryRequirements(dev, image, &requirements);

  u32 memory_type_index =
      vk::find_suitable_memory_type(
          requirements,
          queue.handle->device.handle->phy_device.handle->memory_properties,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
          .unwrap();

  VkMemoryAllocateInfo alloc_info{
      .allocationSize = requirements.size,
      .memoryTypeIndex = memory_type_index,
      .pNext = nullptr,
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

  VkDeviceMemory memory;

  ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

  ASR_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

  VkImageViewCreateInfo view_create_info{
      .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
      .flags = 0,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .image = image,
      .pNext = nullptr,
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseArrayLayer = 0,
                                  .baseMipLevel = 0,
                                  .layerCount = 1,
                                  .levelCount = 1},
      .viewType = VK_IMAGE_VIEW_TYPE_2D};

  VkImageView view;

  ASR_ENSURE(vkCreateImageView(dev, &view_create_info, nullptr, &view));
}

inline void create_msaa_depth_resource(stx::Rc<CommandQueue*> const& queue,
                                       VkFormat swapchain_format,
                                       VkExtent2D swapchain_extent,
                                       VkSampleCountFlags sample_count) {
  VkDevice dev = queue.handle->device.handle->device;

  VkImageCreateInfo create_info{
      .arrayLayers = 1,
      .extent = VkExtent3D{.depth = 1,
                           .height = swapchain_extent.height,
                           .width = swapchain_extent.width},
      .flags = 0,
      .format = swapchain_format,
      .imageType = VK_IMAGE_TYPE_2D,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .mipLevels = 1,
      .pNext = nullptr,
      .pQueueFamilyIndices = &queue.handle->info.family.index,
      .queueFamilyIndexCount = 1,
      .samples = (VkSampleCountFlagBits)sample_count,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};

  VkImage image;

  ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

  VkMemoryRequirements requirements;

  vkGetImageMemoryRequirements(dev, image, &requirements);

  u32 memory_type_index =
      vk::find_suitable_memory_type(
          requirements,
          queue.handle->device.handle->phy_device.handle->memory_properties,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
          .unwrap();

  VkMemoryAllocateInfo alloc_info{
      .allocationSize = requirements.size,
      .memoryTypeIndex = memory_type_index,
      .pNext = nullptr,
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

  VkDeviceMemory memory;

  ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

  ASR_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

  VkImageViewCreateInfo view_create_info{
      .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
      .flags = 0,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .image = image,
      .pNext = nullptr,
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseArrayLayer = 0,
                                  .baseMipLevel = 0,
                                  .layerCount = 1,
                                  .levelCount = 1},
      .viewType = VK_IMAGE_VIEW_TYPE_2D};

  VkImageView view;

  ASR_ENSURE(vkCreateImageView(dev, &view_create_info, nullptr, &view));
}

}  // namespace vkh
}  // namespace asr

#if STX_CFG(COMPILER, CLANG)
#pragma clang diagnostic pop
#endif