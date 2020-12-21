#pragma once

#include <string_view>

#include "stx/report.h"
#include "vulkan/vulkan.h"

STX_FORCE_INLINE constexpr std::string_view vk_error_to_string(
    VkResult error, char const* alt = "Unrecognized Error") noexcept {
  switch (error) {
    case VK_SUCCESS:
      return "VK_SUCCESS";
    case VK_NOT_READY:
      return "VK_NOT_READY";
    case VK_TIMEOUT:
      return "VK_TIMEOUT";
    case VK_EVENT_SET:
      return "VK_EVENT_SET";
    case VK_EVENT_RESET:
      return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
      return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
      return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
      return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
      return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
      return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
      return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
      return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:
      return "VK_ERROR_UNKNOWN";

#if defined(VK_VERSION_1_1) && VK_VERSION_1_1
      // Provided by VK_VERSION_1_1
    case VK_ERROR_OUT_OF_POOL_MEMORY:
      return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
#endif

#if defined(VK_VERSION_1_2) && VK_VERSION_1_2
    // Provided by VK_VERSION_1_2
    case VK_ERROR_FRAGMENTATION:
      return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
      return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
#endif

#if defined(VK_KHR_surface) && VK_KHR_surface
      // Provided by VK_KHR_surface
    case VK_ERROR_SURFACE_LOST_KHR:
      return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
#endif

#if defined(VK_KHR_swapchain) && VK_KHR_swapchain
      // Provided by VK_KHR_swapchain
    case VK_SUBOPTIMAL_KHR:
      return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
      return "VK_ERROR_OUT_OF_DATE_KHR";
#endif

#if defined(VK_KHR_display_swapchain) && VK_KHR_display_swapchain
      // Provided by VK_KHR_display_swapchain
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
#endif

#if defined(VK_EXT_debug_report) && VK_EXT_debug_report
      // Provided by VK_EXT_debug_report
    case VK_ERROR_VALIDATION_FAILED_EXT:
      return "VK_ERROR_VALIDATION_FAILED_EXT";
#endif

#if defined(VK_NV_glsl_shader) && VK_NV_glsl_shader
      // Provided by VK_NV_glsl_shader
    case VK_ERROR_INVALID_SHADER_NV:
      return "VK_ERROR_INVALID_SHADER_NV";
#endif

#if defined(VK_EXT_image_drm_format_modifier) && \
    VK_EXT_image_drm_format_modifier
      // Provided by VK_EXT_image_drm_format_modifier
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
      return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
#endif

#if defined(VK_EXT_global_priority) && VK_EXT_global_priority
      // Provided by VK_EXT_global_priority
    case VK_ERROR_NOT_PERMITTED_EXT:
      return "VK_ERROR_NOT_PERMITTED_EXT";
#endif

#if defined(VK_EXT_full_screen_exclusive) && VK_EXT_full_screen_exclusive
      // Provided by VK_EXT_full_screen_exclusive
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
      return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
#endif

#if defined(VK_KHR_deferred_host_operations) && VK_KHR_deferred_host_operations
      // Provided by VK_KHR_deferred_host_operations
    case VK_THREAD_IDLE_KHR:
      return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:
      return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:
      return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:
      return "VK_OPERATION_NOT_DEFERRED_KHR";
#endif

#if defined(VK_EXT_pipeline_creation_cache_control) && \
    VK_EXT_pipeline_creation_cache_control
      // Provided by VK_EXT_pipeline_creation_cache_control
    case VK_PIPELINE_COMPILE_REQUIRED_EXT:
      return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
#endif

    default:
      return alt;
  }
}

// TODO(lamarrr): must return a span of any type. would be converted to bytes
// and sent
STX_FORCE_INLINE stx::SpanReport operator>>(stx::ReportQuery,
                                            VkResult const& result) noexcept {
  return stx::SpanReport(vk_error_to_string(result));
}
