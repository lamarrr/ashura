/// SPDX-License-Identifier: MIT
#include "ashura/engine/pipelines/bloom.h"

namespace ash
{

BloomPipeline::BloomPipeline(Allocator)
{
}

Str BloomPipeline::label()
{
  return "Bloom"_str;
}

void BloomPipeline::acquire(GpuFramePlan)
{
}

void BloomPipeline::release(GpuFramePlan)
{
}

void BloomPipeline::encode(gpu::CommandEncoder, BloomPipelineParams const &)
{
  /// E' = Blur(E)
  /// D' = Blur(D) + E'
  /// C' = Blur(C) + D'
  /// B' = Blur(B) + C'
  /// A' = Blur(A) + B'
}

}    // namespace ash
