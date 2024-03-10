#pragma once
#include "ashura/renderer/renderer.h"

namespace ash
{

struct MSAAPassParams
{
  gfx::Image        src_image = nullptr;
  gfx::Image        dst_image = nullptr;
  gfx::ImageResolve resolve   = {};
};

struct MSAAPass
{
  void init(Renderer &renderer);
  void add_pass(Renderer &renderer, MSAAPassParams const &params);
  void uninit(Renderer &renderer);
};

}        // namespace ash
