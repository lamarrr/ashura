/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/image_decoder.h"
#include "ashura/std/async.h"

namespace ash
{

enum class FontLoadError : i32
{
};

enum class ImageLoadError : i32
{
};

struct Engine
{
  void              *app     = nullptr;
  StrHashMap<void *> globals = {};
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
  Font                        default_font = nullptr;
  Result<Font, FontLoadError> load_font(Span<char const> name);
  Result<Void, FontLoadError> unload_font(Span<char const> name);

  void init();
  void uninit();
};

extern Engine engine;

}        // namespace ash
