#include "ashura/engine/passes/msaa.h"

namespace ash
{

void MSAAPass::init(RenderContext &ctx)
{
}

void MSAAPass::uninit(RenderContext &ctx)
{
}

void MSAAPass::add_pass(RenderContext &ctx, MSAAPassParams const &params)
{
  gfx::CommandEncoderImpl encoder = ctx.encoder();
  encoder->resolve_image(encoder.self, params.src_image, params.dst_image,
                         Span{&params.resolve, 1});
}

}        // namespace ash
