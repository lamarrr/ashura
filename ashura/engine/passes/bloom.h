
/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/render_context.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/types.h"

namespace ash
{

struct BloomPassParams
{
  Vec2U          offset = {};
  Vec2U          extent = {};
  gfx::Image     image  = nullptr;
  gfx::ImageView view   = nullptr;
};

struct BloomPass
{
  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, BloomPassParams const &params);
};

}        // namespace ash
