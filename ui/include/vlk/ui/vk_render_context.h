#pragma once

#include "vlk/ui/render_context.h"
#include "vlk/ui/vulkan.h"

namespace vlk {
namespace ui {

// NOTE: command queue contains device, phys_device, graphics queue, and
// instance. this struct ensures they aren't destroyed before the render context
struct VkRenderContext {
  vk::CommandQueue graphics_command_queue;

  /// NOTE: placed in this position to ensure Skia deletes its context before
  /// the vulkan command queue, instance, and logical devices are destroyed
  RenderContext render_context;
};

}  // namespace ui
}  // namespace vlk
