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

#define ASR_VK_CHECK(...)                              \
  do {                                                 \
    VkResult operation_result = (__VA_ARGS__);         \
    ASR_ENSURE(operation_result == VK_SUCCESS,         \
               "Vulkan Operation: (" #__VA_ARGS__      \
               ")  failed! (VK_SUCCESS not returned)", \
               operation_result);                      \
  } while (false)

namespace asr {

using namespace std::chrono_literals;

namespace vk {

template <typename T>
inline auto join_copy(stx::Span<T const> a, stx::Span<T const> b) {
  stx::Vec<T> x{stx::os_allocator};
  x.extend(a).unwrap();
  x.extend(b).unwrap();
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
      .pNext = nullptr,
      .flags = 0,
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

  auto createDebugUtilsMessengerEXT =
      reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

  ASR_ENSURE(
      createDebugUtilsMessengerEXT != nullptr,
      "Unable to get process address for vkCreateDebugUtilsMessengerEXT");

  ASR_VK_CHECK(createDebugUtilsMessengerEXT(instance, &create_info, nullptr,
                                            &debug_messenger));

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
  VkApplicationInfo app_info{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = application_name,
      .applicationVersion = application_version,
      .pEngineName = engine_name,
      .engineVersion = engine_version,
      .apiVersion = VK_API_VERSION_1_1,
  };

  VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &app_info,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = 0,
      .ppEnabledExtensionNames = nullptr};

  static constexpr char const* DEBUG_EXTENSIONS[] = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

  // debug message callback extension
  auto extensions =
      join_copy(required_extensions,
                required_validation_layers.is_empty()
                    ? stx::Span<char const* const>{}
                    : stx::Span<char const* const>(DEBUG_EXTENSIONS));

  create_info.enabledExtensionCount = AS_U32(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();

  ensure_extensions_supported(extensions);

  if (!required_validation_layers.is_empty()) {
    // validation layers
    ensure_validation_layers_supported(required_validation_layers);
    create_info.enabledLayerCount = AS_U32(required_validation_layers.size());
    create_info.ppEnabledLayerNames = required_validation_layers.data();

    // debug messenger for when the installed debug messenger is uninstalled.
    // this helps to debug issues with vkDestroyInstance and vkCreateInstance
    // i.e. (before and after the debug messenger is installed)
    create_info.pNext = &debug_messenger_create_info;
  }

  VkInstance vulkan_instance;
  ASR_VK_CHECK(vkCreateInstance(&create_info, nullptr, &vulkan_instance));

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

  for (u32 i = 0; i < AS_U32(queue_families.size()); i++) {
    VkBool32 surface_presentation_supported;
    ASR_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(
        phy_device, i, surface, &surface_presentation_supported));
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
  ASR_VK_CHECK(vkEnumerateDeviceExtensionProperties(
      phy_device, nullptr, &available_extensions_count, nullptr));

  // device specific extensions
  stx::Vec<VkExtensionProperties> available_device_extensions{
      stx::os_allocator};

  available_device_extensions.resize(available_extensions_count).unwrap();

  ASR_VK_CHECK(vkEnumerateDeviceExtensionProperties(
      phy_device, nullptr, &available_extensions_count,
      available_device_extensions.data()));

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
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueCreateInfoCount = AS_U32(command_queue_create_infos.size()),
      .pQueueCreateInfos = command_queue_create_infos.data(),
      .enabledLayerCount = AS_U32(required_validation_layers.size()),
      .ppEnabledLayerNames = required_validation_layers.data(),
      .enabledExtensionCount = AS_U32(required_extensions.size()),
      .ppEnabledExtensionNames = required_extensions.data(),
      .pEnabledFeatures = &required_features};

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

  ASR_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      phy_device, surface, &details.capabilities));

  u32 supported_surface_formats_count = 0;

  ASR_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
      phy_device, surface, &supported_surface_formats_count, nullptr));

  details.supported_formats.resize(supported_surface_formats_count).unwrap();

  ASR_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
      phy_device, surface, &supported_surface_formats_count,
      details.supported_formats.data()));

  u32 surface_presentation_modes_count;
  ASR_VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
      phy_device, surface, &surface_presentation_modes_count, nullptr));

  details.presentation_modes.resize(surface_presentation_modes_count).unwrap();
  ASR_VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
      phy_device, surface, &surface_presentation_modes_count,
      details.presentation_modes.data()));

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
    VkExtent2D target_extent{desired_extent.width, desired_extent.height};

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
    u32 accessing_queue_family_index, VkImageUsageFlags image_usages,
    VkCompositeAlphaFlagBitsKHR alpha_channel_blending, VkBool32 clipped) {
  u32 desired_num_buffers = std::min(properties.capabilities.minImageCount + 1,
                                     properties.capabilities.maxImageCount);

  VkExtent2D selected_extent =
      select_swapchain_extent(properties.capabilities, preferred_extent);

  VkSwapchainCreateInfoKHR create_info{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .surface = surface,
      // number of images to use for buffering on the swapchain
      .minImageCount = select_swapchain_image_count(properties.capabilities,
                                                    desired_num_buffers),
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent = selected_extent,
      .imageArrayLayers = 1,  // 2 for stereoscopic rendering
      .imageUsage = image_usages,
      // under normal circumstances command queues on the same queue family can
      // access data without data race issues
      //
      // VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a
      // time and ownership must be explicitly transferred before using it in
      // another queue family. This option offers the best performance.
      // VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue
      // families without explicit ownership transfers.
      .imageSharingMode = accessing_queue_families_sharing_mode,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices = &accessing_queue_family_index,
      .preTransform = properties.capabilities.currentTransform,
      .compositeAlpha =
          alpha_channel_blending,  // how the alpha channel should be
                                   // used for blending with other
                                   // windows in the window system
      .presentMode = present_mode,
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
  };

  VkSwapchainKHR swapchain;
  ASR_VK_CHECK(vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain));

  return std::make_pair(swapchain, selected_extent);
}

inline stx::Vec<VkImage> get_swapchain_images(VkDevice device,
                                              VkSwapchainKHR swapchain) {
  u32 image_count;

  ASR_VK_CHECK(
      vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr));

  stx::Vec<VkImage> swapchain_images{stx::os_allocator};
  swapchain_images.resize(image_count).unwrap();

  ASR_VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &image_count,
                                       swapchain_images.data()));

  return swapchain_images;
}

// GPU-GPU synchronization primitive, cheap
inline VkSemaphore create_semaphore(VkDevice device) {
  VkSemaphoreCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0};

  VkSemaphore semaphore;
  ASR_VK_CHECK(vkCreateSemaphore(device, &create_info, nullptr, &semaphore));

  return semaphore;
}

// GPU-CPU synchronization primitive, expensive
inline VkFence create_fence(VkDevice device, VkFenceCreateFlags make_signaled) {
  VkFenceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                .pNext = nullptr,
                                .flags = make_signaled};

  VkFence fence;

  ASR_VK_CHECK(vkCreateFence(device, &create_info, nullptr, &fence));

  return fence;
}

inline void reset_fences(VkDevice device, stx::Span<VkFence const> fences) {
  ASR_VK_CHECK(vkResetFences(device, AS_U32(fences.size()), fences.data()));
}

inline void await_fences(VkDevice device, stx::Span<VkFence const> fences) {
  ASR_VK_CHECK(vkWaitForFences(
      device, AS_U32(fences.size()), fences.data(), VK_TRUE,
      std::chrono::duration_cast<std::chrono::nanoseconds>(1min).count()));
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
      .pNext = nullptr,
      .waitSemaphoreCount = AS_U32(await_semaphores.size()),
      .pWaitSemaphores = await_semaphores.data(),
      .swapchainCount = AS_U32(swapchains.size()),
      .pSwapchains = swapchains.data(),
      .pImageIndices = swapchain_image_indexes.data(),
      .pResults = nullptr};

  auto result = vkQueuePresentKHR(command_queue, &present_info);
  ASR_ENSURE(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
                 result == VK_ERROR_OUT_OF_DATE_KHR,
             "Unable to present to swapchain");

  return result;
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
      destroy_debug_messenger(instance, debug_messenger.value());
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

    nfamily_properties.extend(family_properties).unwrap();

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

  ASR_VK_CHECK(vkEnumeratePhysicalDevices(instance.handle->instance,
                                          &devices_count, nullptr));

  ASR_ENSURE(devices_count != 0, "No Physical Device Found");

  stx::Vec<VkPhysicalDevice> phy_devices{stx::os_allocator};

  phy_devices.resize(devices_count).unwrap();

  ASR_VK_CHECK(vkEnumeratePhysicalDevices(instance.handle->instance,
                                          &devices_count, phy_devices.data()));

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
                            .family_properties = get_queue_families(device),
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

// automatically destroyed once the device is destroyed
struct CommandQueueFamilyInfo {
  u32 index = 0;
  stx::Rc<PhyDeviceInfo*> phy_device;
};

struct CommandQueueInfo {
  // automatically destroyed once the device is destroyed
  VkQueue queue = VK_NULL_HANDLE;
  u32 create_index = 0;
  f32 priority = 0.0f;
  CommandQueueFamilyInfo family;
};

struct Device;

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
  auto [instance, messenger] =
      create_vulkan_instance(required_extensions, validation_layers,
                             make_debug_messenger_create_info(), app_name,
                             app_version, engine_name, engine_version);

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
      .index = AS_U32(pos - phy_device.handle->family_properties.begin()),
      .phy_device = phy_device.share()});
}

inline stx::Rc<Device*> create_device(
    stx::Rc<PhyDeviceInfo*> const& phy_device,
    stx::Span<VkDeviceQueueCreateInfo const> command_queue_create_info,
    stx::Span<char const* const> required_extensions,
    stx::Span<char const* const> required_validation_layers,
    VkPhysicalDeviceFeatures required_features) {
  VkDevice device = create_logical_device(
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
      VkQueue command_queue = get_command_queue(
          device, command_queue_family_index, queue_index_in_family);

      command_queues
          .push(CommandQueueInfo{
              .queue = command_queue,
              .create_index = AS_U32(i),
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

  auto const& queue = queue_s[0];

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
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  usize size = 0;
  void* memory_map = nullptr;

  void destroy(VkDevice dev) {
    vkFreeMemory(dev, memory, nullptr);
    vkDestroyBuffer(dev, buffer, nullptr);
  }

  void write(VkDevice dev, void const* data) {
    std::memcpy(memory_map, data, size);

    VkMappedMemoryRange range{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr,
        .memory = memory,
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };

    ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));
  }
};

struct SpanBuffer {
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  usize size = 0;
  usize memory_size = 0;
  void* memory_map = nullptr;

  void destroy(VkDevice dev) {
    vkFreeMemory(dev, memory, nullptr);
    vkDestroyBuffer(dev, buffer, nullptr);
  }

  template <typename T>
  void write(VkDevice dev, u32 family_index,
             VkPhysicalDeviceMemoryProperties const& memory_properties,
             VkBufferUsageFlagBits usage, stx::Span<T const> span) {
    if (span.size_bytes() != size) {
      vkDestroyBuffer(dev, buffer, nullptr);

      VkBufferCreateInfo create_info{
          .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .size = span.size_bytes(),
          .usage = usage,
          .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
          .queueFamilyIndexCount = 1,
          .pQueueFamilyIndices = &family_index};

      ASR_VK_CHECK(vkCreateBuffer(dev, &create_info, nullptr, &buffer));

      size = span.size_bytes();

      VkMemoryRequirements memory_requirements;

      vkGetBufferMemoryRequirements(dev, buffer, &memory_requirements);

      if (memory_requirements.size <= memory_size) {
        ASR_VK_CHECK(vkBindBufferMemory(dev, buffer, memory, 0));
      } else {
        u32 memory_type_index = vk::find_suitable_memory_type(
                                    memory_requirements, memory_properties,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                                    .unwrap();

        VkMemoryAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_type_index};

        ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

        memory_size = memory_requirements.size;

        ASR_VK_CHECK(vkBindBufferMemory(dev, buffer, memory, 0));

        ASR_VK_CHECK(vkMapMemory(dev, memory, 0, memory_size, 0, &memory_map));
      }
    }

    memcpy(memory_map, span.data(), span.size_bytes());

    VkMappedMemoryRange range{.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                              .pNext = nullptr,
                              .memory = memory,
                              .offset = 0,
                              .size = VK_WHOLE_SIZE};

    ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));
  }
};

inline Buffer create_buffer(
    VkDevice dev, CommandQueueFamilyInfo const& graphics_command_queue,
    VkPhysicalDeviceMemoryProperties const& memory_properties, usize size_bytes,
    VkBufferUsageFlags usage) {
  u32 queue_families[] = {graphics_command_queue.index};

  VkBufferCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size = size_bytes,
                                 .usage = usage,
                                 .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                 .queueFamilyIndexCount = 1,
                                 .pQueueFamilyIndices = queue_families};

  VkBuffer buffer;

  ASR_VK_CHECK(vkCreateBuffer(dev, &create_info, nullptr, &buffer));

  VkDeviceMemory memory;

  VkMemoryRequirements memory_requirements;

  vkGetBufferMemoryRequirements(dev, buffer, &memory_requirements);

  u32 memory_type_index =
      find_suitable_memory_type(memory_requirements, memory_properties,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
          .unwrap();

  VkMemoryAllocateInfo alloc_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = memory_type_index};

  ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

  ASR_VK_CHECK(vkBindBufferMemory(dev, buffer, memory, 0));

  void* memory_map;

  ASR_VK_CHECK(vkMapMemory(dev, memory, 0, VK_WHOLE_SIZE, 0, &memory_map));

  return Buffer{.buffer = buffer,
                .memory = memory,
                .size = size_bytes,
                .memory_map = memory_map};
}

struct Image {
  VkImage image = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;

  void destroy(VkDevice dev) {
    vkFreeMemory(dev, memory, nullptr);
    vkDestroyImageView(dev, view, nullptr);
    vkDestroyImage(dev, image, nullptr);
  }
};

struct ImageX {
  VkImage image = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  stx::Rc<CommandQueue*> queue;

  ImageX(VkImage aimage, VkImageView aview, VkDeviceMemory amemory,
         stx::Rc<CommandQueue*> aqueue)
      : image{aimage},
        view{aview},
        memory{amemory},
        queue{std::move(aqueue)} {};

  STX_MAKE_PINNED(ImageX)

  ~ImageX() {
    VkDevice dev = queue.handle->device.handle->device;
    vkFreeMemory(dev, memory, nullptr);
    vkDestroyImageView(dev, view, nullptr);
    vkDestroyImage(dev, image, nullptr);
  }
};

// R | G | B | A
inline stx::Rc<ImageX*> upload_rgba_image(stx::Rc<CommandQueue*> const& queue,
                                          u32 width, u32 height,
                                          stx::Span<u32 const> data) {
  ASR_ENSURE(data.size_bytes() == width * height * 4);

  VkDevice dev = queue.handle->device.handle->device;

  VkImageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .extent = VkExtent3D{.width = width, .height = height, .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices = &queue.handle->info.family.index,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

  VkImage image;

  ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

  VkMemoryRequirements memory_requirements;

  vkGetImageMemoryRequirements(dev, image, &memory_requirements);

  u32 memory_type_index =
      find_suitable_memory_type(
          memory_requirements,
          queue.handle->device.handle->phy_device.handle->memory_properties,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
          .unwrap();

  VkMemoryAllocateInfo alloc_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = memory_type_index};

  VkDeviceMemory memory;

  ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

  ASR_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

  void* memory_map;

  ASR_VK_CHECK(vkMapMemory(dev, memory, 0, VK_WHOLE_SIZE, 0, &memory_map));

  memcpy(memory_map, data.data(), data.size_bytes());

  VkMappedMemoryRange range{.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                            .pNext = nullptr,
                            .memory = memory,
                            .offset = 0,
                            .size = VK_WHOLE_SIZE};

  ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));

  vkUnmapMemory(dev, memory);

  VkImageViewCreateInfo view_create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseMipLevel = 0,
                                  .levelCount = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1}};

  VkImageView view;

  ASR_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

  return stx::rc::make_inplace<ImageX>(stx::os_allocator, image, view, memory,
                                       queue.share())
      .unwrap();
}

// R only
inline std::tuple<VkImage, VkDeviceMemory, VkImageView> create_bitmap_image(
    stx::Rc<CommandQueue*> const& queue, u32 width, u32 height) {
  VkDevice dev = queue.handle->device.handle->device;

  VkImageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R32_SFLOAT,
      .extent = VkExtent3D{.width = width, .height = height, .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices = &queue.handle->info.family.index,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

  VkImage image;

  ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

  VkMemoryRequirements memory_requirements;

  vkGetImageMemoryRequirements(dev, image, &memory_requirements);

  u32 memory_type_index =
      find_suitable_memory_type(
          memory_requirements,
          queue.handle->device.handle->phy_device.handle->memory_properties,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
          .unwrap();

  VkMemoryAllocateInfo alloc_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = memory_type_index};

  VkDeviceMemory memory;

  ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

  ASR_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

  VkImageViewCreateInfo view_create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R32_SFLOAT,
      .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .g = VK_COMPONENT_SWIZZLE_ZERO,
                                       .b = VK_COMPONENT_SWIZZLE_ZERO,
                                       .a = VK_COMPONENT_SWIZZLE_ZERO},
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseMipLevel = 0,
                                  .levelCount = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1}};

  VkImageView view;

  ASR_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

  return std::make_tuple(image, memory, view);
}

struct ImageSampler {
  VkSampler sampler = VK_NULL_HANDLE;
  stx::Rc<ImageX*> image;

  ImageSampler(VkSampler asampler, stx::Rc<ImageX*> aimage)
      : sampler{asampler}, image{std::move(aimage)} {}

  STX_MAKE_PINNED(ImageSampler)

  ~ImageSampler() {
    vkDestroySampler(image.handle->queue.handle->device.handle->device, sampler,
                     nullptr);
  }
};

inline VkSampler create_sampler(stx::Rc<Device*> const& device,
                                VkBool32 enable_anisotropy) {
  VkSamplerCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias = 0.0f,
      .anisotropyEnable = enable_anisotropy,
      .maxAnisotropy = device.handle->phy_device.handle->properties.limits
                           .maxSamplerAnisotropy,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE};

  VkSampler sampler;

  ASR_VK_CHECK(
      vkCreateSampler(device.handle->device, &create_info, nullptr, &sampler));

  return sampler;
}

inline stx::Rc<ImageSampler*> create_image_sampler(
    stx::Rc<ImageX*> const& image) {
  return stx::rc::make_inplace<ImageSampler>(
             stx::os_allocator,
             create_sampler(image.handle->queue.handle->device, VK_TRUE),
             image.share())
      .unwrap();
}

enum class DescriptorType : u8 { Buffer, Sampler };

struct DescriptorBinding {
  DescriptorType type = DescriptorType::Buffer;

  // only valid if type is DescriptorType::Buffer
  VkBuffer buffer = VK_NULL_HANDLE;

  // only valid if type is DescriptorType::Sampler
  VkImageView view = VK_NULL_HANDLE;

  // only valid if type is DescriptorType::Sampler
  VkSampler sampler = VK_NULL_HANDLE;

  static constexpr DescriptorBinding make_buffer(VkBuffer buff) {
    return DescriptorBinding{.type = DescriptorType::Buffer, .buffer = buff};
  }

  static constexpr DescriptorBinding make_sampler(VkImageView view,
                                                  VkSampler sampler) {
    return DescriptorBinding{
        .type = DescriptorType::Sampler, .view = view, .sampler = sampler};
  }
};

struct DescriptorSet {
  stx::Vec<DescriptorType> bindings{stx::os_allocator};

  explicit DescriptorSet(std::initializer_list<DescriptorType> abindings) {
    bindings.extend(abindings).unwrap();
  }
};

inline std::pair<stx::Vec<VkDescriptorSetLayout>, stx::Vec<VkDescriptorSet>>
prepare_descriptor_sets(VkDevice dev, VkDescriptorPool descriptor_pool,
                        VkShaderStageFlags shader_stage,
                        stx::Span<DescriptorSet const> sets) {
  stx::Vec<VkDescriptorSetLayout> layouts{stx::os_allocator};

  for (DescriptorSet const& set : sets) {
    stx::Vec<VkDescriptorSetLayoutBinding> bindings{stx::os_allocator};
    u32 binding_index = 0;

    for (DescriptorType type : set.bindings) {
      VkDescriptorType vktype = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

      switch (type) {
        case DescriptorType::Buffer:
          vktype = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
          break;
        case DescriptorType::Sampler:
          vktype = VK_DESCRIPTOR_TYPE_SAMPLER;
          break;
        default:
          ASR_UNREACHABLE();
      }

      bindings
          .push(VkDescriptorSetLayoutBinding{.binding = binding_index,
                                             .descriptorType = vktype,
                                             .descriptorCount = 1,
                                             .stageFlags = shader_stage,
                                             .pImmutableSamplers = nullptr})
          .unwrap();

      binding_index++;
    }

    VkDescriptorSetLayoutCreateInfo layout_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = AS_U32(bindings.size()),
        .pBindings = bindings.data(),
    };

    VkDescriptorSetLayout layout;

    ASR_VK_CHECK(vkCreateDescriptorSetLayout(dev, &layout_create_info, nullptr,
                                             &layout));

    layouts.push_inplace(layout).unwrap();
  }

  stx::Vec<VkDescriptorSet> descriptor_sets{stx::os_allocator};

  VkDescriptorSetAllocateInfo descriptor_set_alloc_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = descriptor_pool,
      .descriptorSetCount = AS_U32(layouts.size()),
      .pSetLayouts = layouts.data(),
  };

  descriptor_sets.reserve(layouts.size()).unwrap();

  ASR_VK_CHECK(vkAllocateDescriptorSets(dev, &descriptor_set_alloc_info,
                                        descriptor_sets.data()));

  return std::make_pair(std::move(layouts), std::move(descriptor_sets));
}

struct DescriptorSets {
  VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

  stx::Vec<VkDescriptorSetLayout> descriptor_set_layouts{stx::os_allocator};
  stx::Vec<VkDescriptorSet> descriptor_sets{stx::os_allocator};

  stx::Rc<Device*> device;

  stx::Vec<DescriptorSet> descriptor_set_spec{stx::os_allocator};

  DescriptorSets(stx::Rc<Device*> adevice, stx::Span<DescriptorSet> spec)
      : device{std::move(adevice)} {
    VkDescriptorPoolSize pool_sizes[] = {
        {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 20},
        {.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 10}};

    VkDescriptorPoolCreateInfo pool_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = 10,
        .poolSizeCount = AS_U32(std::size(pool_sizes)),
        .pPoolSizes = pool_sizes,
    };

    ASR_VK_CHECK(vkCreateDescriptorPool(
        device.handle->device, &pool_create_info, nullptr, &descriptor_pool));

    descriptor_set_spec.extend_move(spec).unwrap();

    auto [descset_layouts, descsets] = prepare_descriptor_sets(
        device.handle->device, descriptor_pool, VK_SHADER_STAGE_VERTEX_BIT,
        descriptor_set_spec);

    descriptor_set_layouts = std::move(descset_layouts);
    descriptor_sets = std::move(descsets);
  }

  STX_MAKE_PINNED(DescriptorSets)

  ~DescriptorSets() {
    VkDevice dev = device.handle->device;

    for (VkDescriptorSetLayout layout : descriptor_set_layouts) {
      vkDestroyDescriptorSetLayout(dev, layout, nullptr);
    }

    ASR_VK_CHECK(vkFreeDescriptorSets(dev, descriptor_pool,
                                      AS_U32(descriptor_sets.size()),
                                      descriptor_sets.data()));

    vkDestroyDescriptorPool(dev, descriptor_pool, nullptr);
  }

  void write(stx::Span<stx::Span<DescriptorBinding const> const> sets) {
    VkDevice dev = device.handle->device;

    ASR_ENSURE(sets.size() == descriptor_set_spec.size());
    ASR_ENSURE(sets.size() == descriptor_sets.size());

    for (u32 iset = 0; iset < AS_U32(sets.size()); iset++) {
      stx::Span<DescriptorBinding const> set = sets[iset];

      ASR_ENSURE(set.size() == descriptor_set_spec[iset].bindings.size());

      for (u32 ibinding = 0; ibinding < AS_U32(sets[iset].size()); ibinding++) {
        DescriptorBinding binding = sets[iset][ibinding];

        ASR_ENSURE(binding.type ==
                   descriptor_set_spec[iset].bindings[ibinding]);

        switch (binding.type) {
          case DescriptorType::Sampler: {
            VkDescriptorImageInfo image_info{
                .sampler = binding.sampler,
                .imageView = binding.view,
                .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED};

            VkWriteDescriptorSet writes[] = {{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptor_sets[iset],
                .dstBinding = ibinding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                .pImageInfo = &image_info,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr,
            }};

            vkUpdateDescriptorSets(dev, AS_U32(std::size(writes)), writes, 0,
                                   nullptr);

          } break;

          case DescriptorType::Buffer: {
            VkDescriptorBufferInfo buffer_info{
                .buffer = binding.buffer, .offset = 0, .range = VK_WHOLE_SIZE};

            VkWriteDescriptorSet writes[] = {{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptor_sets[iset],
                .dstBinding = ibinding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = nullptr,
                .pBufferInfo = &buffer_info,
                .pTexelBufferView = nullptr,
            }};

            vkUpdateDescriptorSets(dev, AS_U32(std::size(writes)), writes, 0,
                                   nullptr);
          } break;

          default: {
            ASR_UNREACHABLE();
          }
        }
      }
    }
  }
};

inline Image create_msaa_color_resource(stx::Rc<CommandQueue*> const& queue,
                                        VkFormat swapchain_format,
                                        VkExtent2D swapchain_extent,
                                        VkSampleCountFlagBits sample_count) {
  VkDevice dev = queue.handle->device.handle->device;

  VkImageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = swapchain_format,
      .extent = VkExtent3D{.width = swapchain_extent.width,
                           .height = swapchain_extent.height,
                           .depth = 1

      },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = sample_count,
      .tiling = VK_IMAGE_TILING_OPTIMAL,

      .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices = &queue.handle->info.family.index,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VkImage image;

  ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

  VkMemoryRequirements memory_requirements;

  vkGetImageMemoryRequirements(dev, image, &memory_requirements);

  u32 memory_type_index =
      find_suitable_memory_type(
          memory_requirements,
          queue.handle->device.handle->phy_device.handle->memory_properties,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
          .unwrap();

  VkMemoryAllocateInfo alloc_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = memory_type_index};

  VkDeviceMemory memory;

  ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

  ASR_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

  VkImageViewCreateInfo view_create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseMipLevel = 0,
                                  .levelCount = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1}};

  VkImageView view;

  ASR_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

  return Image{.image = image, .view = view, .memory = memory};
}

inline Image create_msaa_depth_resource(stx::Rc<CommandQueue*> const& queue,
                                        VkFormat swapchain_format,
                                        VkExtent2D swapchain_extent,
                                        VkSampleCountFlagBits sample_count) {
  VkDevice dev = queue.handle->device.handle->device;

  VkImageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = swapchain_format,
      .extent = VkExtent3D{.width = swapchain_extent.width,
                           .height = swapchain_extent.height,
                           .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = sample_count,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices = &queue.handle->info.family.index,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

  VkImage image;

  ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

  VkMemoryRequirements memory_requirements;

  vkGetImageMemoryRequirements(dev, image, &memory_requirements);

  u32 memory_type_index =
      find_suitable_memory_type(
          memory_requirements,
          queue.handle->device.handle->phy_device.handle->memory_properties,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
          .unwrap();

  VkMemoryAllocateInfo alloc_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = memory_type_index};

  VkDeviceMemory memory;

  ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

  ASR_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

  VkImageViewCreateInfo view_create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseMipLevel = 0,
                                  .levelCount = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1}};

  VkImageView view;

  ASR_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

  return Image{.image = image, .view = view, .memory = memory};
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

/// Swapchains handle the presentation and update logic of the images to the
/// window surface.
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
  struct Clip {
    Image image;
    VkSampler sampler = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;

    void destroy(VkDevice dev) {
      vkDestroySemaphore(dev, semaphore, nullptr);
      vkDestroyFence(dev, fence, nullptr);
      vkDestroyRenderPass(dev, render_pass, nullptr);
      image.destroy(dev);
      vkDestroySampler(dev, sampler, nullptr);
      vkDestroyFramebuffer(dev, framebuffer, nullptr);
    }
  };

  static constexpr u32 MAX_FRAMES_INFLIGHT = 2;

  // actually holds the images of the surface and used to present to the render
  // target image. when resizing is needed, the swapchain is destroyed and
  // recreated with the desired extents.
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  VkSurfaceFormatKHR format{.format = VK_FORMAT_R8G8B8A8_SRGB,
                            .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR};
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  VkExtent2D extent{.width = 0, .height = 0};
  VkExtent2D window_extent{.width = 0, .height = 0};

  /// IMPORTANT: this is different from the image index obtained via
  /// `vkAcquireNextImageKHR`. this index is used for referencing semaphores
  /// used for submitting and querying rendering operations. this value is
  /// always increasing and wrapping, unlike the index obtained from
  /// `vkAcquireNextImageKHR` which depends on the presentation mode being used
  /// (determines how the images are used, in what order and whether they
  /// repeat).
  u32 next_frame_flight_index = 0;

  // the images in the swapchain
  stx::Vec<VkImage> images{stx::os_allocator};

  // the image views pointing to a part of a whole texture (images in the
  // swapchain)
  stx::Vec<VkImageView> image_views{stx::os_allocator};

  // the rendering semaphores correspond to the frame indexes and not the
  // swapchain images
  stx::Vec<VkSemaphore> rendering_semaphores{stx::os_allocator};

  stx::Vec<VkSemaphore> image_acquisition_semaphores{stx::os_allocator};

  stx::Vec<VkFence> rendering_fences{stx::os_allocator};

  stx::Vec<VkFramebuffer> frame_buffers{stx::os_allocator};

  VkSampleCountFlagBits msaa_sample_count = VK_SAMPLE_COUNT_1_BIT;

  Image msaa_color_image;

  Image msaa_depth_image;

  VkRenderPass render_pass = VK_NULL_HANDLE;

  Clip clip;

  stx::Rc<CommandQueue*> queue;

  SwapChain(stx::Rc<CommandQueue*> aqueue, VkSurfaceKHR target_surface,
            stx::Span<VkSurfaceFormatKHR const> preferred_formats,
            stx::Span<VkPresentModeKHR const> preferred_present_modes,
            VkExtent2D preferred_extent, VkExtent2D awindow_extent,
            VkSampleCountFlagBits amsaa_sample_count,
            VkCompositeAlphaFlagBitsKHR alpha_compositing)
      : queue{std::move(aqueue)} {
    VkPhysicalDevice phy_device =
        queue.handle->device.handle->phy_device.handle->phy_device;
    VkDevice dev = queue.handle->device.handle->device;

    // the properties change every time we need to create a swapchain so we must
    // query for this every time
    SwapChainProperties properties =
        get_swapchain_properties(phy_device, target_surface);

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

    auto [new_swapchain, new_extent] = create_swapchain(
        dev, target_surface, preferred_extent, selected_format,
        selected_present_mode, properties,
        // not thread-safe since GPUs typically have one graphics queue
        VK_SHARING_MODE_EXCLUSIVE, queue.handle->info.family.index,
        // render target image
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        alpha_compositing,
        // we don't care about the color of pixels that are obscured, for
        // example because another window is in front of them. Unless you really
        // need to be able to read these pixels back and get predictable
        // results, you'll get the best performance by enabling clipping.
        VK_TRUE);

    swapchain = new_swapchain;
    format = selected_format;
    present_mode = selected_present_mode;
    extent = new_extent;
    window_extent = awindow_extent;
    msaa_sample_count = amsaa_sample_count;
    msaa_color_image = create_msaa_color_resource(
        queue, format.format, new_extent, msaa_sample_count);
    msaa_depth_image = create_msaa_depth_resource(
        queue, find_depth_format(phy_device), new_extent, msaa_sample_count);

    for (VkImage image : images) {
      VkImageViewCreateInfo create_info{
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .image = image,
          .viewType = VK_IMAGE_VIEW_TYPE_2D,
          .format = format.format,
          .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                           .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                           .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                           .a = VK_COMPONENT_SWIZZLE_IDENTITY},
          .subresourceRange =
              VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                      .baseMipLevel = 0,
                                      .levelCount = 1,
                                      .baseArrayLayer = 0,
                                      .layerCount = 1}};

      VkImageView view;

      ASR_VK_CHECK(vkCreateImageView(dev, &create_info, nullptr, &view));

      image_views.push_inplace(view).unwrap();
    }

    for (usize i = 0; i < MAX_FRAMES_INFLIGHT; i++) {
      rendering_semaphores.push(create_semaphore(dev)).unwrap();
      image_acquisition_semaphores.push(create_semaphore(dev)).unwrap();
      rendering_fences.push(create_fence(dev, VK_FENCE_CREATE_SIGNALED_BIT))
          .unwrap();
    }

    VkAttachmentDescription color_attachment{
        .flags = 0,
        .format = format.format,
        .samples = msaa_sample_count,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentDescription depth_attachment{
        .flags = 0,
        .format = find_depth_format(phy_device),
        .samples = msaa_sample_count,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentDescription color_attachment_resolve{
        .flags = 0,
        .format = format.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentDescription attachments[] = {color_attachment, depth_attachment,
                                             color_attachment_resolve};

    VkAttachmentReference color_attachment_reference{
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depth_attachment_reference{
        .attachment = 1, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference color_attachment_resolve_reference{
        .attachment = 2, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_reference,
        .pResolveAttachments = &color_attachment_resolve_reference,
        .pDepthStencilAttachment = &depth_attachment_reference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr};

    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0};

    VkRenderPassCreateInfo render_pass_create_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = AS_U32(std::size(attachments)),
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency};

    ASR_VK_CHECK(vkCreateRenderPass(dev, &render_pass_create_info, nullptr,
                                    &render_pass));

    for (usize i = 0; i < images.size(); i++) {
      VkFramebuffer framebuffer;

      VkImageView attachments[] = {msaa_color_image.view, msaa_depth_image.view,
                                   image_views[i]};

      VkFramebufferCreateInfo create_info{
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .renderPass = render_pass,
          .attachmentCount = AS_U32(std::size(attachments)),
          .pAttachments = attachments,
          .width = extent.width,
          .height = extent.height,
          .layers = 1};

      ASR_VK_CHECK(
          vkCreateFramebuffer(dev, &create_info, nullptr, &framebuffer));

      frame_buffers.push_inplace(framebuffer).unwrap();
    }

    {
      auto [image, memory, view] =
          create_bitmap_image(queue, extent.width, extent.height);

      VkRenderPass render_pass;

      {
        VkAttachmentDescription color_attachment{
            .flags = 0,
            .format = format.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_UNDEFINED};

        VkAttachmentReference color_attachment_reference{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        VkSubpassDescription subpass{
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_reference,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr};

        VkSubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = 0};

        VkRenderPassCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = 1,
            .pAttachments = &color_attachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency};

        ASR_VK_CHECK(
            vkCreateRenderPass(dev, &create_info, nullptr, &render_pass));
      }

      VkFramebuffer framebuffer;

      {
        VkImageView attachments[] = {view};

        VkFramebufferCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = render_pass,
            .attachmentCount = AS_U32(std::size(attachments)),
            .pAttachments = attachments,
            .width = extent.width,
            .height = extent.height,
            .layers = 1};

        ASR_VK_CHECK(
            vkCreateFramebuffer(dev, &create_info, nullptr, &framebuffer));
      }

      clip =
          Clip{.image = Image{.image = image, .view = view, .memory = memory},
               .sampler = create_sampler(queue.handle->device, false),
               .framebuffer = framebuffer,
               .fence = create_fence(dev, 0),
               .semaphore = create_semaphore(dev),
               .render_pass = render_pass};
    }
  }

  STX_MAKE_PINNED(SwapChain)

  ~SwapChain() {
    VkDevice dev = queue.handle->device.handle->device;

    // await idleness of the semaphores device, so we can destroy the
    // semaphore and images whislt not in use.
    // any part of the device could be using the semaphore

    ASR_VK_CHECK(vkDeviceWaitIdle(dev));

    clip.destroy(dev);

    vkDestroyRenderPass(dev, render_pass, nullptr);

    msaa_color_image.destroy(dev);
    msaa_depth_image.destroy(dev);

    for (VkFramebuffer framebuffer : frame_buffers) {
      vkDestroyFramebuffer(dev, framebuffer, nullptr);
    }

    for (VkFence fence : rendering_fences) {
      vkDestroyFence(dev, fence, nullptr);
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

  Surface(stx::Rc<Instance*> ainstance, VkSurfaceKHR asurface,
          stx::Option<stx::Unique<SwapChain*>> aswapchain)
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
      VkExtent2D preferred_extent, VkExtent2D window_extent,
      VkSampleCountFlagBits msaa_sample_count,
      VkCompositeAlphaFlagBitsKHR alpha_compositing) {
    swapchain = stx::None;  // probably don't want to have two existing at once

    swapchain = stx::Some(stx::rc::make_unique_inplace<SwapChain>(
                              stx::os_allocator, queue.share(), surface,
                              preferred_formats, preferred_present_modes,
                              preferred_extent, window_extent,
                              msaa_sample_count, alpha_compositing)
                              .unwrap());
  }
};

struct Pipeline {
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkPipelineLayout layout = VK_NULL_HANDLE;
  VkRenderPass target_render_pass = VK_NULL_HANDLE;
  VkSampleCountFlagBits msaa_sample_count = VK_SAMPLE_COUNT_1_BIT;
  VkShaderModule target_vertex_shader = VK_NULL_HANDLE;
  VkShaderModule target_fragment_shader = VK_NULL_HANDLE;
  stx::Rc<Device*> device;

  Pipeline(stx::Rc<Device*> adevice, VkShaderModule atarget_vertex_shader,
           VkShaderModule atarget_fragment_shader,
           VkRenderPass atarget_render_pass,
           VkSampleCountFlagBits amsaa_sample_count,
           stx::Span<VkVertexInputAttributeDescription const> vertex_input_attr,
           usize vertex_input_size)
      : target_render_pass{atarget_render_pass},
        msaa_sample_count{amsaa_sample_count},
        target_vertex_shader{atarget_vertex_shader},
        target_fragment_shader{atarget_fragment_shader},
        device{std::move(adevice)} {
    VkDevice dev = device.handle->device;

    VkPipelineShaderStageCreateInfo vert_shader_stage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = target_vertex_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo frag_shader_stage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = target_fragment_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo stages[] = {vert_shader_stage,
                                                frag_shader_stage};

    VkPipelineLayoutCreateInfo layout_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr};

    ASR_VK_CHECK(
        vkCreatePipelineLayout(dev, &layout_create_info, nullptr, &layout));

    VkPipelineColorBlendAttachmentState color_blend_attachment_states[]{{
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    }};

    VkPipelineColorBlendStateCreateInfo color_blend_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = color_blend_attachment_states,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = VkStencilOpState{.failOp = VK_STENCIL_OP_KEEP,
                                  .passOp = VK_STENCIL_OP_KEEP,
                                  .depthFailOp = VK_STENCIL_OP_KEEP,
                                  .compareOp = VK_COMPARE_OP_NEVER,
                                  .compareMask = 0,
                                  .writeMask = 0,
                                  .reference = 0},
        .back = VkStencilOpState{.failOp = VK_STENCIL_OP_KEEP,
                                 .passOp = VK_STENCIL_OP_KEEP,
                                 .depthFailOp = VK_STENCIL_OP_KEEP,
                                 .compareOp = VK_COMPARE_OP_NEVER,
                                 .compareMask = 0,
                                 .writeMask = 0,
                                 .reference = 0},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f};

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    VkPipelineMultisampleStateCreateInfo multisample_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = msaa_sample_count,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    VkPipelineRasterizationStateCreateInfo rasterization_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f};

    VkVertexInputBindingDescription vertex_binding_descriptions[] = {
        {.binding = 0,
         .stride = AS_U32(vertex_input_size),
         .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}};

    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount =
            AS_U32(std::size(vertex_binding_descriptions)),
        .pVertexBindingDescriptions = vertex_binding_descriptions,
        .vertexAttributeDescriptionCount = AS_U32(vertex_input_attr.size()),
        .pVertexAttributeDescriptions = vertex_input_attr.data()};

    VkPipelineViewportStateCreateInfo viewport_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr};

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_SCISSOR,
                                       VK_DYNAMIC_STATE_VIEWPORT};

    VkPipelineDynamicStateCreateInfo dynamic_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = AS_U32(std::size(dynamic_states)),
        .pDynamicStates = dynamic_states};

    VkGraphicsPipelineCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = AS_U32(std::size(stages)),
        .pStages = stages,
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multisample_state,
        .pDepthStencilState = &depth_stencil_state,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = &dynamic_state,
        .layout = layout,
        .renderPass = target_render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    ASR_VK_CHECK(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &create_info,
                                           nullptr, &pipeline));
  }

  STX_MAKE_PINNED(Pipeline)

  ~Pipeline() {
    vkDestroyPipelineLayout(device.handle->device, layout, nullptr);
    vkDestroyPipeline(device.handle->device, pipeline, nullptr);
  }
};

struct ClipPipeline {
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkPipelineLayout layout = VK_NULL_HANDLE;
  VkRenderPass target_render_pass = VK_NULL_HANDLE;
  VkShaderModule target_vertex_shader = VK_NULL_HANDLE;
  VkShaderModule target_fragment_shader = VK_NULL_HANDLE;
  stx::Rc<Device*> device;

  ClipPipeline(
      stx::Rc<Device*> adevice, VkShaderModule atarget_vertex_shader,
      VkShaderModule atarget_fragment_shader, VkRenderPass atarget_render_pass,
      stx::Span<VkVertexInputAttributeDescription const> vertex_input_attr,
      usize vertex_input_size)
      : target_render_pass{atarget_render_pass},
        target_vertex_shader{atarget_vertex_shader},
        target_fragment_shader{atarget_fragment_shader},
        device{std::move(adevice)} {
    VkDevice dev = device.handle->device;

    VkPipelineShaderStageCreateInfo vert_shader_stage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = target_vertex_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo frag_shader_stage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = target_fragment_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo stages[] = {vert_shader_stage,
                                                frag_shader_stage};

    VkPipelineLayoutCreateInfo layout_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr};

    ASR_VK_CHECK(
        vkCreatePipelineLayout(dev, &layout_create_info, nullptr, &layout));

    VkPipelineColorBlendAttachmentState color_blend_attachment_states[]{
        {.blendEnable = VK_TRUE,
         .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
         .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
         .colorBlendOp = VK_BLEND_OP_ADD,
         .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
         .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
         .alphaBlendOp = VK_BLEND_OP_ADD,
         // only R component is written
         .colorWriteMask = VK_COLOR_COMPONENT_R_BIT}};

    VkPipelineColorBlendStateCreateInfo color_blend_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = color_blend_attachment_states,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    VkPipelineMultisampleStateCreateInfo multisample_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    VkPipelineRasterizationStateCreateInfo rasterization_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f};

    VkVertexInputBindingDescription vertex_binding_descriptions[] = {
        {.binding = 0,
         .stride = AS_U32(vertex_input_size),
         .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}};

    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount =
            AS_U32(std::size(vertex_binding_descriptions)),
        .pVertexBindingDescriptions = vertex_binding_descriptions,
        .vertexAttributeDescriptionCount = AS_U32(vertex_input_attr.size()),
        .pVertexAttributeDescriptions = vertex_input_attr.data()};

    VkPipelineViewportStateCreateInfo viewport_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr};

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_SCISSOR,
                                       VK_DYNAMIC_STATE_VIEWPORT};

    VkPipelineDynamicStateCreateInfo dynamic_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = AS_U32(std::size(dynamic_states)),
        .pDynamicStates = dynamic_states};

    VkGraphicsPipelineCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = AS_U32(std::size(stages)),
        .pStages = stages,
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multisample_state,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = &dynamic_state,
        .layout = layout,
        .renderPass = target_render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    ASR_VK_CHECK(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &create_info,
                                           nullptr, &pipeline));
  }

  STX_MAKE_PINNED(ClipPipeline)

  ~ClipPipeline() {
    vkDestroyPipelineLayout(device.handle->device, layout, nullptr);
    vkDestroyPipeline(device.handle->device, pipeline, nullptr);
  }
};

struct RecordingContext {
  VkCommandPool command_pool = VK_NULL_HANDLE;
  VkCommandBuffer command_buffer = VK_NULL_HANDLE;
  VkCommandBuffer clip_command_buffer = VK_NULL_HANDLE;
  VkShaderModule vertex_shader = VK_NULL_HANDLE;
  VkShaderModule fragment_shader = VK_NULL_HANDLE;
  VkShaderModule clip_vertex_shader = VK_NULL_HANDLE;
  VkShaderModule clip_fragment_shader = VK_NULL_HANDLE;
  Pipeline pipeline;
  ClipPipeline clip_pipeline;
  // TODO(lamarrr): each in-flight frame will have a descriptor set
  stx::Vec<DescriptorSets> descriptor_sets{stx::os_allocator};
  DescriptorSets clip_descriptor_sets;
  u32 next_swapchain_image_index = 0;
  stx::Rc<Surface*> surface;
  stx::Rc<CommandQueue*> queue;

  RecordingContext(stx::Rc<Surface*> asurface, stx::Rc<CommandQueue*> aqueue,
                   stx::Span<u32 const> vertex_shader_code,
                   stx::Span<u32 const> fragment_shader_code,
                   stx::Span<u32 const> clip_vertex_shader_code,
                   stx::Span<u32 const> clip_fragment_shader_code)
      : surface{std::move(asurface)}, queue{std::move(aqueue)} {
    VkDevice dev = queue.handle->device.handle->device;

    auto create_shader = [dev](stx::Span<u32 const> code) {
      VkShaderModuleCreateInfo create_info{
          .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .codeSize = code.size_bytes(),
          .pCode = code.data()};

      VkShaderModule shader;

      ASR_VK_CHECK(vkCreateShaderModule(dev, &create_info, nullptr, &shader));

      return shader;
    };

    vertex_shader = create_shader(vertex_shader_code);
    fragment_shader = create_shader(fragment_shader_code);
    clip_vertex_shader = create_shader(clip_vertex_shader_code);
    clip_fragment_shader = create_shader(clip_fragment_shader_code);

    VkCommandPoolCreateInfo command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue.handle->info.family.index};

    ASR_VK_CHECK(vkCreateCommandPool(dev, &command_pool_create_info, nullptr,
                                     &command_pool));

    VkCommandBufferAllocateInfo command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    ASR_VK_CHECK(vkAllocateCommandBuffers(dev, &command_buffer_allocate_info,
                                          &command_buffer));
    ASR_VK_CHECK(vkAllocateCommandBuffers(dev, &command_buffer_allocate_info,
                                          &clip_command_buffer));
  }

  STX_MAKE_PINNED(RecordingContext)

  ~RecordingContext() {
    VkDevice dev = queue.handle->device.handle->device;

    vkDestroyShaderModule(dev, vertex_shader, nullptr);
    vkDestroyShaderModule(dev, fragment_shader, nullptr);
    vkDestroyShaderModule(dev, clip_vertex_shader, nullptr);
    vkDestroyShaderModule(dev, clip_fragment_shader, nullptr);

    vkFreeCommandBuffers(dev, command_pool, 1, &command_buffer);
    vkFreeCommandBuffers(dev, command_pool, 1, &clip_command_buffer);

    vkDestroyCommandPool(dev, command_pool, nullptr);
  }
};

}  // namespace vk
}  // namespace asr
