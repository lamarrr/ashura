#pragma once

#include <string_view>

#include "stx/report.h"
#include "vulkan/vulkan.h"


// TODO: must return a span of any type. would be converted to bytes and sent
STX_FORCE_INLINE stx::SpanReport operator<<(stx::ReportQuery const &query, VkResult result)
{
    using namespace stx;
    switch (result)
    {
    case VK_SUCCESS:
        return SpanReport("VK_SUCCESS");
    case VK_NOT_READY:
        return SpanReport("VK_NOT_READY");
    case VK_TIMEOUT:
        return SpanReport("VK_TIMEOUT");
    case VK_EVENT_SET:
        return SpanReport("VK_EVENT_SET");
    case VK_EVENT_RESET:
        return SpanReport("VK_EVENT_RESET");
    case VK_INCOMPLETE:
        return SpanReport("VK_INCOMPLETE");
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return SpanReport("VK_ERROR_OUT_OF_HOST_MEMORY");
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return SpanReport("VK_ERROR_OUT_OF_DEVICE_MEMORY");
    case VK_ERROR_INITIALIZATION_FAILED:
        return SpanReport("VK_ERROR_INITIALIZATION_FAILED");
    case VK_ERROR_DEVICE_LOST:
        return SpanReport("VK_ERROR_DEVICE_LOST");
    case VK_ERROR_MEMORY_MAP_FAILED:
        return SpanReport("VK_ERROR_MEMORY_MAP_FAILED");
    case VK_ERROR_LAYER_NOT_PRESENT:
        return SpanReport("VK_ERROR_LAYER_NOT_PRESENT");
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return SpanReport("VK_ERROR_EXTENSION_NOT_PRESENT");
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return SpanReport("VK_ERROR_FEATURE_NOT_PRESENT");
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return SpanReport("VK_ERROR_INCOMPATIBLE_DRIVER");
    case VK_ERROR_TOO_MANY_OBJECTS:
        return SpanReport("VK_ERROR_TOO_MANY_OBJECTS");
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return SpanReport("VK_ERROR_FORMAT_NOT_SUPPORTED");
    case VK_ERROR_FRAGMENTED_POOL:
        return SpanReport("VK_ERROR_FRAGMENTED_POOL");
    case VK_ERROR_SURFACE_LOST_KHR:
        return SpanReport("VK_ERROR_SURFACE_LOST_KHR");
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return SpanReport("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR");
    case VK_SUBOPTIMAL_KHR:
        return SpanReport("VK_SUBOPTIMAL_KHR");
    case VK_ERROR_OUT_OF_DATE_KHR:
        return SpanReport("VK_ERROR_OUT_OF_DATE_KHR");
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return SpanReport("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR");
    case VK_ERROR_VALIDATION_FAILED_EXT:
        return SpanReport("VK_ERROR_VALIDATION_FAILED_EXT");
    case VK_ERROR_INVALID_SHADER_NV:
        return SpanReport("VK_ERROR_INVALID_SHADER_NV");
    case VK_RESULT_RANGE_SIZE:
        return SpanReport("VK_RESULT_RANGE_SIZE");
    case VK_RESULT_MAX_ENUM:
        return SpanReport("VK_RESULT_MAX_ENUM");
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return SpanReport("VK_ERROR_INVALID_EXTERNAL_HANDLE");
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return SpanReport("VK_ERROR_OUT_OF_POOL_MEMORY");
    default:
        return SpanReport("Unknown Error");
    }
}

STX_FORCE_INLINE constexpr std::string_view vk_error_to_string(VkResult error, char const * alt = "Unrecognized Error"){
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
	
	// Provided by VK_VERSION_1_1
    case VK_ERROR_OUT_OF_POOL_MEMORY:
    return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
    return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	
	
	// Provided by VK_VERSION_1_2  
	case VK_ERROR_FRAGMENTATION:
    return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
    return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
	
	// Provided by VK_KHR_surface
    case VK_ERROR_SURFACE_LOST_KHR:
    return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
    return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	
	
  	// Provided by VK_KHR_swapchain
    case VK_SUBOPTIMAL_KHR:
    return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
    return "VK_ERROR_OUT_OF_DATE_KHR";
	
	
    //case :
    // return "";
    //case :
    // return "";
    default:
    return alt;
}
/*
  
  // Provided by VK_KHR_swapchain
    VK_ERROR_OUT_OF_DATE_KHR = -1000001004,
  // Provided by VK_KHR_display_swapchain
    VK_ERROR_INCOMPATIBLE_DISPLAY_KHR = -1000003001,
  // Provided by VK_EXT_debug_report
    VK_ERROR_VALIDATION_FAILED_EXT = -1000011001,
  // Provided by VK_NV_glsl_shader
    VK_ERROR_INVALID_SHADER_NV = -1000012000,
  // Provided by VK_EXT_image_drm_format_modifier
    VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT = -1000158000,
  // Provided by VK_EXT_global_priority
    VK_ERROR_NOT_PERMITTED_EXT = -1000174001,
  // Provided by VK_EXT_full_screen_exclusive
    VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT = -1000255000,
  // Provided by VK_KHR_deferred_host_operations
    VK_THREAD_IDLE_KHR = 1000268000,
  // Provided by VK_KHR_deferred_host_operations
    VK_THREAD_DONE_KHR = 1000268001,
  // Provided by VK_KHR_deferred_host_operations
    VK_OPERATION_DEFERRED_KHR = 1000268002,
  // Provided by VK_KHR_deferred_host_operations
    VK_OPERATION_NOT_DEFERRED_KHR = 1000268003,
  // Provided by VK_EXT_pipeline_creation_cache_control
    VK_PIPELINE_COMPILE_REQUIRED_EXT = 1000297000,
  // Provided by VK_KHR_maintenance1
    VK_ERROR_OUT_OF_POOL_MEMORY_KHR = VK_ERROR_OUT_OF_POOL_MEMORY,
  // Provided by VK_KHR_external_memory
    VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR = VK_ERROR_INVALID_EXTERNAL_HANDLE,
  // Provided by VK_EXT_descriptor_indexing
    VK_ERROR_FRAGMENTATION_EXT = VK_ERROR_FRAGMENTATION,
  // Provided by VK_EXT_buffer_device_address
    VK_ERROR_INVALID_DEVICE_ADDRESS_EXT = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
  // Provided by VK_KHR_buffer_device_address
    VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
    VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT = VK_PIPELINE_COMPILE_REQUIRED_EXT,
   */
