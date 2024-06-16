#include "ashura/engine/passes/bloom.h"

namespace ash
{

void BloomPass::init(RenderContext &ctx)
{
}

void BloomPass::uninit(RenderContext &ctx)
{
}

void BloomPass::add_pass(RenderContext &ctx, BloomPassParams const &params)
{
  /// E' = Blur(E)
  /// D' = Blur(D) + E'
  /// C' = Blur(C) + D'
  /// B' = Blur(B) + C'
  /// A' = Blur(A) + B'
}

}        // namespace ash