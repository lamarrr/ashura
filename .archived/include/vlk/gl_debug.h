
#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "stx/backtrace.h"
#include "stx/result.h"
#include "stx/span.h"
#include "vulkan/vulkan.h"

#include "vlk/utils/utils.h"

#ifndef NDEBUG
#define VLK_DEBUG true
#else
#define VLK_DEBUG false
#endif

namespace vlk {

inline void ensure_validation_layers_supported(
    stx::Span<char const* const> required_validation_layers) {
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

  for (auto const req_layer : required_validation_layers) {
    if (std::find_if(available_validation_layers.begin(),
                     available_validation_layers.end(),
                     [&req_layer](VkLayerProperties const& available_layer) {
                       return std::string_view(req_layer) ==
                              std::string_view(available_layer.layerName);
                     }) == available_validation_layers.end()) {
      all_layers_available = false;
      VLK_WARN("Required validation layer is not available",
               std::string_view(req_layer));
    }
  }

  VLK_ENSURE(all_layers_available,
             "One or more required validation layers are not available");
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

}  // namespace vlk
