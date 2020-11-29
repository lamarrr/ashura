
#pragma once

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

#include "stx/result.h"
#include "stx/span.h"
#include "vulkan/vulkan.h"

#include "vlk/utils.h"

#ifndef NDEBUG
#define VLK_DEBUG true
#else
#define VLK_DEBUG false
#endif

#if VLK_DEBUG

static void ensure_validation_layers_supported(
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
    VLK_LOG("\t" << layer.layerName << ", version: " << layer.specVersion);
  }

  for (auto const req_layer : required_validation_layers) {
    if (std::find_if(available_validation_layers.begin(),
                     available_validation_layers.end(),
                     [&req_layer](VkLayerProperties const& available_layer) {
                       return std::string_view(req_layer) ==
                              std::string_view(available_layer.layerName);
                     }) == available_validation_layers.end()) {
      VLK_LOG("Required validation layer: " << req_layer
                                            << " is not supported");
    }
  }
}

static VkBool32 VKAPI_ATTR VKAPI_CALL default_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
    void* user_data) {
  // Message is important enough to show
  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_*_BIT_EXT are bit flags

  // you can use comparisions like messageSeverity >=
  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT to see if they are
  // important o not

  (void)user_data;
  (void)message_severity;

  if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    VLK_LOG(
        "[Validation Layer Hint] Specification violation or possible mistake "
        "detected");
  }

  if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
    VLK_LOG(
        "[Validation Layer Hint] Potential non-optimal use of Vulkan "
        "detected");
  }

  VLK_LOG("[Validation Layer Message] " << callback_data->pMessage);

  return VK_FALSE;
}

static VkDebugUtilsMessengerCreateInfoEXT make_debug_messenger_create_info() {
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

static VkResult create_install_debug_messenger_helper(
    VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const* create_info,
    VkAllocationCallbacks const* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, create_info, allocator, debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

static VkDebugUtilsMessengerEXT create_install_debug_messenger(
    VkInstance instance, VkAllocationCallbacks const* allocator) {
  VkDebugUtilsMessengerEXT debug_messenger;

  auto create_info = make_debug_messenger_create_info();

  if (create_install_debug_messenger_helper(instance, &create_info, allocator,
                                            &debug_messenger) != VK_SUCCESS)
    stx::panic("Failed to setup debug messenger");

  return debug_messenger;
}

static void destroy_debug_messenger(VkInstance instance,
                                    VkDebugUtilsMessengerEXT debug_messenger,
                                    VkAllocationCallbacks const* allocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debug_messenger, allocator);
  } else {
    stx::panic("Failed to destroy debug messenger");
  }
}

#endif
