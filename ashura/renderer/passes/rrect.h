#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/render_context.h"

namespace ash
{

// TODO(lamarrr): vertex buffer is in object coordinate space. needs to be
// transformed to world then to view space, used for uv-interp as well.
// TODO(lamarrr): create atlas renderer similar to this but uses plain rects, no
// rrect, no border, uv->x,y,array index into atlas
// rename to view/screen space transform
constexpr Mat4Affine rrect_model(Vec2 extent)
{
  return affine_translate3d({-extent.x / 2, -extent.y / 2, 0}) *
         affine_scale3d(Vec3{extent.x, extent.y, 1});
}

struct RRectParam
{
  ViewTransform transform    = {};
  f32           radii[4]     = {};
  Vec2          uv[2]        = {};
  Vec4          tint[4]      = {};
  Vec2          aspect_ratio = {1, 1};
  u32           albedo       = 0;
};

struct RRectPassParams
{
  gfx::RenderingInfo rendering_info     = {};
  gfx::DescriptorSet params_ssbo        = nullptr;
  u32                params_ssbo_offset = 0;
  gfx::DescriptorSet textures           = nullptr;
  u32                first_instance     = 0;
  u32                num_instances      = 0;
};

struct RRectPass
{
  gfx::GraphicsPipeline pipeline = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, RRectPassParams const &params);
};

}        // namespace ash
