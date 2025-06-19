/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/systems.h"
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
    Option<i64>                    max_fps    = none;
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

  u32 font_height = 64;

  StringDict<Vec<char>> shaders{};

  StringDict<Vec<char>> fonts{};

  StringDict<Vec<char>> images{};

  Vec<char> pipeline_cache{};

  static Result<EngineCfg> parse(AllocatorRef allocator, Vec<u8> & json);
};

struct Engine
{
  AllocatorRef allocator;

  Dyn<Logger *> logger;

  Dyn<Scheduler *> scheduler;

  FileSystem file_sys;

  Dyn<gpu::Instance *> instance;

  ref<gpu::Device> device;

  GpuSystem gpu_sys;

  ImageSystem image_sys;

  Dyn<FontSystem *> font_sys;

  ShaderSystem shader_sys;

  Dyn<WindowSystem *> window_sys;

  Window window;

  ref<ClipBoard> clipboard;

  gpu::Surface surface;

  gpu::Swapchain swapchain = nullptr;

  gpu::PresentMode present_mode_preference;

  Renderer renderer;

  Canvas canvas;

  ui::System ui_sys;

  InputState input_state;

  Vec<char> working_dir{};

  Vec<char> pipeline_cache_path{};

  nanoseconds min_frame_interval;

  static Dyn<Engine *> create(AllocatorRef allocator, Str config_path,
                              Str working_dir);

  Engine(AllocatorRef allocator, Dyn<Logger *> logger,
         Dyn<Scheduler *> scheduler, FileSystem file_sys,
         Dyn<gpu::Instance *> instance, gpu::Device & device, GpuSystem gpu_sys,
         ImageSystem image_sys, Dyn<FontSystem *> font_sys,
         ShaderSystem shader_sys, Dyn<WindowSystem *> window_sys, Window window,
         ClipBoard & clipboard, gpu::Surface surface,
         gpu::PresentMode present_mode_preference, Renderer renderer,
         Canvas canvas, ui::System ui_sys, Vec<char> working_dir,
         Vec<char> pipeline_cache_path, nanoseconds min_frame_interval) :
    allocator{allocator},
    logger{std::move(logger)},
    scheduler{std::move(scheduler)},
    file_sys{std::move(file_sys)},
    instance{std::move(instance)},
    device{device},
    gpu_sys{std::move(gpu_sys)},
    image_sys{std::move(image_sys)},
    font_sys{std::move(font_sys)},
    shader_sys{std::move(shader_sys)},
    window_sys{std::move(window_sys)},
    window{window},
    clipboard{clipboard},
    surface{surface},
    present_mode_preference{present_mode_preference},
    renderer{std::move(renderer)},
    canvas{std::move(canvas)},
    ui_sys{std::move(ui_sys)},
    input_state{allocator},
    working_dir{std::move(working_dir)},
    pipeline_cache_path{std::move(pipeline_cache_path)},
    min_frame_interval{min_frame_interval}
  {
  }

  Engine(Engine const &)             = delete;
  Engine & operator=(Engine const &) = delete;
  Engine(Engine &&)                  = default;
  Engine & operator=(Engine &&)      = default;
  ~Engine()                          = default;

  Systems get_systems()
  {
    return Systems{.file   = file_sys,
                   .gpu    = gpu_sys,
                   .image  = image_sys,
                   .font   = *font_sys,
                   .shader = shader_sys,
                   .window = *window_sys};
  }

  void engage_(EngineCfg const & cfg);

  void shutdown();

  void recreate_swapchain_();

  time_point get_inputs_(time_point prev_frame_end);

  void run(ui::View & view, Fn<void(ui::Ctx const &)> loop = noop);
};

/// Global Engine Pointer. Can be hooked at runtime for dynamically loaded
/// executables.
extern Engine * engine;

ASH_C_LINKAGE ASH_DLL_EXPORT void hook_engine(Engine *);

}    // namespace ash
