/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief Passes are re-usable and stateless compute and graphics pipeline
/// components. They set up static resources: pipelines, shaders, and render
/// data needed for executing rendering operations. Passes dispatch
/// compute/graphics shaders using their specified arguments. They are used by renderers.
typedef struct IPipeline * Pipeline;

struct IPipeline
{
  virtual Str label() = 0;

  virtual void acquire(GpuFramePlan plan) = 0;

  virtual void release(GpuFramePlan plan) = 0;

  virtual ~IPipeline() = default;
};

struct PipelineStencil
{
  gpu::StencilState front = {};
  gpu::StencilState back  = {};
};

enum class PipelineVariantId : u32
{
  Base = 0
};

enum class FillRule : u32
{
  EvenOdd = 0,
  NonZero = 1
};

using BlendMode         = shader::BlendMode;
using BezierRegions     = shader::BezierRegions;
using TriangleShadeRate = shader::TriangleShadeRate;
using ShadeType         = shader::sdf::ShadeType;

}    // namespace ash
