/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct BloomPipelineParams
{
};

struct BloomPipeline final : IPipeline
{
  BloomPipeline(Allocator);
  BloomPipeline(BloomPipeline const &)             = delete;
  BloomPipeline(BloomPipeline &&)                  = delete;
  BloomPipeline & operator=(BloomPipeline const &) = delete;
  BloomPipeline & operator=(BloomPipeline &&)      = delete;

  virtual ~BloomPipeline() override = default;

  virtual Str label() override;

  virtual void acquire(GpuFramePlan plan) override;

  virtual void release(GpuFramePlan plan) override;

  void encode(gpu::CommandEncoder encoder, BloomPipelineParams const & params);
};

}    // namespace ash
