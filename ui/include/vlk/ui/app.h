#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <utility>

#include "vlk/ui/primitives.h"
#include "vlk/utils/utils.h"

namespace vlk {

namespace vk {
struct Instance;
struct CommandQueue;
};  // namespace vk

namespace ui {

// forward declarations here

enum class AppLogTarget : uint8_t { Std = 0, File = 1 };

VLK_DEFINE_ENUM_BIT_OPS(AppLogTarget);

//
//
// Issues: direct context order with app, also given we can't create vulkan
// instance without creating video first
//
//

struct AppCfg {
  AppLogTarget log_target = AppLogTarget::Std;
  std::string name;
  Extent extent = Extent{1920, 1080};
  std::string title = "Valkyrie App";
  bool resizable = true;
  bool maximized = false;
  // borderless
  // fullscreen
  // minimized
};

struct Widget;
struct Window;
struct WindowApi;
struct Pipeline;

struct Version {
  uint8_t major = 0;
  uint8_t minor = 0;
  uint8_t patch = 0;
};

struct EngineCfg {
  char const* name = "Valkyrie Engine";
  Version version = Version{0, 0, 1};
};

struct App {
  VLK_MAKE_PINNED(App)

  App(Widget* widget, AppCfg app_cfg)
      : cfg{std::move(app_cfg)}, root_widget{widget} {
    init();
  }

  virtual void tick(std::chrono::nanoseconds interval) {}

  ~App();

 private:
  void init();

  // nice-to-have: custom vulkan instance, custom window api abstracted by
  // virtual methods?
  std::unique_ptr<WindowApi> window_api;
  std::unique_ptr<Window> window;
  std::unique_ptr<vk::Instance> vk_instance;
  // used for rendering and presentation
  std::unique_ptr<vk::CommandQueue> vk_graphics_command_queue;
  std::unique_ptr<Pipeline> pipeline;
  std::unique_ptr<Widget> root_widget;

  AppCfg cfg;

  static constexpr EngineCfg engine_cfg{};
};

}  // namespace ui
}  // namespace vlk
