#pragma once

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
