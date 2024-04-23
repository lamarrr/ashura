#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/render_context.h"
#include "ashura/renderer/shader.h"
#include "ashura/std/math.h"

namespace ash
{

BEGIN_SHADER_PARAMETER(RRectShaderParameter)
SHADER_COMBINED_IMAGE_SAMPLER(albedo, 1)
END_SHADER_PARAMETER(RRectShaderParameter)

// TODO(lamarrr): vertex buffer is in object coordinate space. needs to be
// transformed to world then to view space, used for uv-interp as well.
// TODO(lamarrr): create atlas renderer similar to this but uses plain rects, no
// rrect, no border, uv->x,y,array index into atlas
constexpr Mat4Affine rrect_model(Vec2 extent)
{
  return affine_translate3d({-extent.x / 2, -extent.y / 2, 0}) *
         affine_scale3d(Vec3{extent.x, extent.y, 1});
}

struct RRectShaderUniform
{
  MVPTransform transform        = {};
  f32          radii[4]         = {};
  Vec2         uv[2]            = {};
  Vec4         tint             = {};
  Vec4         border_color     = {};
  f32          border_thickness = 0;
  f32          border_softness  = 0;
};

struct RRectObject
{
  gfx::DescriptorSet descriptor = {};
  Uniform            uniform    = {};
};

struct RRectPassParams
{
  RenderTarget            render_target = {};
  Span<RRectObject const> objects       = {};
};

struct RRectPass
{
  gfx::RenderPass          render_pass           = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, RRectPassParams const &params);
};

}        // namespace ash
