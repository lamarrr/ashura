#pragma once
#include "ashura/renderer/render_context.h"

namespace ash
{

BEGIN_SHADER_PARAMETER(BlurPassShaderParameter)
SHADER_SAMPLED_IMAGE(src, 1)
SHADER_STORAGE_IMAGE(dst, 1)
END_SHADER_PARAMETER(BlurPassShaderParameter)

struct BlurPassShaderUniform
{
  Vec2I src_offset;
  Vec2I dst_offset;
  Vec2I extent;
  Vec2I radius;
};

struct BlurPassParams
{
  Vec2U          offset = {};
  Vec2U          extent = {};
  Vec2U          radius = {0, 0};
  gfx::ImageView view   = nullptr;
};

struct BlurPass
{
  ShaderParameterHeap<BlurPassShaderParameter> parameter_heap_ = {};
  gfx::ComputePipeline                         pipeline_       = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, BlurPassParams const &params);
};

}        // namespace ash
