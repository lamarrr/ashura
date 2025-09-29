/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/cfg.h"

namespace ash
{

typedef struct ILogger *       Logger;
typedef struct IScheduler *    Scheduler;
typedef struct IGpuSys *       GpuSys;
typedef struct IFileSys *      FileSys;
typedef struct IImageSys *     ImageSys;
typedef struct IFontSys *      FontSys;
typedef struct IShaderSys *    ShaderSys;
typedef struct IWindowSys *    WindowSys;
typedef struct IPipelineSys *  PipelineSys;
typedef struct IAudioSys *     AudioSys;
typedef struct IVideoSys *     VideoSys;
typedef struct IAnimationSys * AnimationSys;
typedef struct IViewSys *      ViewSys;

struct Systems
{
  Logger       logger    = nullptr;
  Scheduler    sched     = nullptr;
  GpuSys       gpu       = nullptr;
  FileSys      file      = nullptr;
  ImageSys     image     = nullptr;
  FontSys      font      = nullptr;
  ShaderSys    shader    = nullptr;
  WindowSys    win       = nullptr;
  PipelineSys  pipeline  = nullptr;
  AudioSys     audio     = nullptr;
  VideoSys     video     = nullptr;
  AnimationSys animation = nullptr;
  ViewSys      view      = nullptr;
};

// [ ] ensure main thread's scheduler tasks are executed

extern Systems sys;

// [ ] implement

/// @brief Global system object. Designed for hooking across DLLs. Must be
/// initialized at program startup.
ASH_C_LINKAGE ASH_DLL_EXPORT void hook_system(Systems const * sys);

}    // namespace ash
