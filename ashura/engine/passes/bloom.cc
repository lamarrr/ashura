/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/bloom.h"

namespace ash
{

void BloomPass::init(RenderContext &)
{
}

void BloomPass::uninit(RenderContext &)
{
}

void BloomPass::add_pass(RenderContext &, BloomPassParams const &)
{
  /// E' = Blur(E)
  /// D' = Blur(D) + E'
  /// C' = Blur(C) + D'
  /// B' = Blur(B) + C'
  /// A' = Blur(A) + B'
}

}        // namespace ash