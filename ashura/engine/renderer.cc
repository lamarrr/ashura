/// SPDX-License-Identifier: MIT
#include "ashura/engine/renderer.h"
#include "ashura/engine/canvas.h"

namespace ash
{

void Renderer::begin_frame(GpuSystem & gpu, Framebuffer const &,
                           Canvas &    canvas)
{
  gpu::CommandEncoder & enc = gpu.encoder();
  Resources &           r   = resources[gpu.ring_index()];

  r.ngon_vertices.copy(gpu, span(canvas.ngon_vertices).as_u8());
  r.ngon_indices.copy(gpu, span(canvas.ngon_indices).as_u8());
  r.ngon_params.copy(gpu, span(canvas.ngon_params).as_u8());
  r.rrect_params.copy(gpu, span(canvas.rrect_params).as_u8());
  r.ngon_vertices.copy(gpu, span(canvas.ngon_vertices).as_u8());

  for (Dyn<GpuPipeline *> const & p : pipelines)
  {
    p->begin_frame(gpu, passes, enc);
  }
}

void Renderer::end_frame(GpuSystem & gpu, Framebuffer const &, Canvas &)
{
  gpu::CommandEncoder & enc = gpu.encoder();

  for (Dyn<GpuPipeline *> const & p : pipelines)
  {
    p->end_frame(gpu, passes, enc);
  }
}

void Renderer::render_frame(GpuSystem & gpu, Framebuffer const & fb,
                            Canvas & canvas)
{
  Resources &           r   = resources[gpu.ring_index()];
  gpu::CommandEncoder & enc = gpu.encoder();

  Canvas::RenderContext render_ctx{.canvas        = canvas,
                                   .gpu           = gpu,
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
