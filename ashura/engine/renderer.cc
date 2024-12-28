/// SPDX-License-Identifier: MIT
#include "ashura/engine/renderer.h"
#include "ashura/engine/canvas.h"

namespace ash
{

PassContext PassContext::create(AllocatorRef allocator)
{
  Dyn bloom = dyn(allocator, BloomPass{}).unwrap();
  Dyn blur  = dyn(allocator, BlurPass{}).unwrap();
  Dyn ngon  = dyn(allocator, NgonPass{}).unwrap();
  Dyn pbr   = dyn(allocator, PBRPass{}).unwrap();
  Dyn rrect = dyn(allocator, RRectPass{}).unwrap();

  BloomPass * pbloom = bloom.get();
  BlurPass *  pblur  = blur.get();
  NgonPass *  pngon  = ngon.get();
  PBRPass *   ppbr   = pbr.get();
  RRectPass * prrect = rrect.get();

  Vec<Dyn<Pass *>> all{allocator};

  all.push(cast<Pass *>(std::move(bloom))).unwrap();
  all.push(cast<Pass *>(std::move(blur))).unwrap();
  all.push(cast<Pass *>(std::move(ngon))).unwrap();
  all.push(cast<Pass *>(std::move(pbr))).unwrap();
  all.push(cast<Pass *>(std::move(rrect))).unwrap();

  return PassContext{*pbloom, *pblur, *pngon, *ppbr, *prrect, std::move(all)};
}

void PassContext::acquire()
{
  for (auto const & p : all)
  {
    p->acquire();
  };
}

void PassContext::release()
{
  for (auto const & p : all)
  {
    p->release();
  };
}

Renderer Renderer::create(AllocatorRef allocator)
{
  PassContext passes = PassContext::create(allocator);
  return Renderer{allocator, std::move(passes)};
}

void Renderer::acquire()
{
  passes.acquire();

  for (Dyn<GpuPipeline *> const & p : pipelines)
  {
    p->acquire(passes);
  }

  resources.resize(sys->gpu.buffering).unwrap();
}

void Renderer::release()
{
  for (Resources & r : resources)
  {
    r.pbr_params.release(sys->gpu);
    r.pbr_light_params.release(sys->gpu);
    r.ngon_vertices.release(sys->gpu);
    r.ngon_indices.release(sys->gpu);
    r.ngon_params.release(sys->gpu);
    r.rrect_params.release(sys->gpu);
  }

  resources.reset();

  for (Dyn<GpuPipeline *> const & p : pipelines)
  {
    p->release(passes);
  }

  passes.release();
}

void Renderer::add_pass(Dyn<Pass *> pass)
{
  pass->acquire();
  passes.all.push(std::move(pass)).unwrap();
}

void Renderer::add_pipeline(Dyn<GpuPipeline *> pipeline)
{
  pipeline->acquire(passes);
  pipelines.push(std::move(pipeline)).unwrap();
}

void Renderer::begin_frame(Framebuffer const &, Canvas & canvas)
{
  gpu::CommandEncoder & enc = sys->gpu.encoder();
  Resources &           r   = resources[sys->gpu.ring_index()];

  r.ngon_vertices.assign(sys->gpu, span(canvas.ngon_vertices).as_u8());
  r.ngon_indices.assign(sys->gpu, span(canvas.ngon_indices).as_u8());
  r.ngon_params.assign(sys->gpu, span(canvas.ngon_params).as_u8());
  r.rrect_params.assign(sys->gpu, span(canvas.rrect_params).as_u8());
  r.ngon_vertices.assign(sys->gpu, span(canvas.ngon_vertices).as_u8());

  for (Dyn<GpuPipeline *> const & p : pipelines)
  {
    p->begin_frame(passes, enc);
  }
}

void Renderer::end_frame(Framebuffer const &, Canvas &)
{
  gpu::CommandEncoder & enc = sys->gpu.encoder();

  for (Dyn<GpuPipeline *> const & p : pipelines)
  {
    p->end_frame(passes, enc);
  }
}

void Renderer::render_frame(Framebuffer const & fb, Canvas & canvas)
{
  Resources &           r   = resources[sys->gpu.ring_index()];
  gpu::CommandEncoder & enc = sys->gpu.encoder();

  Canvas::RenderContext render_ctx{.canvas        = canvas,
                                   .passes        = passes,
                                   .framebuffer   = fb,
                                   .enc           = enc,
                                   .rrects        = r.rrect_params,
                                   .ngons         = r.ngon_params,
                                   .ngon_vertices = r.ngon_vertices,
                                   .ngon_indices  = r.ngon_indices};

  for (Canvas::Pass const & pass : canvas.passes)
  {
    pass.task(render_ctx);
  }
}

}    // namespace ash
