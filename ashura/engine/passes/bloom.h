/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct BloomPassParams
{
};

struct BloomPass final : Pass
{
  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder & encoder, BloomPassParams const & params);
};

}    // namespace ash
