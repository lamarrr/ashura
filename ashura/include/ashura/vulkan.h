#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
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
#include "vulkan/vk_enum_string_helper.h"
#include "vulkan/vulkan.h"

#define ASR_VK_CHECK(...)                             \
  do {                                                \
    VkResult operation_result = (__VA_ARGS__);        \
    ASR_CHECK(operation_result == VK_SUCCESS,         \
              "Vulkan Operation: (" #__VA_ARGS__      \
              ")  failed! (VK_SUCCESS not returned)", \
              string_VkResult(operation_result));     \
  } while (false)

namespace asr {
using namespace std::chrono_literals;

static constexpr u64 COMMAND_TIMEOUT =
    AS_U64(std::chrono::duration_cast<std::chrono::nanoseconds>(1min).count());

namespace vk {

// NICE-TO-HAVE(lamarrr): versioning of extensions, know which one wasn't
// available and adjust features to that
inline void ensure_extensions_supported(
    stx::Span<VkExtensionProperties const> available_extentions,
    stx::Span<char const* const> required_extensions) {
  bool all_available = true;

  for (char const* required_extension : required_extensions) {
    if (available_extentions
            .which([required_extension](VkExtensionProperties const& props) {
              return std::string_view(required_extension) ==
                     std::string_view(props.extensionName);
            })
            .is_empty()) {
      all_available = false;
      ASR_LOG_WARN("Required extension `{}` is not available",
                   std::string_view(required_extension));
    }
  }

  ASR_CHECK(all_available, "one or more required extensions are not available");
}

inline void ensure_validation_layers_supported(
    stx::Span<VkLayerProperties const> available_validation_layers,
    stx::Span<char const* const> required_layers) {
  bool all_layers_available = true;

  for (char const* required_layer : required_layers) {
    if (available_validation_layers
            .which([required_layer](VkLayerProperties const& available_layer) {
              return std::string_view(required_layer) ==
                     std::string_view(available_layer.layerName);
            })
            .is_empty()) {
      all_layers_available = false;
      ASR_LOG_WARN("Required validation layer `{}` is not available",
                   std::string_view(required_layer));
    }
  }

  ASR_CHECK(all_layers_available,
            "one or more required validation layers are not available");
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

  // typedef struct VkDebugUtilsMessengerCallbackDataEXT {
  //     VkStructureType                              sType;
  //     const void*                                  pNext;
  //     VkDebugUtilsMessengerCallbackDataFlagsEXT    flags;
  //     const char*                                  pMessageIdName;
  //     int32_t                                      messageIdNumber;
  //     const char*                                  pMessage;
  //     uint32_t                                     queueLabelCount;
  //     const VkDebugUtilsLabelEXT*                  pQueueLabels;
  //     uint32_t                                     cmdBufLabelCount;
  //     const VkDebugUtilsLabelEXT*                  pCmdBufLabels;
  //     uint32_t                                     objectCount;
  //     const VkDebugUtilsObjectNameInfoEXT*         pObjects;
  // } VkDebugUtilsMessengerCallbackDataEXT;

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

inline std::pair<VkInstance, VkDebugUtilsMessengerEXT> create_vulkan_instance(
    stx::Span<char const* const> irequired_extensions,
    stx::Span<char const* const> required_validation_layers,
    char const* const application_name, u32 application_version,
    char const* const engine_name, u32 engine_version) {
  VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{
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

  static constexpr char const* DEBUG_EXTENSIONS[] = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

  // debug message callback extension
  stx::Vec<char const*> required_extensions{stx::os_allocator};

  required_extensions.extend(irequired_extensions).unwrap();

  if (!required_validation_layers.is_empty())
    required_extensions.extend(DEBUG_EXTENSIONS).unwrap();

  u32 available_extensions_count = 0;

  vkEnumerateInstanceExtensionProperties(nullptr, &available_extensions_count,
                                         nullptr);

  stx::Vec<VkExtensionProperties> available_extensions(stx::os_allocator);

  available_extensions.resize(available_extensions_count).unwrap();

  vkEnumerateInstanceExtensionProperties(nullptr, &available_extensions_count,
                                         available_extensions.data());

  ASR_LOG("Available Vulkan Extensions:");

  for (VkExtensionProperties extension : available_extensions) {
    ASR_LOG("\t{},  spec version: {}", extension.extensionName,
            extension.specVersion);
  }

  u32 available_validation_layers_count;

  ASR_VK_CHECK(vkEnumerateInstanceLayerProperties(
      &available_validation_layers_count, nullptr));

  stx::Vec<VkLayerProperties> available_validation_layers(stx::os_allocator);

  available_validation_layers.resize(available_validation_layers_count)
      .unwrap();

  ASR_VK_CHECK(vkEnumerateInstanceLayerProperties(
      &available_validation_layers_count, available_validation_layers.data()));

  ASR_LOG("Available Vulkan Validation Layers:");

  for (VkLayerProperties const& layer : available_validation_layers) {
    ASR_LOG("\t{} (spec version: {})", layer.layerName, layer.specVersion);
  }

  ensure_extensions_supported(available_extensions, required_extensions);

  ensure_validation_layers_supported(available_validation_layers,
                                     required_validation_layers);

  // helps but not necessary
  VkApplicationInfo app_info{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = application_name,
      .applicationVersion = application_version,
      .pEngineName = engine_name,
      .engineVersion = engine_version,
      .apiVersion = VK_API_VERSION_1_3,
  };

  VkInstanceCreateInfo create_info{
      .sType =
          VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,  // debug messenger for when
                                                   // the installed debug
                                                   // messenger is uninstalled.
      // this helps to debug issues with vkDestroyInstance and vkCreateInstance
      // i.e. (before and after the debug messenger is installed)
      .pNext = required_validation_layers.is_empty()
                   ? nullptr
                   : &debug_utils_messenger_create_info,
      .flags = 0,
      .pApplicationInfo = &app_info,  // validation layers
      .enabledLayerCount = AS_U32(required_validation_layers.size()),
      .ppEnabledLayerNames = required_validation_layers.data(),
      .enabledExtensionCount = AS_U32(required_extensions.size()),
      .ppEnabledExtensionNames = required_extensions.data(),
  };

  VkInstance instance;

  ASR_VK_CHECK(vkCreateInstance(&create_info, nullptr, &instance));

  VkDebugUtilsMessengerEXT debug_utils_messenger;

  if (!required_validation_layers.is_empty()) {
    PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    ASR_CHECK(
        createDebugUtilsMessengerEXT != nullptr,
        "unable to get procedure address for vkCreateDebugUtilsMessengerEXT");

    ASR_VK_CHECK(createDebugUtilsMessengerEXT(
        instance, &debug_utils_messenger_create_info, nullptr,
        &debug_utils_messenger));
  }

  return std::make_pair(instance, debug_utils_messenger);
}

//  to do anything on the GPU (render, draw, compute, allocate memory, create
//  texture, etc.) we use command queues
inline stx::Vec<VkQueueFamilyProperties> get_queue_families(
    VkPhysicalDevice dev) {
  u32 queue_families_count;

  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_families_count, nullptr);

  stx::Vec<VkQueueFamilyProperties> queue_families_properties(
      stx::os_allocator);

  queue_families_properties.resize(queue_families_count).unwrap();

  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_families_count,
                                           queue_families_properties.data());

  return queue_families_properties;
}

inline stx::Vec<bool> get_command_queue_support(
    stx::Span<VkQueueFamilyProperties const> queue_families,
    VkQueueFlagBits required_command_queue) {
  stx::Vec<bool> supports{stx::os_allocator};

  for (VkQueueFamilyProperties const& fam_props : queue_families) {
    supports.push(fam_props.queueFlags & required_command_queue).unwrap();
  }

  return supports;
}

// find the device's queue family capable of supporting surface presentation
inline stx::Vec<bool> get_surface_presentation_command_queue_support(
    VkPhysicalDevice phy_dev,
    stx::Span<VkQueueFamilyProperties const> queue_families,
    VkSurfaceKHR surface) {
  stx::Vec<bool> supports{stx::os_allocator};

  for (u32 i = 0; i < AS_U32(queue_families.size()); i++) {
    VkBool32 surface_presentation_supported;
    ASR_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(
        phy_dev, i, surface, &surface_presentation_supported));
    supports.push_inplace(surface_presentation_supported == VK_TRUE).unwrap();
  }

  return supports;
}

inline VkDevice create_logical_device(
    VkPhysicalDevice phy_dev, stx::Span<char const* const> required_extensions,
    stx::Span<char const* const> required_validation_layers,
    stx::Span<VkDeviceQueueCreateInfo const> command_queue_create_infos,
    VkPhysicalDeviceFeatures const& required_features = {}) {
  u32 available_extensions_count;
  ASR_VK_CHECK(vkEnumerateDeviceExtensionProperties(
      phy_dev, nullptr, &available_extensions_count, nullptr));

  // device specific extensions
  stx::Vec<VkExtensionProperties> available_device_extensions{
      stx::os_allocator};

  available_device_extensions.resize(available_extensions_count).unwrap();

  ASR_VK_CHECK(vkEnumerateDeviceExtensionProperties(
      phy_dev, nullptr, &available_extensions_count,
      available_device_extensions.data()));

  ASR_LOG("Required Device Extensions: ");

  required_extensions.for_each([](char const* ext) { ASR_LOG("\t{}", ext); });

  ASR_LOG("Available Device Extensions: ");

  available_device_extensions.span().for_each([](VkExtensionProperties ext) {
    ASR_LOG("\t{} (spec version: {})", ext.extensionName, ext.specVersion);
  });

  ASR_CHECK(required_extensions.is_all([&](char const* ext) {
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

  ASR_VK_CHECK(
      vkCreateDevice(phy_dev, &device_create_info, nullptr, &logical_device));

  return logical_device;
}

struct SwapChainProperties {
  VkSurfaceCapabilitiesKHR capabilities;
  stx::Vec<VkSurfaceFormatKHR> supported_formats{stx::os_allocator};
  stx::Vec<VkPresentModeKHR> presentation_modes{stx::os_allocator};
};

inline SwapChainProperties get_swapchain_properties(VkPhysicalDevice phy_dev,
                                                    VkSurfaceKHR surface) {
  SwapChainProperties details;

  ASR_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      phy_dev, surface, &details.capabilities));

  u32 supported_surface_formats_count = 0;

  ASR_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
      phy_dev, surface, &supported_surface_formats_count, nullptr));

  details.supported_formats.resize(supported_surface_formats_count).unwrap();

  ASR_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
      phy_dev, surface, &supported_surface_formats_count,
      details.supported_formats.data()));

  u32 surface_presentation_modes_count;

  ASR_VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
      phy_dev, surface, &surface_presentation_modes_count, nullptr));

  details.presentation_modes.resize(surface_presentation_modes_count).unwrap();

  ASR_VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
      phy_dev, surface, &surface_presentation_modes_count,
      details.presentation_modes.data()));

  return details;
}

inline bool is_swapchain_adequate(SwapChainProperties const& properties) {
  // we use any available for selecting devices
  ASR_CHECK(!properties.supported_formats.is_empty(),
            "Physical Device does not support any window surface "
            "format");

  ASR_CHECK(!properties.presentation_modes.is_empty(),
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
    VkSurfaceCapabilitiesKHR const& capabilities, u32 desired_nbuffers) {
  return
      // no limit on the number of swapchain images
      capabilities.maxImageCount == 0
          ? std::clamp(desired_nbuffers, capabilities.minImageCount,
                       stx::u32_max)
          : std::clamp(desired_nbuffers, capabilities.minImageCount,
                       capabilities.maxImageCount);
}

inline std::pair<VkSwapchainKHR, VkExtent2D> create_swapchain(
    VkDevice dev, VkSurfaceKHR surface, VkExtent2D preferred_extent,
    VkSurfaceFormatKHR surface_format, VkPresentModeKHR present_mode,
    SwapChainProperties const& properties,
    VkSharingMode accessing_queue_families_sharing_mode,
    VkImageUsageFlags image_usages,
    VkCompositeAlphaFlagBitsKHR alpha_channel_blending, VkBool32 clipped) {
  u32 desired_nbuffers = std::min(properties.capabilities.minImageCount + 1,
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
                                                    desired_nbuffers),
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
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
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
  ASR_VK_CHECK(vkCreateSwapchainKHR(dev, &create_info, nullptr, &swapchain));

  return std::make_pair(swapchain, selected_extent);
}

inline stx::Vec<VkImage> get_swapchain_images(VkDevice dev,
                                              VkSwapchainKHR swapchain) {
  u32 image_count;

  ASR_VK_CHECK(vkGetSwapchainImagesKHR(dev, swapchain, &image_count, nullptr));

  stx::Vec<VkImage> swapchain_images{stx::os_allocator};
  swapchain_images.resize(image_count).unwrap();

  ASR_VK_CHECK(vkGetSwapchainImagesKHR(dev, swapchain, &image_count,
                                       swapchain_images.data()));

  return swapchain_images;
}

// get memory requirements for an image based on it's type, usage mode, and
// other properties
inline VkMemoryRequirements get_memory_requirements(VkDevice dev,
                                                    VkImage image) {
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(dev, image, &memory_requirements);
  return memory_requirements;
}

// returns index of the heap on the physical device, could be RAM, SWAP, or VRAM
inline stx::Option<u32> find_suitable_memory_type(
    VkPhysicalDeviceMemoryProperties const& memory_properties,
    VkMemoryRequirements const& memory_requirements,
    VkMemoryPropertyFlags required_properties) {
  // different types of memory exist within the graphics card heap memory.
  // this can affect performance.
  for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
    if ((memory_properties.memoryTypes[i].propertyFlags &
         required_properties) == required_properties &&
        (memory_requirements.memoryTypeBits & (1 << i))) {
      return stx::Some(AS_U32(i));
    }
  }
  return stx::None;
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
      return "unidentified Device Type";
  }
}

struct Instance {
  VkInstance instance = VK_NULL_HANDLE;
  stx::Option<VkDebugUtilsMessengerEXT> debug_utils_messenger = stx::None;

  Instance(VkInstance ainstance,
           stx::Option<VkDebugUtilsMessengerEXT> adebug_utils_messenger)
      : instance{ainstance},
        debug_utils_messenger{std::move(adebug_utils_messenger)} {}

  STX_MAKE_PINNED(Instance)

  ~Instance() {
    if (debug_utils_messenger.is_some()) {
      PFN_vkDestroyDebugUtilsMessengerEXT func =
          reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
              vkGetInstanceProcAddr(instance,
                                    "vkDestroyDebugUtilsMessengerEXT"));

      ASR_CHECK(func != nullptr,
                "unable to get procedure address for "
                "vkDestroyDebugUtilsMessengerEXT");

      func(instance, debug_utils_messenger.value(), nullptr);
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

  ASR_CHECK(devices_count != 0, "No Physical Device Found");

  stx::Vec<VkPhysicalDevice> phy_devices{stx::os_allocator};

  phy_devices.resize(devices_count).unwrap();

  ASR_VK_CHECK(vkEnumeratePhysicalDevices(instance.handle->instance,
                                          &devices_count, phy_devices.data()));

  stx::Vec<PhyDeviceInfo> devices{stx::os_allocator};

  for (VkPhysicalDevice dev : phy_devices) {
    VkPhysicalDeviceProperties device_properties;

    vkGetPhysicalDeviceProperties(dev, &device_properties);

    VkPhysicalDeviceFeatures device_features;

    vkGetPhysicalDeviceFeatures(dev, &device_features);

    VkPhysicalDeviceMemoryProperties memory_properties;

    vkGetPhysicalDeviceMemoryProperties(dev, &memory_properties);

    devices
        .push(PhyDeviceInfo{.phy_device = dev,
                            .properties = device_properties,
                            .features = device_features,
                            .memory_properties = memory_properties,
                            .family_properties = get_queue_families(dev),
                            .instance = instance.share()})
        .unwrap();
  }

  return devices;
}

inline std::string format(PhyDeviceInfo const& dev) {
  return fmt::format("Device(name: '{}', ID: {}, type: {}) ",
                     dev.properties.deviceName, dev.properties.deviceID,
                     ::asr::vk::format(dev.properties.deviceType));
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
  f32 priority = 0;
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
  auto [instance, debug_utils_messenger] =
      create_vulkan_instance(required_extensions, validation_layers, app_name,
                             app_version, engine_name, engine_version);

  return stx::rc::make_inplace<Instance>(
             stx::os_allocator, instance,
             debug_utils_messenger == VK_NULL_HANDLE
                 ? stx::None
                 : stx::make_some(std::move(debug_utils_messenger)))
      .unwrap();
}

// can also be used for transfer
inline stx::Option<CommandQueueFamilyInfo> get_graphics_command_queue(
    stx::Rc<PhyDeviceInfo*> const& phy_dev) {
  auto pos = std::find_if(phy_dev.handle->family_properties.begin(),
                          phy_dev.handle->family_properties.end(),
                          [](VkQueueFamilyProperties const& prop) -> bool {
                            return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                          });

  if (pos == phy_dev.handle->family_properties.end()) {
    return stx::None;
  }

  return stx::Some(CommandQueueFamilyInfo{
      .index = AS_U32(pos - phy_dev.handle->family_properties.begin()),
      .phy_device = phy_dev.share()});
}

inline stx::Rc<Device*> create_device(
    stx::Rc<PhyDeviceInfo*> const& phy_dev,
    stx::Span<VkDeviceQueueCreateInfo const> command_queue_create_info,
    stx::Span<char const* const> required_extensions,
    stx::Span<char const* const> required_validation_layers,
    VkPhysicalDeviceFeatures required_features) {
  VkDevice dev = create_logical_device(
      phy_dev.handle->phy_device, required_extensions,
      required_validation_layers, command_queue_create_info, required_features);

  stx::Vec<CommandQueueInfo> command_queues{stx::os_allocator};

  for (usize i = 0; i < command_queue_create_info.size(); i++) {
    VkDeviceQueueCreateInfo create_info = command_queue_create_info[i];
    u32 command_queue_family_index = create_info.queueFamilyIndex;
    u32 queue_count = create_info.queueCount;
    ASR_CHECK(command_queue_family_index <
              phy_dev.handle->family_properties.size());

    for (u32 queue_index_in_family = 0; queue_index_in_family < queue_count;
         queue_index_in_family++) {
      f32 priority = create_info.pQueuePriorities[i];

      VkQueue command_queue;

      vkGetDeviceQueue(dev, command_queue_family_index, queue_index_in_family,
                       &command_queue);

      ASR_CHECK(command_queue != nullptr,
                "requested command queue not created on target device");

      command_queues
          .push(CommandQueueInfo{
              .queue = command_queue,
              .create_index = AS_U32(i),
              .priority = priority,
              .family =
                  CommandQueueFamilyInfo{.index = command_queue_family_index,
                                         .phy_device = phy_dev.share()},
          })
          .unwrap();
    }
  }

  return stx::rc::make_inplace<Device>(stx::os_allocator, dev, phy_dev.share(),
                                       std::move(command_queues))
      .unwrap();
}

inline stx::Option<CommandQueue> get_command_queue(
    stx::Rc<Device*> const& device, CommandQueueFamilyInfo const& family,
    u32 command_queue_create_index) {
  // We shouldn't have to perform checks?
  ASR_CHECK(device.handle->phy_device.handle->phy_device ==
            family.phy_device.handle->phy_device);

  stx::Span queue_s = device.handle->command_queues.span().which(
      [&](CommandQueueInfo const& info) {
        return info.family.index == family.index &&
               info.create_index == command_queue_create_index;
      });

  if (queue_s.is_empty()) return stx::None;

  CommandQueueInfo const& queue = queue_s[0];

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
  void write(VkDevice dev,
             VkPhysicalDeviceMemoryProperties const& memory_properties,
             VkBufferUsageFlags usage, stx::Span<T const> span) {
    ASR_CHECK(!span.is_empty());
    if (span.size_bytes() != size) {
      vkDestroyBuffer(dev, buffer, nullptr);

      VkBufferCreateInfo create_info{
          .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .size = span.size_bytes(),
          .usage = usage,
          .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
          .queueFamilyIndexCount = 0,
          .pQueueFamilyIndices = nullptr};

      ASR_VK_CHECK(vkCreateBuffer(dev, &create_info, nullptr, &buffer));

      size = span.size_bytes();

      VkMemoryRequirements memory_requirements;

      vkGetBufferMemoryRequirements(dev, buffer, &memory_requirements);

      if (memory_requirements.size <= memory_size) {
        if (memory != VK_NULL_HANDLE) {
          ASR_VK_CHECK(vkBindBufferMemory(dev, buffer, memory, 0));
        }
      } else {
        vkFreeMemory(dev, memory, nullptr);

        u32 memory_type_index = vk::find_suitable_memory_type(
                                    memory_properties, memory_requirements,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                                    .unwrap();

        VkMemoryAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_type_index};

        ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

        memory_size = memory_requirements.size;

        ASR_VK_CHECK(vkBindBufferMemory(dev, buffer, memory, 0));

        ASR_VK_CHECK(
            vkMapMemory(dev, memory, 0, VK_WHOLE_SIZE, 0, &memory_map));
      }
    }

    if (!span.is_empty()) {
      memcpy(memory_map, span.data(), span.size_bytes());

      VkMappedMemoryRange range{.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                                .pNext = nullptr,
                                .memory = memory,
                                .offset = 0,
                                .size = VK_WHOLE_SIZE};

      ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));
    }
  }
};

inline Buffer create_host_buffer(
    VkDevice dev, VkPhysicalDeviceMemoryProperties const& memory_properties,
    usize size_bytes, VkBufferUsageFlags usage) {
  VkBufferCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size = size_bytes,
                                 .usage = usage,
                                 .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                 .queueFamilyIndexCount = 0,
                                 .pQueueFamilyIndices = nullptr};

  VkBuffer buffer;

  ASR_VK_CHECK(vkCreateBuffer(dev, &create_info, nullptr, &buffer));

  VkDeviceMemory memory;

  VkMemoryRequirements memory_requirements;

  vkGetBufferMemoryRequirements(dev, buffer, &memory_requirements);

  u32 memory_type_index =
      find_suitable_memory_type(memory_properties, memory_requirements,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
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

struct ImageResource {
  VkImage image = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  stx::Rc<CommandQueue*> queue;

  ImageResource(VkImage aimage, VkImageView aview, VkDeviceMemory amemory,
                stx::Rc<CommandQueue*> aqueue)
      : image{aimage},
        view{aview},
        memory{amemory},
        queue{std::move(aqueue)} {};

  STX_MAKE_PINNED(ImageResource)

  ~ImageResource() {
    VkDevice dev = queue.handle->device.handle->device;
    ASR_VK_CHECK(vkDeviceWaitIdle(dev));
    vkFreeMemory(dev, memory, nullptr);
    vkDestroyImageView(dev, view, nullptr);
    vkDestroyImage(dev, image, nullptr);
  }
};

struct ImageSampler {
  VkSampler sampler = VK_NULL_HANDLE;
  stx::Rc<ImageResource*> image;

  ImageSampler(VkSampler asampler, stx::Rc<ImageResource*> aimage)
      : sampler{asampler}, image{std::move(aimage)} {}

  STX_MAKE_PINNED(ImageSampler)

  ~ImageSampler() {
    ASR_VK_CHECK(
        vkDeviceWaitIdle(image.handle->queue.handle->device.handle->device));
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
      .mipLodBias = 0,
      .anisotropyEnable = enable_anisotropy,
      .maxAnisotropy = device.handle->phy_device.handle->properties.limits
                           .maxSamplerAnisotropy,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .minLod = 0,
      .maxLod = 0,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE};

  VkSampler sampler;

  ASR_VK_CHECK(
      vkCreateSampler(device.handle->device, &create_info, nullptr, &sampler));

  return sampler;
}

inline stx::Rc<ImageSampler*> create_image_sampler(
    stx::Rc<ImageResource*> const& image) {
  return stx::rc::make_inplace<ImageSampler>(
             stx::os_allocator,
             create_sampler(image.handle->queue.handle->device, VK_TRUE),
             image.share())
      .unwrap();
}

enum class DescriptorType : u8 { UniformBuffer, CombinedImageSampler };

struct DescriptorBinding {
  DescriptorType type = DescriptorType::UniformBuffer;

  // only valid if type is DescriptorType::UniformBuffer
  VkBuffer buffer = VK_NULL_HANDLE;

  // only valid if type is DescriptorType::CombinedImageSampler
  VkImageView view = VK_NULL_HANDLE;

  // only valid if type is DescriptorType::CombinedImageSampler
  VkSampler sampler = VK_NULL_HANDLE;

  static constexpr DescriptorBinding make_buffer(VkBuffer buffer) {
    return DescriptorBinding{.type = DescriptorType::UniformBuffer,
                             .buffer = buffer};
  }

  static constexpr DescriptorBinding make_sampler(VkImageView view,
                                                  VkSampler sampler) {
    return DescriptorBinding{.type = DescriptorType::CombinedImageSampler,
                             .view = view,
                             .sampler = sampler};
  }

  constexpr bool operator==(DescriptorBinding const& other) const {
    return type == other.type && buffer == other.buffer && view == other.view &&
           sampler == other.sampler;
  }
};

struct DescriptorSetSpec {
  stx::Vec<DescriptorType> bindings{stx::os_allocator};

  explicit DescriptorSetSpec(std::initializer_list<DescriptorType> abindings) {
    bindings.extend(abindings).unwrap();
  }

  explicit DescriptorSetSpec(stx::Span<DescriptorType const> abindings) {
    bindings.extend(abindings).unwrap();
  }

  DescriptorSetSpec() {}
};

inline Image create_msaa_color_resource(
    VkDevice dev, VkPhysicalDeviceMemoryProperties const& memory_properties,
    VkFormat swapchain_format, VkExtent2D swapchain_extent,
    VkSampleCountFlagBits sample_count) {
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
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

  VkImage image;

  ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

  VkMemoryRequirements memory_requirements;

  vkGetImageMemoryRequirements(dev, image, &memory_requirements);

  u32 memory_type_index =
      find_suitable_memory_type(memory_properties, memory_requirements,
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
      .format = swapchain_format,
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

inline Image create_msaa_depth_resource(
    VkDevice dev, VkPhysicalDeviceMemoryProperties const& memory_properties,
    VkFormat depth_format, VkExtent2D swapchain_extent,
    VkSampleCountFlagBits sample_count) {
  VkImageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = depth_format,
      .extent = VkExtent3D{.width = swapchain_extent.width,
                           .height = swapchain_extent.height,
                           .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = sample_count,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

  VkImage image;

  ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

  VkMemoryRequirements memory_requirements;

  vkGetImageMemoryRequirements(dev, image, &memory_requirements);

  u32 memory_type_index =
      find_suitable_memory_type(memory_properties, memory_requirements,
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
      .format = depth_format,
      .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
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
  ASR_CHECK(!formats.is_empty(),
            "no window surface format supported by physical device");

  for (VkSurfaceFormatKHR preferred_format : preferred_formats) {
    if (!formats
             .which([&](VkSurfaceFormatKHR format) {
               return preferred_format.colorSpace == format.colorSpace &&
                      preferred_format.format == format.format;
             })
             .is_empty())
      return preferred_format;
  }

  ASR_PANIC("unable to find any of the preferred swapchain surface formats");
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

  ASR_CHECK(!available_presentation_modes.is_empty(),
            "no surface presentation mode available");

  for (VkPresentModeKHR preferred_present_mode : preferred_present_modes) {
    if (!available_presentation_modes.find(preferred_present_mode).is_empty())
      return preferred_present_mode;
  }

  ASR_PANIC("unable to find any of the preferred presentation modes");
}

inline VkFormat find_supported_format(VkPhysicalDevice phy_dev,
                                      stx::Span<VkFormat const> candidates,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(phy_dev, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  ASR_PANIC("could not find any supported format");
}

inline VkFormat find_depth_format(VkPhysicalDevice phy_dev) {
  VkFormat formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                        VK_FORMAT_D24_UNORM_S8_UINT};

  return find_supported_format(phy_dev, formats, VK_IMAGE_TILING_OPTIMAL,
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
  static constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

  // actually holds the images of the surface and used to present to the render
  // target image. when resizing is needed, the swapchain is destroyed and
  // recreated with the desired extents.
  VkSurfaceFormatKHR color_format{
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR};
  VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  VkExtent2D image_extent{.width = 0, .height = 0};
  VkExtent2D window_extent{.width = 0, .height = 0};

  VkSampleCountFlagBits msaa_sample_count = VK_SAMPLE_COUNT_1_BIT;

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

  stx::Vec<VkFramebuffer> framebuffers{stx::os_allocator};

  // the rendering semaphores correspond to the frame indexes and not the
  // swapchain images
  stx::Vec<VkSemaphore> rendering_semaphores{stx::os_allocator};

  stx::Vec<VkSemaphore> image_acquisition_semaphores{stx::os_allocator};

  stx::Vec<VkFence> rendering_fences{stx::os_allocator};

  stx::Vec<VkFence> image_acquisition_fences{stx::os_allocator};

  Image msaa_color_image;

  Image msaa_depth_image;

  VkRenderPass render_pass = VK_NULL_HANDLE;

  VkSwapchainKHR swapchain = VK_NULL_HANDLE;

  stx::Rc<CommandQueue*> queue;

  SwapChain(stx::Rc<CommandQueue*> aqueue, VkSurfaceKHR target_surface,
            stx::Span<VkSurfaceFormatKHR const> preferred_formats,
            stx::Span<VkPresentModeKHR const> preferred_present_modes,
            VkExtent2D preferred_extent, VkExtent2D awindow_extent,
            VkSampleCountFlagBits amsaa_sample_count,
            VkCompositeAlphaFlagBitsKHR alpha_compositing)
      : queue{std::move(aqueue)} {
    VkPhysicalDevice phy_dev =
        queue.handle->device.handle->phy_device.handle->phy_device;
    VkDevice dev = queue.handle->device.handle->device;

    // the properties change every time we need to create a swapchain so we must
    // query for this every time
    SwapChainProperties properties =
        get_swapchain_properties(phy_dev, target_surface);

    ASR_LOG("Device Supported Surface Formats:");
    for (VkSurfaceFormatKHR const& format : properties.supported_formats) {
      ASR_LOG("\tFormat: {}, Color Space: {}", string_VkFormat(format.format),
              string_VkColorSpaceKHR(format.colorSpace));
    }

    // swapchain formats are device-dependent
    VkSurfaceFormatKHR selected_format = select_swapchain_surface_formats(
        properties.supported_formats, preferred_formats);

    spdlog::info(
        "selected swapchain surface format: [format: {}, color space: {}]",
        string_VkFormat(selected_format.format),
        string_VkColorSpaceKHR(selected_format.colorSpace));

    spdlog::info("Available swapchain presentation modes:");

    // TODO(lamarrr): log selections
    // swapchain presentation modes are device-dependent
    VkPresentModeKHR selected_present_mode = select_swapchain_presentation_mode(
        properties.presentation_modes, preferred_present_modes);

    spdlog::info("selected swapchain presentation mode: {}",
                 string_VkPresentModeKHR(selected_present_mode));

    auto [new_swapchain, new_extent] = create_swapchain(
        dev, target_surface, preferred_extent, selected_format,
        selected_present_mode, properties,
        // not thread-safe since GPUs typically have one graphics queue
        VK_SHARING_MODE_EXCLUSIVE,
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
    images = get_swapchain_images(dev, swapchain);
    color_format = selected_format;
    depth_format = find_depth_format(phy_dev);
    present_mode = selected_present_mode;
    image_extent = new_extent;
    window_extent = awindow_extent;
    msaa_sample_count = amsaa_sample_count;
    msaa_color_image = create_msaa_color_resource(
        dev, queue.handle->device.handle->phy_device.handle->memory_properties,
        color_format.format, new_extent, msaa_sample_count);
    msaa_depth_image = create_msaa_depth_resource(
        dev, queue.handle->device.handle->phy_device.handle->memory_properties,
        depth_format, new_extent, msaa_sample_count);

    for (VkImage image : images) {
      VkImageViewCreateInfo create_info{
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .image = image,
          .viewType = VK_IMAGE_VIEW_TYPE_2D,
          .format = color_format.format,
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

    VkAttachmentDescription color_attachment{
        .flags = 0,
        .format = color_format.format,
        .samples = msaa_sample_count,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentDescription depth_attachment{
        .flags = 0,
        .format = depth_format,
        .samples = msaa_sample_count,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentDescription color_attachment_resolve{
        .flags = 0,
        .format = color_format.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    VkAttachmentDescription attachments[] = {color_attachment, depth_attachment,
                                             color_attachment_resolve};

    VkAttachmentReference color_attachment_reference{
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depth_attachment_reference{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

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
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};

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
          .width = image_extent.width,
          .height = image_extent.height,
          .layers = 1};

      ASR_VK_CHECK(
          vkCreateFramebuffer(dev, &create_info, nullptr, &framebuffer));

      framebuffers.push_inplace(framebuffer).unwrap();
    }

    for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      VkSemaphoreCreateInfo semaphore_create_info{
          .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0};

      VkSemaphore rendering_semaphore;

      ASR_VK_CHECK(vkCreateSemaphore(dev, &semaphore_create_info, nullptr,
                                     &rendering_semaphore));

      rendering_semaphores.push_inplace(rendering_semaphore).unwrap();

      VkSemaphore image_acquisition_semaphore;

      ASR_VK_CHECK(vkCreateSemaphore(dev, &semaphore_create_info, nullptr,
                                     &image_acquisition_semaphore));

      image_acquisition_semaphores.push_inplace(image_acquisition_semaphore)
          .unwrap();

      VkFenceCreateInfo image_acquisition_fence_create_info{
          .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0};

      VkFence image_acquisition_fence;

      ASR_VK_CHECK(vkCreateFence(dev, &image_acquisition_fence_create_info,
                                 nullptr, &image_acquisition_fence));

      image_acquisition_fences.push_inplace(image_acquisition_fence).unwrap();

      VkFenceCreateInfo rendering_fence_create_info{
          .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
          .pNext = nullptr,
          .flags = VK_FENCE_CREATE_SIGNALED_BIT};

      VkFence rendering_fence;

      ASR_VK_CHECK(vkCreateFence(dev, &rendering_fence_create_info, nullptr,
                                 &rendering_fence));

      rendering_fences.push_inplace(rendering_fence).unwrap();
    }
  }

  void destroy() {
    VkDevice dev = queue.handle->device.handle->device;

    // await idleness of the semaphores device, so we can destroy the
    // semaphore and images whilst not in use.
    // any part of the device could be using the semaphore

    ASR_VK_CHECK(vkDeviceWaitIdle(dev));

    vkDestroyRenderPass(dev, render_pass, nullptr);

    msaa_color_image.destroy(dev);
    msaa_depth_image.destroy(dev);

    for (VkFramebuffer framebuffer : framebuffers) {
      vkDestroyFramebuffer(dev, framebuffer, nullptr);
    }

    for (VkFence fence : rendering_fences) {
      vkDestroyFence(dev, fence, nullptr);
    }

    for (VkFence fence : image_acquisition_fences) {
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

    // swapchain images are automatically deleted along with the swapchain
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
  stx::Option<SwapChain> swapchain;

  stx::Rc<Instance*> instance;

  Surface(stx::Rc<Instance*> ainstance, VkSurfaceKHR asurface)
      : surface{asurface}, instance{std::move(ainstance)} {}

  STX_MAKE_PINNED(Surface)

  ~Surface() {
    // we need to ensure the swapchain is destroyed before the surface (if not
    // already destroyed)
    if (swapchain.is_some()) {
      swapchain.value().destroy();
    }

    vkDestroySurfaceKHR(instance.handle->instance, surface, nullptr);
  }

  void change_swapchain(
      stx::Rc<CommandQueue*> const& queue,
      stx::Span<VkSurfaceFormatKHR const> preferred_formats,
      stx::Span<VkPresentModeKHR const> preferred_present_modes,
      VkExtent2D preferred_extent, VkExtent2D window_extent,
      VkSampleCountFlagBits msaa_sample_count,
      VkCompositeAlphaFlagBitsKHR alpha_compositing) {
    // don't want to have two existing at once
    if (swapchain.is_some()) {
      swapchain.value().destroy();
      swapchain = stx::None;
    }

    swapchain = stx::Some(SwapChain{
        queue.share(), surface, preferred_formats, preferred_present_modes,
        preferred_extent, window_extent, msaa_sample_count, alpha_compositing});
  }
};

struct PushConstants {
  mat4 transform = mat4::identity();
  vec4 overlay{0, 0, 0, 0};
};

struct Pipeline {
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkPipelineLayout layout = VK_NULL_HANDLE;
  VkRenderPass target_render_pass = VK_NULL_HANDLE;
  VkSampleCountFlagBits msaa_sample_count = VK_SAMPLE_COUNT_1_BIT;

  void build(
      VkDevice dev, VkShaderModule vertex_shader,
      VkShaderModule fragment_shader, VkRenderPass target_render_pass,
      VkSampleCountFlagBits amsaa_sample_count,
      stx::Span<VkDescriptorSetLayout const> descriptor_set_layout,
      stx::Span<VkVertexInputAttributeDescription const> vertex_input_attr,
      u32 vertex_input_size) {
    msaa_sample_count = amsaa_sample_count;

    VkPipelineShaderStageCreateInfo vert_shader_stage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo frag_shader_stage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo stages[] = {vert_shader_stage,
                                                frag_shader_stage};

    static_assert(sizeof(PushConstants) % 4 == 0);

    VkPushConstantRange push_constant{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(PushConstants)};

    VkPipelineLayoutCreateInfo layout_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = AS_U32(descriptor_set_layout.size()),
        .pSetLayouts = descriptor_set_layout.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant};

    ASR_VK_CHECK(
        vkCreatePipelineLayout(dev, &layout_create_info, nullptr, &layout));

    VkPipelineColorBlendAttachmentState color_blend_attachment_states[] = {
        {.blendEnable = VK_TRUE,
         .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
         .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
         .colorBlendOp = VK_BLEND_OP_ADD,
         .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
         .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
         .alphaBlendOp = VK_BLEND_OP_ADD,
         .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT |
                           VK_COLOR_COMPONENT_A_BIT}};

    VkPipelineColorBlendStateCreateInfo color_blend_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = AS_U32(std::size(color_blend_attachment_states)),
        .pAttachments = color_blend_attachment_states,
        .blendConstants = {0, 0, 0, 0}};

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
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
                                 .compareOp = VK_COMPARE_OP_ALWAYS,
                                 .compareMask = 0,
                                 .writeMask = 0,
                                 .reference = 0},
        .minDepthBounds = 0,
        .maxDepthBounds = 1};

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
        .minSampleShading = 1,
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
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0,
        .depthBiasClamp = 0,
        .depthBiasSlopeFactor = 0,
        .lineWidth = 1};

    VkVertexInputBindingDescription vertex_binding_descriptions[] = {
        {.binding = 0,
         .stride = vertex_input_size,
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

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                       VK_DYNAMIC_STATE_SCISSOR};

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
        .basePipelineIndex = 0};

    ASR_VK_CHECK(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &create_info,
                                           nullptr, &pipeline));
  }

  void destroy(VkDevice dev) {
    vkDestroyPipelineLayout(dev, layout, nullptr);
    vkDestroyPipeline(dev, pipeline, nullptr);
  }
};

struct DescriptorPoolInfo {
  stx::Vec<VkDescriptorPoolSize> sizes;
  u32 max_sets = 0;
};

struct RecordingContext {
  VkCommandPool cmd_pool = VK_NULL_HANDLE;
  stx::Vec<VkCommandBuffer> draw_cmd_buffers{stx::os_allocator};
  VkCommandBuffer upload_cmd_buffer = VK_NULL_HANDLE;
  VkShaderModule vertex_shader = VK_NULL_HANDLE;
  VkShaderModule fragment_shader = VK_NULL_HANDLE;
  VkFence upload_fence = VK_NULL_HANDLE;
  Pipeline pipeline{};
  // one descriptor pool per frame in flight
  stx::Vec<VkDescriptorPool> descriptor_pools{stx::os_allocator};
  stx::Vec<DescriptorPoolInfo> descriptor_pool_infos{stx::os_allocator};
  // specifications describing binding types/layouts for the descriptor sets
  // used. we will have multiple of each
  stx::Vec<DescriptorSetSpec> descriptor_set_specs{stx::os_allocator};
  // the created layouts for each of the descriptor sets
  stx::Vec<VkDescriptorSetLayout> descriptor_set_layouts{stx::os_allocator};
  // the allocated descriptor sets, the first vec is for each frame in flight
  // and the second vec contains the descriptor sets repeated for each of the
  // draw calls. i.e. num_draw_calls x num_descriptor_sets_per_frame
  stx::Vec<stx::Vec<VkDescriptorSet>> descriptor_sets{stx::os_allocator};
  stx::Vec<VkVertexInputAttributeDescription> vertex_input_attr{
      stx::os_allocator};
  u32 vertex_input_size = 0;

  void init(
      CommandQueue const& queue, stx::Span<u32 const> vertex_shader_code,
      stx::Span<u32 const> fragment_shader_code,
      stx::Span<VkVertexInputAttributeDescription const> avertex_input_attr,
      u32 avertex_input_size,
      stx::Span<DescriptorSetSpec> adescriptor_sets_specs,
      stx::Span<VkDescriptorPoolSize const> adescriptor_pool_sizes,
      u32 max_descriptor_sets) {
    VkDevice dev = queue.device.handle->device;

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

    VkCommandPoolCreateInfo cmd_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue.info.family.index};

    ASR_VK_CHECK(
        vkCreateCommandPool(dev, &cmd_pool_create_info, nullptr, &cmd_pool));

    VkCommandBufferAllocateInfo cmd_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    ASR_VK_CHECK(vkAllocateCommandBuffers(dev, &cmd_buffer_allocate_info,
                                          &upload_cmd_buffer));

    VkFenceCreateInfo fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0};

    ASR_VK_CHECK(
        vkCreateFence(dev, &fence_create_info, nullptr, &upload_fence));

    vertex_input_attr.extend(avertex_input_attr).unwrap();
    vertex_input_size = avertex_input_size;

    descriptor_set_specs.extend_move(adescriptor_sets_specs).unwrap();

    for (DescriptorSetSpec const& spec : descriptor_set_specs) {
      stx::Vec<VkDescriptorSetLayoutBinding> bindings{stx::os_allocator};

      u32 ibinding = 0;

      for (DescriptorType type : spec.bindings) {
        VkDescriptorType binding_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        switch (type) {
          case DescriptorType::UniformBuffer:
            binding_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
          case DescriptorType::CombinedImageSampler:
            binding_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
          default:
            ASR_UNREACHABLE();
        }

        VkDescriptorSetLayoutBinding binding{
            .binding = ibinding,
            .descriptorType = binding_type,
            .descriptorCount = 1,
            .stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};

        bindings.push_inplace(binding).unwrap();
        ibinding++;
      }

      VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{
          .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .bindingCount = AS_U32(bindings.size()),
          .pBindings = bindings.data()};

      VkDescriptorSetLayout descriptor_set_layout;

      ASR_VK_CHECK(
          vkCreateDescriptorSetLayout(dev, &descriptor_set_layout_create_info,
                                      nullptr, &descriptor_set_layout));

      descriptor_set_layouts.push_inplace(descriptor_set_layout).unwrap();
    }

    draw_cmd_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT).unwrap();

    VkCommandBufferAllocateInfo cmd_buffers_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = SwapChain::MAX_FRAMES_IN_FLIGHT};

    ASR_VK_CHECK(vkAllocateCommandBuffers(dev, &cmd_buffers_allocate_info,
                                          draw_cmd_buffers.data()));

    for (u32 i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
      stx::Vec<VkDescriptorPoolSize> pool_sizes{stx::os_allocator};
      pool_sizes.extend(adescriptor_pool_sizes).unwrap();

      VkDescriptorPoolCreateInfo descriptor_pool_create_info{
          .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
          .pNext = nullptr,
          .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
          .maxSets = max_descriptor_sets,
          .poolSizeCount = AS_U32(adescriptor_pool_sizes.size()),
          .pPoolSizes = adescriptor_pool_sizes.data()};

      VkDescriptorPool descriptor_pool;

      ASR_VK_CHECK(vkCreateDescriptorPool(dev, &descriptor_pool_create_info,
                                          nullptr, &descriptor_pool));

      descriptor_pools.push_inplace(descriptor_pool).unwrap();

      descriptor_pool_infos
          .push(DescriptorPoolInfo{
              .sizes = std::move(pool_sizes),
              .max_sets = descriptor_pool_create_info.maxSets})
          .unwrap();
    }

    for (usize i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
      descriptor_sets.push(stx::Vec<VkDescriptorSet>{stx::os_allocator})
          .unwrap();
    }
  }

  void on_swapchain_changed(VkDevice dev, SwapChain const& swapchain) {
    pipeline.build(dev, vertex_shader, fragment_shader, swapchain.render_pass,
                   swapchain.msaa_sample_count, descriptor_set_layouts,
                   vertex_input_attr, vertex_input_size);
  }

  void destroy(VkDevice dev) {
    vkDestroyShaderModule(dev, vertex_shader, nullptr);
    vkDestroyShaderModule(dev, fragment_shader, nullptr);

    vkFreeCommandBuffers(dev, cmd_pool, SwapChain::MAX_FRAMES_IN_FLIGHT,
                         draw_cmd_buffers.data());
    vkFreeCommandBuffers(dev, cmd_pool, 1, &upload_cmd_buffer);

    vkDestroyFence(dev, upload_fence, nullptr);

    vkDestroyCommandPool(dev, cmd_pool, nullptr);

    for (VkDescriptorSetLayout layout : descriptor_set_layouts) {
      vkDestroyDescriptorSetLayout(dev, layout, nullptr);
    }

    u32 frame_index = 0;
    for (stx::Vec<VkDescriptorSet> const& set : descriptor_sets) {
      vkFreeDescriptorSets(dev, descriptor_pools[frame_index],
                           AS_U32(set.size()), set.data());
      frame_index++;
    }

    for (VkDescriptorPool descriptor_pool : descriptor_pools) {
      vkDestroyDescriptorPool(dev, descriptor_pool, nullptr);
    }

    pipeline.destroy(dev);
  }

  stx::Rc<ImageResource*> upload_image(stx::Rc<CommandQueue*> const& queue,
                                       ImageDimensions dimensions,
                                       stx::Span<u8 const> data) {
    VkDevice dev = queue.handle->device.handle->device;

    VkPhysicalDeviceMemoryProperties const& memory_properties =
        queue.handle->device.handle->phy_device.handle->memory_properties;

    ASR_CHECK(data.size_bytes() == dimensions.size());
    ASR_CHECK(dimensions.nchannels == 4,
              "only 4-channel images presently supported");

    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

    if (dimensions.nchannels == 4) {
    } else if (dimensions.nchannels == 3) {
      format = VK_FORMAT_R8G8B8_SRGB;
    } else if (dimensions.nchannels == 1) {
      format = VK_FORMAT_R8_SRGB;
    } else {
      ASR_PANIC("image channels must either be 1, 3, or 4");
    }

    VkImageCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = VkExtent3D{.width = dimensions.width,
                             .height = dimensions.height,
                             .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

    VkImage image;

    ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

    VkMemoryRequirements memory_requirements;

    vkGetImageMemoryRequirements(dev, image, &memory_requirements);

    u32 memory_type_index =
        find_suitable_memory_type(memory_properties, memory_requirements,
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
        .format = format,
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

    Buffer staging_buffer =
        create_host_buffer(dev, memory_properties, dimensions.size(),
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    std::memcpy(staging_buffer.memory_map, data.data(), dimensions.size());

    VkCommandBufferBeginInfo cmd_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr};

    ASR_VK_CHECK(
        vkBeginCommandBuffer(upload_cmd_buffer, &cmd_buffer_begin_info));

    VkImageMemoryBarrier pre_upload_barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_NONE_KHR,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange =
            VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .baseMipLevel = 0,
                                    .levelCount = 1,
                                    .baseArrayLayer = 0,
                                    .layerCount = 1}};

    vkCmdPipelineBarrier(upload_cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, nullptr, 1,
                         &pre_upload_barrier);

    VkBufferImageCopy copy{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            VkImageSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .mipLevel = 0,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1},
        .imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},
        .imageExtent = VkExtent3D{.width = dimensions.width,
                                  .height = dimensions.height,
                                  .depth = 1}};

    vkCmdCopyBufferToImage(upload_cmd_buffer, staging_buffer.buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    VkImageMemoryBarrier post_upload_barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange =
            VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .baseMipLevel = 0,
                                    .levelCount = 1,
                                    .baseArrayLayer = 0,
                                    .layerCount = 1}};

    vkCmdPipelineBarrier(upload_cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                         &post_upload_barrier);

    ASR_VK_CHECK(vkEndCommandBuffer(upload_cmd_buffer));

    VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                             .pNext = nullptr,
                             .waitSemaphoreCount = 0,
                             .pWaitSemaphores = nullptr,
                             .pWaitDstStageMask = nullptr,
                             .commandBufferCount = 1,
                             .pCommandBuffers = &upload_cmd_buffer,
                             .signalSemaphoreCount = 0,
                             .pSignalSemaphores = nullptr};

    ASR_VK_CHECK(vkResetFences(dev, 1, &upload_fence));

    ASR_VK_CHECK(
        vkQueueSubmit(queue.handle->info.queue, 1, &submit_info, upload_fence));

    ASR_VK_CHECK(
        vkWaitForFences(dev, 1, &upload_fence, VK_TRUE, COMMAND_TIMEOUT));

    ASR_VK_CHECK(vkResetCommandBuffer(upload_cmd_buffer, 0));

    staging_buffer.destroy(dev);

    return stx::rc::make_inplace<ImageResource>(stx::os_allocator, image, view,
                                                memory, queue.share())
        .unwrap();
  }

  stx::Rc<ImageResource*> upload_font(stx::Rc<CommandQueue*> const& queue,
                                      extent extent, stx::Span<u8 const> data,
                                      color color) {
    usize target_size = static_cast<usize>(extent.w) * extent.h * 4;
    stx::Memory mem =
        stx::mem::allocate(stx::os_allocator, target_size).unwrap();
    u8* pixels = static_cast<u8*>(mem.handle);

    for (usize i = 0; i < target_size; i += 4) {
      pixels[i] = color.r;
      pixels[i + 1] = color.g;
      pixels[i + 2] = color.b;
      pixels[i + 3] = AS_U8((color.a / 255.0f) * data[i / 4]);
    }

    return upload_image(
        queue,
        ImageDimensions{.width = extent.w, .height = extent.h, .nchannels = 4},
        stx::Span{pixels, target_size});
  }
};

}  // namespace vk
}  // namespace asr
