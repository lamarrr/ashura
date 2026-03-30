/// SPDX-License-Identifier: MIT
#include "ashura/engine/pipelines/bloom.hpp"

namespace ash
{

BloomPipeline::BloomPipeline(Allocator)
{
}

Str BloomPipeline::label()
{
    return "Bloom"_s;
}

void BloomPipeline::acquire(GpuFramePlan, Allocator)
{
}

void BloomPipeline::release(GpuFramePlan, Allocator)
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
