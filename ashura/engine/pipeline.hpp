/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.hpp"
#include "ashura/engine/shaders/items.gen.hpp"
#include "ashura/gpu/gpu.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

/// @brief Passes are re-usable and stateless compute and graphics pipeline
/// components. They set up static resources: pipelines, shaders, and render
/// data needed for executing rendering operations. Passes dispatch
/// compute/graphics shaders using their specified arguments. They are used by
/// renderers.
typedef struct IPipeline * Pipeline;

struct IPipeline
{
    virtual Str label() = 0;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) = 0;

    virtual void release(GpuFramePlan plan, Allocator allocator) = 0;

    virtual ~IPipeline() = default;
};

struct PipelineStencil
{
    gpu::StencilState front = {};
    gpu::StencilState back  = {};

    constexpr bool operator==(PipelineStencil const & other) const = default;
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

using BlendMode     = shader::BlendMode;
using BezierRegions = shader::BezierRegions;
using SdfShapeType  = shader::SdfShapeType;
using SdfBlendOp    = shader::SdfBlendOp;
using ShadeType     = shader::SdfShadeType;

}    // namespace ash
