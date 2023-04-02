#pragma once

#include <chrono>

#include "ashura/app_config.h"
#include "ashura/asset_bundle.h"
#include "ashura/canvas.h"
#include "ashura/plugins/vulkan_image_bundle.h"
#include "ashura/version.h"
#include "ashura/vulkan.h"
#include "ashura/vulkan_canvas_renderer.h"
#include "ashura/widget.h"
#include "ashura/widget_system.h"
#include "ashura/window.h"
#include "ashura/window_api.h"
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

  stx::Option<stx::Rc<spdlog::logger *>>    logger;
  stx::Option<stx::Rc<WindowApi *>>         window_api;
  stx::Option<stx::Rc<Window *>>            window;
  stx::Option<stx::Rc<vk::CommandQueue *>>  queue;
  gfx::Canvas                               canvas;
  vk::CanvasRenderer                        renderer;
  vk::ImageManager                          manager;
  stx::TaskScheduler                        task_scheduler;
  WidgetContext                             widget_context;
  Widget                                   *root_widget = nullptr;
  WidgetSystem                              widget_system;

  void tick(std::chrono::nanoseconds interval);
};

}        // namespace ash
