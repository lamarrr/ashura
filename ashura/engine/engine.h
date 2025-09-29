/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/systems.h"
#include "ashura/engine/view_system.h"
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

  static Result<EngineCfg> parse(Allocator allocator, Vec<u8> & json);
};

typedef struct Window_T * Window;
typedef struct IEngine *  Engine;

struct IEngine
{
  Allocator allocator;

  struct Systems
  {
    Dyn<Logger>       logger;
    Dyn<Scheduler>    sched;
    Dyn<GpuSys>       gpu;
    Dyn<FileSys>      file;
    Dyn<ImageSys>     image;
    Dyn<FontSys>      font;
    Dyn<ShaderSys>    shader;
    Dyn<WindowSys>    win;
    Dyn<PipelineSys>  pipeline;
    Dyn<AudioSys>     audio;
    Dyn<VideoSys>     video;
    Dyn<AnimationSys> animation;
    Dyn<ViewSys>      view;
  } sys;

  Dyn<gpu::Instance> instance;

  Dyn<gpu::Device> device;

  Window window;

  gpu::Surface surface;

  gpu::PresentMode present_mode_preference;

  Canvas canvas;

  InputState input_state;

  Vec<char> working_dir{};

  Vec<char> pipeline_cache_path{};

  nanoseconds min_frame_interval;

  static Dyn<Engine *> create(Allocator allocator, Str config_path,
                              Str working_dir);

  IEngine() :
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

  IEngine(IEngine const &)            = delete;
  Engine & operator=(IEngine const &) = delete;
  IEngine(IEngine &&)                 = default;
  IEngine & operator=(IEngine &&)     = default;
  ~IEngine()                          = default;

  Systems get_systems()
  {
    return Systems{.file   = file_sys,
                   .gpu    = gpu_sys,
                   .image  = image_sys,
                   .font   = *font_sys,
                   .shader = shader_sys,
                   .window = *window_sys};
  }

  void init(Allocator allocator, EngineCfg const & cfg, Vec<char> working_dir,
            Vec<char> pipeline_cache_path, nanoseconds min_frame_interval);

  void shutdown();

  time_point get_inputs_(time_point prev_frame_end);

  void run(ui::View & view, Fn<void(ui::Ctx const &)> loop = noop);
};

/// Global Engine Pointer. Can be hooked at runtime for dynamically loaded
/// executables.
extern Engine engine;

ASH_C_LINKAGE ASH_DLL_EXPORT void hook_engine(Engine);

}    // namespace ash
