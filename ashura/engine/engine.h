/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/gpu_context.h"
#include "ashura/engine/image_decoder.h"
#include "ashura/engine/view_system.h"
#include "ashura/engine/window.h"
#include "ashura/std/async.h"
#include "ashura/std/cfg.h"

namespace ash
{

enum class FontLoadError : i32
{
};

enum class ImageLoadError : i32
{
};

struct EngineCfg
{
  struct Gpu
  {
    bool                           validation = false;
    bool                           vsync      = true;
    InplaceVec<gpu::DeviceType, 5> preferences{};
    bool                           hdr       = true;
    u32                            buffering = 2;
  };

  struct Window
  {
    bool resizable = true;
    bool maximized = false;
    u32  width     = 1920;
    u32  height    = 1080;
  };

  Gpu gpu{};

  Window window{};

  Vec<char> default_font{};

  StrMap<Vec<char>> shaders{};

  StrMap<Vec<char>> fonts{};

  StrMap<Vec<char>> images{};

  static EngineCfg parse(AllocatorImpl allocator, Span<u8 const> json);
};

// [ ] Font Manager
// [ ] Shader Manager
// [ ] Image Manager
// [ ] Graphics Device
// [ ] Render Context : Shader Manager + input
// [ ] Renderer
// [ ] Window Manager? We might need to attach new contexts to new windows
// [ ] Audio Device/Manager (FFMPEG, SDL)
// [ ] Video Manager (Vulkan, FFMPEG)
// [ ] Custom Subsystems
// [ ] Engine Startup() -> Tick () -> Shutdown() Tasks
// [ ] UI tick rate (time-based/adaptive frame rate), with custom frequency
// allowed, need to be able to merge inputs?
//

struct Engine
{
  void *app;

  Dyn<gpu::Instance *> instance;

  gpu::Device *device;

  Window window;

  gpu::Surface surface;

  gpu::Swapchain swapchain = nullptr;

  gpu::PresentMode present_mode_preference;

  GpuContext gpu_ctx;

  Renderer renderer;

  Canvas canvas;

  ViewSystem view_system;

  ViewContext view_ctx;

  AssetMap assets;

  Vec<char> default_font_name;

  Font *default_font = nullptr;

  bool should_shutdown = false;

  Engine(AllocatorImpl allocator, void *app, Dyn<gpu::Instance *> instance,
         gpu::Device *device, Window window, gpu::Surface surface,
         gpu::PresentMode present_mode_preference, GpuContext gpu_ctx,
         Renderer renderer, Canvas canvas, ViewSystem view_system,
         ViewContext view_ctx) :
      app{app},
      instance{std::move(instance)},
      device{device},
      window{window},
      surface{surface},
      present_mode_preference{present_mode_preference},
      gpu_ctx{std::move(gpu_ctx)},
      renderer{std::move(renderer)},
      canvas{std::move(canvas)},
      view_system{std::move(view_system)},
      view_ctx{std::move(view_ctx)},
      assets{allocator},
      default_font_name{allocator}
  {
  }

  ~Engine();

  static void init(AllocatorImpl allocator, void *app,
                   Span<char const> config_path, Span<char const> assets_dir);

  static void uninit();

  void recreate_swapchain_();

  void run(View &view);
};

/// Global Engine Pointer. Can be hooked at runtime for dynamically loaded
/// executables.
ASH_C_LINKAGE ASH_DLL_EXPORT Engine *engine;

}        // namespace ash
