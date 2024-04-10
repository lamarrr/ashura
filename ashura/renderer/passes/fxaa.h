#pragma once
#include "ashura/renderer/render_context.h"

namespace ash
{

struct FXAAPassParams
{
  gfx::Image        src_image = nullptr;
  gfx::Image        dst_image = nullptr;
  gfx::ImageResolve resolve   = {};
};

struct FXAAPass
{
  void init(RenderContext & ctx);
  void add_pass(RenderContext & ctx, FXAAPassParams const &params);
  void uninit(RenderContext & ctx);
};

}        // namespace ash
