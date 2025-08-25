/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/pipeline.h"
#include "ashura/std/dyn.h"
#include "ashura/std/option.h"

namespace ash
{

typedef struct IPipelineSys * PipelineSys;

struct SdfPipeline;
struct QuadPipeline;
struct NgonPipeline;
struct FillStencilPipeline;
struct BezierStencilPipeline;
struct BlurPipeline;
struct PBRPipeline;

struct IPipelineSys
{
  SdfPipeline *           sdf_;
  QuadPipeline *          quad_;
  NgonPipeline *          ngon_;
  FillStencilPipeline *   fill_stencil_;
  BezierStencilPipeline * bezier_stencil_;
  BlurPipeline *          blur_;
  PBRPipeline *           pbr_;
  Vec<Dyn<Pipeline>>      all_;

  IPipelineSys() :
    sdf_{nullptr},
    quad_{nullptr},
    ngon_{nullptr},
    fill_stencil_{nullptr},
    bezier_stencil_{nullptr},
    blur_{nullptr},
    pbr_{nullptr},
    all_{}
  {
  }

  IPipelineSys(IPipelineSys const &)             = delete;
  IPipelineSys(IPipelineSys &&)                  = delete;
  IPipelineSys & operator=(IPipelineSys const &) = delete;
  IPipelineSys & operator=(IPipelineSys &&)      = delete;
  ~IPipelineSys()                                = default;

  void init(Allocator allocator);

  void uninit();

  SdfPipeline & sdf() const;

  QuadPipeline & quad() const;

  NgonPipeline & ngon() const;

  FillStencilPipeline & fill_stencil() const;

  BezierStencilPipeline & bezier_stencil() const;

  BlurPipeline & blur() const;

  PBRPipeline & pbr() const;

  void add_pipeline(Dyn<Pipeline> pipeline);

  Option<IPipeline &> get(Str pipeline);
};

}    // namespace ash
