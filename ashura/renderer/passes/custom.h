#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/light.h"
#include "ashura/renderer/render_context.h"
#include "ashura/renderer/shader.h"

namespace ash
{

// pipelines statically have depth/stencil tests enabled or disabled
struct CustomShaderPassObject
{
  gfx::GraphicsPipeline    pipeline  = nullptr;
  Mesh                     mesh      = {};
  gfx::DescriptorSet       set_0     = {};
  Uniform                  uniform_0 = {};
  gfx::DescriptorSet       set_1     = {};
  Uniform                  uniform_1 = {};
  gfx::IndirectDrawCommand command   = {};
};

struct CustomShaderPassParams
{
  RenderTarget                       render_target = {};
  Span<CustomShaderPassObject const> objects       = {};
};

struct CustomShaderPass
{
  gfx::RenderPass render_pass = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, CustomShaderPassParams const &params);
};

}        // namespace ash