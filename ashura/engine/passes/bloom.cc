/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/bloom.h"

namespace ash
{

void BloomPass::acquire()
{
}

void BloomPass::release()
{
}

void BloomPass::encode(gpu::CommandEncoder &, BloomPassParams const &)
{
  /// E' = Blur(E)
  /// D' = Blur(D) + E'
  /// C' = Blur(C) + D'
  /// B' = Blur(B) + C'
  /// A' = Blur(A) + B'
}

}    // namespace ash
