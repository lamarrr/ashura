/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/systems.h"
#include "ashura/engine/view_system.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/cfg.h"
#include "ashura/std/dict.h"
#include "ashura/std/vec.h"

namespace ash
{

enum class WindowId : usize
{
  Undefined = USIZE_MAX
};

struct EngineCfg
{
  struct Gpu
  {
    bool                           validation = false;
    InplaceVec<gpu::DeviceType, 5> preferences{};
    bool                           hdr        = true;
    u32                            buffering  = 2;
    gpu::SampleCount               msaa_level = gpu::SampleCount::C4;
  };

  struct Window
  {
    Vec<char> title;
    bool      resizable   = true;
    bool      maximized   = false;
    bool      full_screen = false;
    bool      borderless  = false;
    bool      vsync       = true;
    u32       width       = 1'920;
    u32       height      = 1'080;
  };

  Gpu gpu{};

  Window window;

  u32 font_height = 64;

  StrVecDict<Vec<char>> font_paths;

  StrVecDict<Vec<char>> image_paths;

  Vec<char> working_dir_path;

  Vec<char> pipeline_cache_path;

  static Result<EngineCfg> parse_json(Span<u8 const> json, Allocator allocator,
                                      Allocator scratch_allocator);

  static Result<EngineCfg> load_json(Str path, Allocator allocator);
};

typedef struct IWindow * Window;
typedef struct IEngine * Engine;

using WindowLoop = Dyn<Fn<ui::View &(Engine, ui::Scope const &)>>;

struct IEngine
{
  struct Systems
  {
    Dyn<Logger>       logger;
    Dyn<Scheduler>    sched;
    Dyn<FileSys>      file;
    Dyn<GpuSys>       gpu;
    Dyn<ImageSys>     image;
    Dyn<FontSys>      font;
    Dyn<ShaderSys>    shader;
    Dyn<WindowSys>    win;
    Dyn<PipelineSys>  pipeline;
    Dyn<AudioSys>     audio;
    Dyn<VideoSys>     video;
    Dyn<AnimationSys> animation;
  };

  struct WindowEntry
  {
    Engine engine_;

    Window win_ = nullptr;

    WindowState state_;

    gpu::Surface surface_ = nullptr;

    Option<gpu::ISwapchain &> swapchain_ = none;

    Dyn<ViewSys> view_sys_{nullptr, dyn_noop};

    ICanvas canvas_;

    WindowLoop loop_{nullptr, dyn_noop};

    gpu::PresentMode present_mode_preference_ = gpu::PresentMode::Fifo;

    WindowEntry(IEngine & engine, Allocator allocator) :
      engine_{&engine},
      state_{allocator},
      canvas_{allocator}
    {
    }

    WindowEntry(WindowEntry const &)             = delete;
    WindowEntry & operator=(WindowEntry const &) = delete;
    WindowEntry(WindowEntry &&)                  = delete;
    WindowEntry & operator=(WindowEntry &&)      = delete;
    ~WindowEntry()                               = default;
  };

  struct Paths
  {
    Vec<char> working_dir;

    Vec<char> pipeline_cache;
  };

  struct Callbacks
  {
    Dyn<Fn<void(Engine)>> post_init;

    Dyn<Fn<void(Engine)>> pre_shutdown;

    Dyn<Fn<void(Engine)>> post_shutdown;
  };

  Allocator allocator_;

  Systems sys_;

  gpu::Instance gpu_instance_;

  gpu::Device gpu_device_;

  u32 buffering_;

  SystemState state_;

  Paths paths_;

  Callbacks callbacks_;

  Dyn<WindowEntry *> window_;

  IEngine() :
    allocator_{
      noop_allocator
  },
    sys_{.logger{nullptr, dyn_noop},
         .sched{nullptr, dyn_noop},
         .file{nullptr, dyn_noop},
         .gpu{nullptr, dyn_noop},
         .image{nullptr, dyn_noop},
         .font{nullptr, dyn_noop},
         .shader{nullptr, dyn_noop},
         .win{nullptr, dyn_noop},
         .pipeline{nullptr, dyn_noop},
         .audio{nullptr, dyn_noop},
         .video{nullptr, dyn_noop},
         .animation{nullptr, dyn_noop}},
    gpu_instance_{},
    gpu_device_{},
    buffering_{},
    state_{},
    paths_{.working_dir{allocator_}, .pipeline_cache{allocator_}},
    callbacks_{
      .post_init{nullptr, dyn_noop},
      .pre_shutdown{nullptr, dyn_noop},
      .post_shutdown{nullptr, dyn_noop},
    },
    window_{nullptr, dyn_noop}
  {
  }

  IEngine(IEngine const &)             = delete;
  IEngine & operator=(IEngine const &) = delete;
  IEngine(IEngine &&)                  = default;
  IEngine & operator=(IEngine &&)      = default;
  ~IEngine()                           = default;

  static Dyn<Engine> create(Allocator allocator, EngineCfg const & cfg,
                            Callbacks callbacks, WindowLoop loop);

  Dyn<WindowEntry *> add_window_(EngineCfg::Window const & cfg,
                                 WindowLoop                loop);

  Option<gpu::SwapchainInfo>
    create_swapchain_info_(WindowEntry const & win_entry);

  void poll_inputs_(time_point prev_frame_end, time_point frame_start);

  void shutdown();

  void run();
};

/// Global Engine Pointer. Can be hooked at runtime for dynamically loaded
/// executables.
extern Engine engine;

ASH_C_LINKAGE ASH_DLL_EXPORT void hook_engine(Engine);

}    // namespace ash
