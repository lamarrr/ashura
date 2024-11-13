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

// [ ] Font Manager
// [ ] Shader Manager
// [ ] Image Manager
// [ ] Graphics Device
// [ ] Render Context : Shader Manager + input
// [ ] Renderer
// [ ] Async Task Executor
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
  gpu::InstanceImpl instance = {};

  gpu::DeviceImpl device = {};

  Window window = nullptr;

  gpu::Surface surface = nullptr;

  gpu::Swapchain swapchain = nullptr;

  gpu::PresentMode present_mode_preference = gpu::PresentMode::Immediate;

  GpuContext gpu_ctx = {};

  Renderer renderer = {};

  Canvas canvas = {};

  ViewSystem view_system = {};

  ViewContext view_ctx = {};

  Font default_font = nullptr;

  static void init();

  static void uninit();

  Engine() = default;

  ~Engine();

  void run(void *app, View &view);
};

/// Global Engine Pointer. Can be hooked at runtime for dynamically loaded
/// executables.
ASH_C_LINKAGE ASH_DLL_EXPORT Engine *engine;

}        // namespace ash
