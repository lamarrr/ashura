#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/light.h"
#include "ashura/renderer/renderer.h"
#include "ashura/renderer/shader.h"

namespace ash
{

BEGIN_SHADER_PARAMETER(RRectShaderParameter)
SHADER_SAMPLER(sampler, 1)
SHADER_SAMPLED_IMAGE(base_color, 1)
END_SHADER_PARAMETER(RRectShaderParameter)

struct RRectShaderUniform
{
  MVPTransform transform        = {};
  Vec4         tint_tl          = {};
  Vec4         tint_tr          = {};
  Vec4         tint_bl          = {};
  Vec4         tint_br          = {};
  Vec4         border_color_tl  = {};
  Vec4         border_color_tr  = {};
  Vec4         border_color_bl  = {};
  Vec4         border_color_br  = {};
  Vec4         border_radii     = {};
  f32          border_thickness = 0;
  Vec2         uv0              = {};
  Vec2         uv1              = {};
};

struct RRectObject
{
  gfx::DescriptorSet descriptor       = {};
  MVPTransform       transform        = {};
  Vec3               center           = {};
  Vec3               half_extent      = {};
  Vec4               tint_tl          = {};
  Vec4               tint_tr          = {};
  Vec4               tint_bl          = {};
  Vec4               tint_br          = {};
  Vec4               border_color_tl  = {};
  Vec4               border_color_tr  = {};
  Vec4               border_color_bl  = {};
  Vec4               border_color_br  = {};
  Vec4               border_radii     = {};
  f32                border_thickness = 0;
  Vec2               uv0              = {};
  Vec2               uv1              = {};
};

struct RRectParams
{
  RenderTarget            render_target = {};
  Span<RRectObject const> objects       = {};
};

struct RRectPass
{
  gfx::RenderPass          render_pass           = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;
  gfx::Buffer              vertex_buffer         = nullptr;
  gfx::Buffer              index_buffer          = nullptr;

  void init(Renderer &renderer);
  void uninit(Renderer &renderer);
  void add_pass(Renderer &renderer, RRectParams const &params);
};

}        // namespace ash
