/// SPDX-License-Identifier: MIT
#include "ashura/engine/pipeline_system.h"
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipelines/bezier_stencil.h"
#include "ashura/engine/pipelines/blur.h"
#include "ashura/engine/pipelines/fill_stencil.h"
#include "ashura/engine/pipelines/pbr.h"
#include "ashura/engine/pipelines/quad.h"
#include "ashura/engine/pipelines/sdf.h"
#include "ashura/engine/pipelines/triangle_fill.h"
#include "ashura/engine/pipelines/vector_path.h"
#include "ashura/engine/systems.h"
#include "ashura/std/trace.h"

namespace ash
{

void IPipelineSys::init(Allocator allocator)
{
    tracing::ScopeTrace trace;

    Dyn sdf  = dyn<SdfPipeline>(inplace, allocator, allocator).unwrap();
    Dyn quad = dyn<QuadPipeline>(inplace, allocator, allocator).unwrap();
    Dyn triangle_fill =
      dyn<TriangleFillPipeline>(inplace, allocator, allocator).unwrap();
    Dyn fill_stencil =
      dyn<FillStencilPipeline>(inplace, allocator, allocator).unwrap();
    Dyn bezier_stencil =
      dyn<BezierStencilPipeline>(inplace, allocator, allocator).unwrap();
    Dyn blur = dyn<BlurPipeline>(inplace, allocator, allocator).unwrap();
    Dyn pbr  = dyn<PBRPipeline>(inplace, allocator, allocator).unwrap();
    Dyn vector_path =
      dyn<VectorPathPipeline>(inplace, allocator, allocator).unwrap();

    auto p_sdf            = sdf.get();
    auto p_quad           = quad.get();
    auto p_triangle_fill  = triangle_fill.get();
    auto p_fill_stencil   = fill_stencil.get();
    auto p_bezier_stencil = bezier_stencil.get();
    auto p_blur           = blur.get();
    auto p_pbr            = pbr.get();
    auto p_vector_path    = vector_path.get();

    SparseVec<PipelineId, Dyn<Pipeline>> all{allocator};

    all.push(cast<Pipeline>(std::move(sdf))).unwrap();
    all.push(cast<Pipeline>(std::move(quad))).unwrap();
    all.push(cast<Pipeline>(std::move(triangle_fill))).unwrap();
    all.push(cast<Pipeline>(std::move(fill_stencil))).unwrap();
    all.push(cast<Pipeline>(std::move(bezier_stencil))).unwrap();
    all.push(cast<Pipeline>(std::move(blur))).unwrap();
    all.push(cast<Pipeline>(std::move(pbr))).unwrap();

    sdf_            = p_sdf;
    quad_           = p_quad;
    triangle_fill_  = p_triangle_fill;
    fill_stencil_   = p_fill_stencil;
    bezier_stencil_ = p_bezier_stencil;
    blur_           = p_blur;
    pbr_            = p_pbr;
    vector_path_    = p_vector_path;
    all_            = std::move(all);
    allocator_      = allocator;

    auto scratch = IFallbackAllocator{get_thread_arena(), allocator_};

    for (auto [pass] : all)
    {
        pass->acquire(gpu_sys_->current_plan(), allocator_, scratch);
    }
}

void IPipelineSys::shutdown()
{
    auto scratch = IFallbackAllocator{get_thread_arena(), allocator_};

    WriteGuard guard{rw_lock_};
    for (auto [p] : all_)
    {
        p->release(gpu_sys_->current_plan(), allocator_, scratch);
    }
}

SdfPipeline & IPipelineSys::sdf() const
{
    return *sdf_;
}

QuadPipeline & IPipelineSys::quad() const
{
    return *quad_;
}

TriangleFillPipeline & IPipelineSys::triangle_fill() const
{
    return *triangle_fill_;
}

FillStencilPipeline & IPipelineSys::fill_stencil() const
{
    return *fill_stencil_;
}

BezierStencilPipeline & IPipelineSys::bezier_stencil() const
{
    return *bezier_stencil_;
}

BlurPipeline & IPipelineSys::blur() const
{
    return *blur_;
}

PBRPipeline & IPipelineSys::pbr() const
{
    return *pbr_;
}

VectorPathPipeline & IPipelineSys::vector_path() const
{
    return *vector_path_;
}

Future<PipelineId> IPipelineSys::add_pipeline(Dyn<Pipeline> pipeline)
{
    return scheduler
      ->run(
        allocator_, MainThread::Main,
        [pipeline = std::move(pipeline), this]() mutable {
            auto scratch = IFallbackAllocator{get_thread_arena(), allocator_};
            pipeline->acquire(gpu_sys_->current_plan(), allocator_, scratch);
            WriteGuard guard{this->rw_lock_};
            return this->all_.push(std::move(pipeline)).unwrap();
        })
      .unwrap();
}

Option<IPipeline &> IPipelineSys::get(Str label)
{
    ReadGuard guard{rw_lock_};
    for (auto [pass] : all_)
    {
        if (mem::eq(pass->label(), label))
        {
            return *pass;
        }
    }

    return none;
}

IPipeline & IPipelineSys::get(PipelineId id)
{
    ReadGuard guard{rw_lock_};
    ASH_CHECK(all_.is_valid_id(id), "Invalid PipelineId");
    return *all_[id].v0;
}

}    // namespace ash
