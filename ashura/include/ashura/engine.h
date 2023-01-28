#pragma once

#include <chrono>

#include "ashura/app_config.h"
#include "ashura/asset_bundle.h"
#include "ashura/canvas.h"
#include "ashura/version.h"
#include "ashura/vulkan.h"
#include "ashura/window.h"
#include "ashura/window_api.h"
#include "spdlog/logger.h"
#include "stx/rc.h"
#include "stx/string.h"
#include "ashura/vulkan_canvas_renderer.h"

namespace asr {

struct Engine {
  Engine(AppConfig const& cfg);

  stx::Option<stx::Rc<spdlog::logger*>> logger;
  stx::Option<stx::Rc<WindowApi*>> window_api;
  stx::Option<stx::Rc<Window*>> window;
  stx::Option<stx::Rc<vk::CommandQueue*>> queue;
  stx::Option<gfx::Canvas> canvas;
  stx::Option<stx::Rc<vk::CanvasRenderer*>> renderer;
  AssetBundle<stx::Rc<vk::ImageSampler*>> image_bundle;

  // asset manager
  // plugins & systems

  void tick(std::chrono::nanoseconds interval);
};

}  // namespace asr