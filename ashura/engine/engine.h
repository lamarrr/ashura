/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/image_decoder.h"
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
// [ ] URI system and parser with specifications: caching behaviour, identifier
// name, etc. generic resource type. load status, registered? etc. named paths,
// i.e. $HOME $FONTS $IMAGES $ASSETS/
struct Engine
{
  void              *app          = nullptr;
  StrHashMap<void *> globals      = {};
  Font               default_font = nullptr;

  Result<Font, FontLoadError> load_font(Span<char const> uri);
  Result<Void, FontLoadError> unload_font(Span<char const> uri);

  void init();
  void uninit();
};

/// Global Engine Pointer. Can be hooked at runtime for dynamically loaded
/// executables.
ASH_C_LINKAGE ASH_DLL_EXPORT Engine *engine;

}        // namespace ash
