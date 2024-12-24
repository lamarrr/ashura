/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/view_system.h"
#include "ashura/engine/window.h"
#include "ashura/std/cfg.h"

namespace ash
{

struct EngineCfg
{
  struct Gpu
  {
    bool                           validation = false;
    bool                           vsync      = true;
    InplaceVec<gpu::DeviceType, 5> preferences{};
    bool                           hdr        = true;
    u32                            buffering  = 2;
    gpu::SampleCount               msaa_level = gpu::SampleCount::C4;
  };

  struct Window
  {
    bool resizable   = true;
    bool maximized   = false;
    bool full_screen = false;
    bool borderless  = false;
    u32  width       = 1'920;
    u32  height      = 1'080;
  };

  Gpu gpu{};

  Window window{};

  Vec<char> default_font{};

  StrVecMap<Vec<char>> shaders{};

  StrVecMap<Vec<char>> fonts{};

  StrVecMap<Vec<char>> images{};

  static EngineCfg parse(AllocatorImpl allocator, Span<u8 const> json);
};

struct Engine
{
  AllocatorImpl allocator;

  void * app;

  Dyn<gpu::Instance *> instance;

  gpu::Device * device;

  Window window;

  ClipBoard * clipboard;

  gpu::Surface surface;

  gpu::Swapchain swapchain = nullptr;

  gpu::PresentMode present_mode_preference;

  Renderer renderer;

  Canvas canvas;

  ViewSystem view_system;

  Vec<char> default_font_name;

  Font * default_font = nullptr;

  InputState input_buffer;

  Engine(AllocatorImpl allocator, void * app, Dyn<gpu::Instance *> instance,
         gpu::Device * device, Window window, ClipBoard & clipboard,
         gpu::Surface surface, gpu::PresentMode present_mode_preference,
         Renderer renderer, Canvas canvas, ViewSystem view_system) :
    allocator{allocator},
    app{app},
    instance{std::move(instance)},
    device{device},
    window{window},
    clipboard{&clipboard},
    surface{surface},
    present_mode_preference{present_mode_preference},
    renderer{std::move(renderer)},
    canvas{std::move(canvas)},
    view_system{std::move(view_system)},
    default_font_name{allocator},
    input_buffer{allocator}
  {
  }

  Engine(Engine const &)             = delete;
  Engine & operator=(Engine const &) = delete;
  Engine(Engine &&)                  = default;
  Engine & operator=(Engine &&)      = default;
  ~Engine();

  static void init(AllocatorImpl allocator, void * app,
                   Span<char const> config_path, Span<char const> assets_dir);

  static void uninit();

  // shutdown

  void recreate_swapchain_();

  void run(View & view, Fn<void(InputState const &)> loop = noop);
};

/// Global Engine Pointer. Can be hooked at runtime for dynamically loaded
/// executables.
ASH_C_LINKAGE ASH_DLL_EXPORT Engine * engine;

}    // namespace ash
