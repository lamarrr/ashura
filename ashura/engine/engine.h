/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/image_decoder.h"
#include "ashura/std/async.h"

namespace ash
{

struct Engine
{
  // [ ] Font Manager
  // [ ] Shader Manager
  // [ ] Image Manager
  // [ ] Graphics Device
  // [ ] Render Context : Shader Manager + input
  // [ ] Renderer
  // [ ] Async Task Executor
  // [ ] Window Manager? We might need to attach new contexts to new windows
  // [ ] Audio Device
  Font              default_font = nullptr;
  Result<Font, int> load_font(char const *);

  void init();
  void uninit();
};

extern Engine engine;

}        // namespace ash
