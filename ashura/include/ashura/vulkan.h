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
      ASR_LOG_WARN("Required validation layer `{}` is not available",
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
      ASR_LOG_WARN("Required extension `{}` is not available",
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
    ASR_LOG_WARN_IF(!is_general, "[Validation Layer Message] {}",
                    callback_data->pMessage);
  } else {
    ASR_LOG_IF(is_general, "[Validation Layer Message, Hints=\"{}\"] {}", hint,
               callback_data->pMessage);
    ASR_LOG_WARN_IF(!is_general, "[Validation Layer Message, Hints=\"{}\"] {}",
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
    VkDevice device, VkSurfaceKHR surface, VkExtent2D preferred_extent,
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
      select_swapchain_extent(properties.capabilities, preferred_extent);

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

  VkSampleCountFlagBits get_max_sample_count() const {
    VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts &
                                properties.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
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
    stx::Span<char const* const> required_extensions,
    stx::Span<char const* const> required_validation_layers,
    VkPhysicalDeviceFeatures required_features) {
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

  STX_MAKE_PINNED(Buffer)

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

  STX_MAKE_PINNED(Image)

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

  ASR_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

  return stx::rc::make_inplace<Image>(stx::os_allocator, image, view, memory,
                                      queue.share())
      .unwrap();
}

struct ImageSampler {
  VkSampler sampler = VK_NULL_HANDLE;
  stx::Rc<Image*> image;

  ImageSampler(VkSampler asampler, stx::Rc<Image*> aimage)
      : sampler{asampler}, image{std::move(aimage)} {}

  STX_MAKE_PINNED(ImageSampler)

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
          ASR_UNREACHABLE();
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
  STX_MAKE_PINNED(ShaderProgram)

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
          ASR_UNREACHABLE();
      }
    }
  }
};

inline std::tuple<VkImage, VkImageView, VkDeviceMemory>
create_msaa_color_resource(stx::Rc<CommandQueue*> const& queue,
                           VkFormat swapchain_format,
                           VkExtent2D swapchain_extent,
                           VkSampleCountFlagBits sample_count) {
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
      .samples = sample_count,
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

  ASR_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

  return std::make_tuple(image, view, memory);
}

inline std::tuple<VkImage, VkImageView, VkDeviceMemory>
create_msaa_depth_resource(stx::Rc<CommandQueue*> const& queue,
                           VkFormat swapchain_format,
                           VkExtent2D swapchain_extent,
                           VkSampleCountFlagBits sample_count) {
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
      .samples = sample_count,
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

  ASR_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

  return std::make_tuple(image, view, memory);
}

// choose a specific swapchain format available on the surface
inline VkSurfaceFormatKHR select_swapchain_surface_formats(
    stx::Span<VkSurfaceFormatKHR const> formats,
    stx::Span<VkSurfaceFormatKHR const> preferred_formats) {
  ASR_ENSURE(!formats.is_empty(),
             "No window surface format supported by physical device");

  for (VkSurfaceFormatKHR preferred_format : preferred_formats) {
    if (!formats
             .which([&](VkSurfaceFormatKHR format) {
               return preferred_format.colorSpace == format.colorSpace &&
                      preferred_format.format == format.format;
             })
             .is_empty())
      return preferred_format;
  }

  ASR_PANIC("Unable to find any of the preferred swapchain surface formats");
}

inline VkPresentModeKHR select_swapchain_presentation_mode(
    stx::Span<VkPresentModeKHR const> available_presentation_modes,
    stx::Span<VkPresentModeKHR const> preferred_present_modes) noexcept {
  /// - VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application
  /// are transferred to the screen right away, which may result in tearing.
  ///
  /// - VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the
  /// display takes an image from the front of the queue when the display is
  /// refreshed and the program inserts rendered images at the back of the
  /// queue. If the queue is full then the program has to wait. This is most
  /// similar to vertical sync as found in modern games. The moment that the
  /// display is refreshed is known as "vertical blank" (v-sync).
  ///
  /// - VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs
  /// from the previous one if the application is late and the queue was
  /// empty at the last vertical blank. Instead of waiting for the next
  /// vertical blank, the image is transferred right away when it finally
  /// arrives. This may result in visible tearing.
  ///
  /// - VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the
  /// second mode. Instead of blocking the application when the queue is
  /// full, the images that are already queued are simply replaced with the
  /// newer ones. This mode can be used to implement triple buffering, which
  /// allows you to avoid tearing with significantly less latency issues
  /// than standard vertical sync that uses double buffering.

  ASR_ENSURE(!available_presentation_modes.is_empty(),
             "No surface presentation mode available");

  for (auto const& preferred_present_mode : preferred_present_modes) {
    if (!available_presentation_modes.find(preferred_present_mode).is_empty())
      return preferred_present_mode;
  }

  ASR_PANIC("Unable to find any of the preferred presentation modes");
}

/// Swapchains handle the presentation and update logic of the images to the
/// window surface.
///
///
///
/// NOTE: all arguments to create a swapchain for a window surface are
/// preferences, meaning another available argument will be used if the
/// suggested ones are not supported. Thus do not assume your arguments are
/// final.
///
///
/// swapchains can not be headless, nor exist independently of the surface they
/// originated from, its lifetime thus depends on the surface. the surface can
/// and should be able to destroy and create it at will (which would be
/// impossible to do correctly with ref-counting, since we are not holding a
/// reference to the surface) we thus can't hold a reference to the swapchain,
/// its images, nor its image views outside itself (the swapchain object).
///
struct SwapChain {
  // actually holds the images of the surface and used to present to the render
  // target image. when resizing is needed, the swapchain is destroyed and
  // recreated with the desired extents.
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  VkSurfaceFormatKHR format{.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
                            .format = VK_FORMAT_R8G8B8A8_SRGB};
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  VkExtent2D extent{.height = 0, .width = 0};

  /// IMPORTANT: this is different from the image index obtained via
  /// `vkAcquireNextImageKHR`. this index is used for referencing semaphores
  /// used for submitting and querying rendering operations. this value is
  /// always increasing and wrapping, unlike the index obtained from
  /// `vkAcquireNextImageKHR` which depends on the presentation mode being used
  /// (determines how the images are used, in what order and whether they
  /// repeat).
  u32 frame_flight_index = 0;

  // the images in the swapchain
  stx::Vec<VkImage> images{stx::os_allocator};

  // the image views pointing to a part of a whole texture (images in the
  // swapchain)
  stx::Vec<VkImageView> image_views{stx::os_allocator};

  // the rendering semaphores correspond to the frame indexes and not the
  // swapchain images
  stx::Vec<VkSemaphore> rendering_semaphores{stx::os_allocator};

  stx::Vec<VkSemaphore> image_acquisition_semaphores{stx::os_allocator};

  stx::Vec<VkFramebuffer> frame_buffers{stx::os_allocator};

  VkImage msaa_color_image = VK_NULL_HANDLE;
  VkImageView msaa_color_image_view = VK_NULL_HANDLE;
  VkDeviceMemory msaa_color_image_memory = VK_NULL_HANDLE;

  VkImage msaa_depth_image = VK_NULL_HANDLE;
  VkImageView msaa_depth_image_view = VK_NULL_HANDLE;
  VkDeviceMemory msaa_depth_image_memory = VK_NULL_HANDLE;

  VkRenderPass render_pass = VK_NULL_HANDLE;

  stx::Rc<CommandQueue*> queue;

  SwapChain(stx::Rc<CommandQueue*> aqueue, VkSurfaceKHR target_surface,
            stx::Span<VkSurfaceFormatKHR const> preferred_formats,
            stx::Span<VkPresentModeKHR const> preferred_present_modes,
            VkExtent2D preferred_extent,
            VkCompositeAlphaFlagBitsKHR alpha_compositing)
      : queue{std::move(aqueue)} {
    VkPhysicalDevice phy_device =
        queue.handle->device.handle->phy_device.handle->phy_device;
    VkDevice dev = queue.handle->device.handle->device;

    // the properties change every time we need to create a swapchain so we must
    // query for this every time
    vk::SwapChainProperties properties =
        vk::get_swapchain_properties(phy_device, target_surface);

    ASR_LOG("Device Supported Surface Formats:");
    for (VkSurfaceFormatKHR const& format : properties.supported_formats) {
      ASR_LOG("\tFormat: {}, Color Space: {}", vk::format(format.format),
              vk::format(format.colorSpace));
    }

    // swapchain formats are device-dependent
    VkSurfaceFormatKHR selected_format = select_swapchain_surface_formats(
        properties.supported_formats, preferred_formats);

    // TODO(lamarrr): log selections
    // swapchain presentation modes are device-dependent
    VkPresentModeKHR selected_present_mode = select_swapchain_presentation_mode(
        properties.presentation_modes, preferred_present_modes);

    u32 accessing_families[] = {queue.handle->info.family.index};

    auto [new_swapchain, new_extent] = vk::create_swapchain(
        dev, target_surface, preferred_extent, selected_format,
        selected_present_mode, properties,
        // not thread-safe since GPUs typically have one graphics queue
        VK_SHARING_MODE_EXCLUSIVE, accessing_families,
        // render target image
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        alpha_compositing,
        // we don't care about the color of pixels that are obscured, for
        // example because another window is in front of them. Unless you really
        // need to be able to read these pixels back and get predictable
        // results, you'll get the best performance by enabling clipping.
        true);

    swapchain = new_swapchain;
    format = selected_format;
    present_mode = selected_present_mode;
    extent = new_extent;

    for (VkImage image : images) {
      VkImageView image_view = vk::create_image_view(
          dev, image, format.format, VK_IMAGE_VIEW_TYPE_2D,
          VK_IMAGE_ASPECT_COLOR_BIT,
          VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                             .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                             .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                             .a = VK_COMPONENT_SWIZZLE_IDENTITY});
      image_views.push_inplace(image_view).unwrap();
    }

    for (usize i = 0; i < images.size(); i++) {
      rendering_semaphores.push(vk::create_semaphore(dev)).unwrap();
      image_acquisition_semaphores.push(vk::create_semaphore(dev)).unwrap();
    }

    VkSampleCountFlagBits msaa_sample_count =
        queue.handle->device.handle->phy_device.handle->get_max_sample_count();

    auto [xmsaa_color_image, xmsaa_color_image_view, xmsaa_color_image_memory] =
        create_msaa_color_resource(queue, format.format, new_extent,
                                   msaa_sample_count);

    // TODO(lamarrr): depth format is incorrect
    auto [xmsaa_depth_image, xmsaa_depth_image_view, xmsaa_depth_image_memory] =
        create_msaa_depth_resource(queue, format.format, new_extent,
                                   msaa_sample_count);

    msaa_color_image = xmsaa_color_image;
    msaa_color_image_view = xmsaa_color_image_view;
    msaa_color_image_memory = xmsaa_color_image_memory;

    msaa_depth_image = xmsaa_depth_image;
    msaa_depth_image_view = xmsaa_depth_image_view;
    msaa_depth_image_memory = xmsaa_depth_image_memory;

    // TODO(lamarrrr)
    VkSampleCountFlagBits sample_count = 0;

    VkAttachmentDescription color_attachment{
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .flags = 0,
        .format = format.format,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .samples = sample_count,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE};

    VkAttachmentDescription depth_attachment{
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .flags = 0,
        .format = find_depth_format(
            queue.handle->device.handle->phy_device.handle->phy_device),
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .samples = sample_count,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE};

    VkAttachmentDescription color_attachment_resolve{
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .flags = 0,
        .format = format.format,
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

    for (usize i = 0; i < images.size(); i++) {
      VkFramebuffer framebuffer;

      VkImageView attachments[] = {msaa_color_image_view, msaa_depth_image_view,
                                   image_views[i]};

      VkFramebufferCreateInfo create_info{
          .attachmentCount = std::size(attachments),
          .flags = 0,
          .height = extent.height,
          .layers = 1,
          .pAttachments = attachments,
          .pNext = nullptr,
          .renderPass = render_pass,
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .width = extent.width};

      ASR_VK_CHECK(vkCreateFramebuffer(queue.handle->device.handle->device,
                                       &create_info, nullptr, &framebuffer));

      frame_buffers.push_inplace(framebuffer).unwrap();
    }
  }

  STX_MAKE_PINNED(SwapChain)

  ~SwapChain() {
    VkDevice dev = queue.handle->device.handle->device;

    // await idleness of the semaphores device, so we can destroy the
    // semaphore and images whislt not in use.
    // any part of the device could be using the semaphore

    ASR_MUST_SUCCEED(vkDeviceWaitIdle(dev), "Unable to await device idleness");

    vkDestroyRenderPass(dev, render_pass, nullptr);

    vkFreeMemory(dev, msaa_color_image_memory, nullptr);
    vkDestroyImageView(dev, msaa_color_image_view, nullptr);
    vkDestroyImage(dev, msaa_color_image, nullptr);

    vkFreeMemory(dev, msaa_depth_image_memory, nullptr);
    vkDestroyImageView(dev, msaa_depth_image_view, nullptr);
    vkDestroyImage(dev, msaa_depth_image, nullptr);

    for (VkFramebuffer framebuffer : frame_buffers) {
      vkDestroyFramebuffer(dev, framebuffer, nullptr);
    }

    for (VkSemaphore semaphore : rendering_semaphores) {
      vkDestroySemaphore(dev, semaphore, nullptr);
    }

    for (VkSemaphore semaphore : image_acquisition_semaphores) {
      vkDestroySemaphore(dev, semaphore, nullptr);
    }

    for (VkImageView image_view : image_views) {
      vkDestroyImageView(dev, image_view, nullptr);
    }

    // swapchain image is automatically deleted along with the swapchain
    vkDestroySwapchainKHR(dev, swapchain, nullptr);
  }
};

struct Pipeline;

struct Surface {
  // only a pointer to metadata, does not contain data itself, resilient to
  // resizing
  VkSurfaceKHR surface = VK_NULL_HANDLE;

  // empty and invalid until change_swapchain is called.
  // not ref-counted since it solely belongs to this surface and the surface can
  // create and destroy it upon request.
  //
  // also, we need to be certain it is non-existent and not referring to any
  // resources when destroyed, not just by calling a method to destroy its
  // resources.
  //
  stx::Option<stx::Unique<SwapChain*>> swapchain;

  stx::Rc<Instance*> instance;

  Surface(VkSurfaceKHR asurface,
          stx::Option<stx::Unique<SwapChain*>> aswapchain,
          stx::Rc<Instance*> ainstance)
      : surface{asurface},
        swapchain{std::move(aswapchain)},
        instance{std::move(ainstance)} {}

  STX_MAKE_PINNED(Surface)

  ~Surface() {
    // we need to ensure the swapchain is destroyed before the surface (if not
    // already destroyed)
    swapchain = stx::None;

    vkDestroySurfaceKHR(instance.handle->instance, surface, nullptr);
  }

  void change_swapchain(
      stx::Rc<CommandQueue*> const& queue,
      stx::Span<VkSurfaceFormatKHR const> preferred_formats,
      stx::Span<VkPresentModeKHR const> preferred_present_modes,
      VkExtent2D preferred_extent,
      VkCompositeAlphaFlagBitsKHR alpha_compositing) {
    swapchain = stx::None;  // probably don't want to have two existing at once

    swapchain = stx::Some(stx::rc::make_unique_inplace<SwapChain>(
                              stx::os_allocator, queue.share(), surface,
                              preferred_formats, preferred_present_modes,
                              preferred_extent, alpha_compositing)
                              .unwrap());
  }

  void add_framebuffers(stx::Rc<Pipeline*> const& pipeline);
};

inline VkFormat find_supported_format(VkPhysicalDevice phy_device,
                                      stx::Span<VkFormat const> candidates,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(phy_device, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  ASR_UNREACHABLE();
}

inline VkFormat find_depth_format(VkPhysicalDevice phy_device) {
  VkFormat formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                        VK_FORMAT_D24_UNORM_S8_UINT};

  return find_supported_format(phy_device, formats, VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

// void bind() {
// vkCmdSetViewport
// vkCmdSetScissor
// }
struct Pipeline {
  VkPipeline pipeline = VK_NULL_HANDLE;
  stx::Rc<ShaderProgram*> program;

  Pipeline(stx::Rc<ShaderProgram*> aprogram,
           stx::Span<VkVertexInputAttributeDescription const> vertex_input_attr,
           usize vertex_input_size, VkFormat swapchain_format,
           VkSampleCountFlagBits sample_count)
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
        .rasterizationSamples = sample_count,
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
         .stride = static_cast<u32>(vertex_input_size)}};

    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .flags = 0,
        .pNext = nullptr,
        .pVertexAttributeDescriptions = vertex_input_attr.data(),
        .pVertexBindingDescriptions = vertex_binding_descriptions,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexAttributeDescriptionCount =
            static_cast<u32>(vertex_input_attr.size()),
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

  STX_MAKE_PINNED(Pipeline)

  ~Pipeline() {
    vkDestroyPipeline(program.handle->queue.handle->device.handle->device,
                      pipeline, nullptr);
  }
};

void Surface::add_framebuffers(stx::Rc<Pipeline*> const& pipeline) {
  // TODO(lamarrr): store pipeline!!!
  // TODO(lamarrr): this will mean the swapchain can only be used with one
  // pipeline as we will be binding to the renderpass
  //
  // each pipeline will have a framebuffer, we will need to maintain this in the
  // surface?
  //
  //
  ASR_ENSURE(swapchain.is_some(),
             "surface swapchain must be present before adding framebuffers");

  auto& swapchain_r = *swapchain.value().handle;

  ASR_ENSURE(swapchain_r.frame_buffers.is_empty(),
             "attempted to add framebuffers whilst some are already present");

  for (usize i = 0; i < swapchain_r.images.size(); i++) {
    VkFramebuffer framebuffer;

    VkImageView attachments[] = {swapchain_r.msaa_color_image_view,
                                 swapchain_r.msaa_depth_image_view,
                                 swapchain_r.image_views[i]};

    VkFramebufferCreateInfo create_info{
        .attachmentCount = std::size(attachments),
        .flags = 0,
        .height = swapchain_r.extent.height,
        .layers = 1,
        .pAttachments = attachments,
        .pNext = nullptr,
        .renderPass = pipeline.handle->render_pass,
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .width = swapchain_r.extent.width};

    ASR_VK_CHECK(vkCreateFramebuffer(
        pipeline.handle->program.handle->queue.handle->device.handle->device,
        &create_info, nullptr, &framebuffer));

    swapchain_r.frame_buffers.push_inplace(framebuffer).unwrap();
  }
}

}  // namespace vk
}  // namespace asr

#if STX_CFG(COMPILER, CLANG)
#pragma clang diagnostic pop
#endif