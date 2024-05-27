#pragma once
#include "ashura/engine/render_context.h"

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
  void init(RenderContext & ctx);
  void add_pass(RenderContext & ctx, MSAAPassParams const &params);
  void uninit(RenderContext & ctx);
};

}        // namespace ash
