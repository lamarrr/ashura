#pragma once

#include <chrono>

#include "ashura/app_config.h"
#include "ashura/canvas.h"
#include "ashura/clipboard.h"
#include "ashura/font.h"
#include "ashura/plugins/vulkan_image_manager.h"
#include "ashura/version.h"
#include "ashura/vulkan.h"
#include "ashura/vulkan_canvas_renderer.h"
#include "ashura/widget.h"
#include "ashura/widget_system.h"
#include "ashura/window.h"
#include "spdlog/logger.h"
#include "stx/rc.h"
#include "stx/scheduler.h"
#include "stx/string.h"

namespace ash
{

struct Engine
{
  static constexpr u32 DEFAULT_MAX_FRAMES_IN_FLIGHT = 2;

  Engine(AppConfig const &cfg, Widget *root_widget);

  ~Engine()
  {
    delete root_widget;
    renderer.destroy();
    manager.destroy();
  }

  // TODO(lamarrr): move all into engine ctx?
  WindowManager                            window_manager;
  stx::Option<stx::Rc<Window *>>           root_window;
  stx::Option<stx::Rc<vk::CommandQueue *>> queue;
  gfx::Canvas                              canvas;
  vk::CanvasRenderer                       renderer;
  vk::RenderResourceManager                manager;
  stx::TaskScheduler                       task_scheduler;
  Context                                  ctx;
  Widget                                  *root_widget = nullptr;
  WidgetSystem                             widget_system;
  ClipBoard                                clipboard;
  stx::Vec<BundledFont>                    font_bundle;

  void tick(std::chrono::nanoseconds interval);
};

}        // namespace ash
