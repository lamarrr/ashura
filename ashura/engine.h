#pragma once

#include <chrono>

#include "ashura/app_config.h"
#include "ashura/canvas.h"
#include "ashura/clipboard.h"
#include "ashura/font.h"
#include "ashura/pipeline.h"
#include "ashura/primitives.h"
#include "ashura/subsystems/vulkan_image_manager.h"
#include "ashura/utils.h"
#include "ashura/uuid.h"
#include "ashura/vulkan.h"
#include "ashura/vulkan_canvas_renderer.h"
#include "ashura/widget.h"
#include "ashura/widget_system.h"
#include "ashura/window.h"
#include "spdlog/logger.h"
#include "stx/c_string_view.h"
#include "stx/rc.h"
#include "stx/scheduler.h"
#include "stx/string.h"

namespace ash
{

using namespace stx::literals;

// refresh rate and various other settings
// log directories, file outputs, configuration files etc
struct AppConfig
{
  stx::CStringView                    name = "Ashura Engine";
  bool                                enable_validation_layers = false;
  stx::Span<FontSpec const>           fonts;
  stx::Span<CanvasPipelineSpec const> pipelines;
  stx::CStringView                    log_file         = "log.txt";
  WindowType                          root_window_type = WindowType::Normal;
  WindowCreateFlags root_window_create_flags = WindowCreateFlags::None;
  ash::Extent       root_window_extent{1920, 1080};
};

struct Engine
{
  static constexpr u32 DEFAULT_MAX_FRAMES_IN_FLIGHT = 2;

  template <Impl<Widget> DerivedWidget>
  Engine(AppConfig const &cfg, DerivedWidget &&root_widget) :
      Engine{cfg, new DerivedWidget{std::move(root_widget)}}
  {
  }

  Engine(AppConfig const &cfg, Widget *root_widget);

  ~Engine()
  {
    delete root_widget;
    renderer.destroy();
    render_resource_manager.destroy();
  }

  // TODO(lamarrr): move all into engine ctx?
  stx::Rc<PrngUuidGenerator *>             uuid_generator;
  WindowManager                            window_manager;
  stx::Option<stx::Rc<Window *>>           root_window;
  stx::Option<stx::Rc<vk::CommandQueue *>> queue;
  gfx::Canvas                              canvas;
  vk::CanvasRenderer                       renderer;
  vk::RenderResourceManager                render_resource_manager;
  vk::CanvasPipelineManager                pipeline_manager;
  stx::TaskScheduler                       task_scheduler;
  Context                                  ctx;
  Widget                                  *root_widget = nullptr;
  WidgetSystem                             widget_system;
  WidgetTree                               widget_tree;
  ClipBoard                                clipboard;
  stx::Vec<BundledFont>                    font_bundle;

  void tick(std::chrono::nanoseconds interval);
};

}        // namespace ash
