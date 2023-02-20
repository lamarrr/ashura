#pragma once

#include <chrono>

#include "ashura/app_config.h"
#include "ashura/asset_bundle.h"
#include "ashura/canvas.h"
#include "ashura/plugins/vulkan_image_bundle.h"
#include "ashura/version.h"
#include "ashura/vulkan.h"
#include "ashura/vulkan_canvas_renderer.h"
#include "ashura/window.h"
#include "ashura/window_api.h"
#include "spdlog/logger.h"
#include "stx/rc.h"
#include "stx/string.h"


namespace ash {

struct Engine {
  Engine(AppConfig const& cfg);

  ~Engine(){
    renderer.destroy();
    upload_context.destroy();
  }

  stx::Option<stx::Rc<spdlog::logger*>> logger;
  stx::Option<stx::Rc<WindowApi*>> window_api;
  stx::Option<stx::Rc<Window*>> window;
  stx::Option<stx::Rc<vk::CommandQueue*>> queue;
  gfx::Canvas canvas;
  vk::CanvasRenderer renderer;
  vk::UploadContext upload_context;
  AssetBundle<stx::Rc<vk::ImageResource*>> image_bundle;

  // asset manager
  // plugins & systems

  void tick(std::chrono::nanoseconds interval);
};

}  // namespace ash