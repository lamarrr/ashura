/// SPDX-License-Identifier: MIT
#include "ashura/engine/renderer.h"
#include "ashura/engine/canvas.h"
#include "ashura/std/trace.h"

namespace ash
{

PassContext PassContext::create(AllocatorRef allocator)
{
  Dyn bloom    = dyn(allocator, BloomPass{}).unwrap();
  Dyn blur     = dyn(allocator, BlurPass{}).unwrap();
  Dyn ngon     = dyn(allocator, NgonPass{}).unwrap();
  Dyn pbr      = dyn(allocator, PBRPass{}).unwrap();
  Dyn rrect    = dyn(allocator, RRectPass{}).unwrap();
  Dyn squircle = dyn(allocator, SquirclePass{}).unwrap();

  auto * pbloom    = bloom.get();
  auto * pblur     = blur.get();
  auto * pngon     = ngon.get();
  auto * ppbr      = pbr.get();
  auto * prrect    = rrect.get();
  auto * psquircle = squircle.get();

  Vec<Dyn<Pass *>> all{allocator};

  all.push(cast<Pass *>(std::move(bloom))).unwrap();
  all.push(cast<Pass *>(std::move(blur))).unwrap();
  all.push(cast<Pass *>(std::move(ngon))).unwrap();
  all.push(cast<Pass *>(std::move(pbr))).unwrap();
  all.push(cast<Pass *>(std::move(rrect))).unwrap();
  all.push(cast<Pass *>(std::move(squircle))).unwrap();

  return PassContext{*pbloom, *pblur,     *pngon,        *ppbr,
                     *prrect, *psquircle, std::move(all)};
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

void PassContext::add_pass(Dyn<Pass *> pass)
{
  pass->acquire();
  all.push(std::move(pass)).unwrap();
}

void BlurRenderer::render(FrameGraph & graph, Framebuffer const & fb,
                          Span<ColorTexture const>,
                          Span<DepthStencilTexture const>,
                          PassContext const &     passes,
                          BlurRenderParam const & blur)
{
  if (blur.area.extent.x == 0 || blur.area.extent.y == 0)
  {
    return;
  }

  if (blur.spread_radius.x == 0 || blur.spread_radius.y == 0)
  {
    return;
  }

  BlurPassParams const params{
    .framebuffer = fb, .area = blur.area, .spread_radius = blur.spread_radius};

  if (!params.area.is_visible())
  {
    return;
  }

  u32 const rrect = graph.push_ssbo(Span{&blur.rrect, 1}.as_u8());

  graph.add_pass(
    "RRect Blur"_str, [rrect, blur, fb, &passes,
                       params](FrameGraph & graph, gpu::CommandEncoder & enc) {
      auto const result = passes.blur->encode(enc, params).unwrap();

      auto [sb, slice] = graph.get_structured_buffer(rrect);
      passes.rrect->encode(enc,
                           RRectPassParams{.framebuffer  = fb,
                                           .scissor      = blur.scissor,
                                           .viewport     = blur.viewport,
                                           .world_to_ndc = blur.world_to_ndc,
                                           .params_ssbo  = sb.descriptor_,
                                           .params_ssbo_offset = slice.offset,
                                           .textures = result.color.texture,
                                           .first_instance = 0,
                                           .num_instances  = 1});
    });
}

Renderer Renderer::create(AllocatorRef allocator)
{
  PassContext passes = PassContext::create(allocator);
  return Renderer{dyn<PassContext>(allocator, std::move(passes)).unwrap()};
}

void Renderer::acquire()
{
  passes_->acquire();
}

void Renderer::release()
{
  passes_->release();
}

void Renderer::render_canvas(FrameGraph & graph, Canvas const & c,
                             Framebuffer const &             fb,
                             Span<ColorTexture const>        scratch_colors,
                             Span<DepthStencilTexture const> scratch_ds)
{
  ScopeTrace trace;

  auto const rrect_params = graph.push_ssbo(c.rrect_params_.view().as_u8());
  auto const squircle_params =
    graph.push_ssbo(c.squircle_params_.view().as_u8());
  auto const ngon_params   = graph.push_ssbo(c.ngon_params_.view().as_u8());
  auto const ngon_vertices = graph.push_ssbo(c.ngon_vertices_.view().as_u8());
  auto const ngon_indices  = graph.push_ssbo(c.ngon_indices_.view().as_u8());

  for (Canvas::Batch const & batch : c.batches_)
  {
    switch (batch.type)
    {
      case Canvas::BatchType::RRect:
        graph.add_pass(
          "RRect"_str, [&c, fb, rrect_params, batch, &passes = *passes_](
                         FrameGraph & graph, gpu::CommandEncoder & enc) {
            auto [prm, slice] = graph.get_structured_buffer(rrect_params);

            RRectPassParams const params{.framebuffer = fb,
                                         .scissor =
                                           c.clip_to_scissor(batch.clip),
                                         .viewport           = c.viewport_,
                                         .world_to_ndc       = c.world_to_ndc_,
                                         .params_ssbo        = prm.descriptor_,
                                         .params_ssbo_offset = slice.offset,
                                         .textures       = sys->gpu.textures_,
                                         .first_instance = batch.run.offset,
                                         .num_instances  = batch.run.span};

            passes.rrect->encode(enc, params);
          });
        break;

      case Canvas::BatchType::Squircle:
        graph.add_pass(
          "Squircle"_str, [&c, fb, squircle_params, batch, &passes = *passes_](
                            FrameGraph & graph, gpu::CommandEncoder & enc) {
            auto [prm, slice] = graph.get_structured_buffer(squircle_params);

            SquirclePassParams const params{.framebuffer = fb,
                                            .scissor =
                                              c.clip_to_scissor(batch.clip),
                                            .viewport     = c.viewport_,
                                            .world_to_ndc = c.world_to_ndc_,
                                            .params_ssbo  = prm.descriptor_,
                                            .params_ssbo_offset = slice.offset,
                                            .textures = sys->gpu.textures_,
                                            .first_instance = batch.run.offset,
                                            .num_instances  = batch.run.span};

            passes.squircle->encode(enc, params);
          });
        break;

      case Canvas::BatchType::Ngon:
        graph.add_pass(
          "Ngon"_str,
          [&c, fb, ngon_vertices, ngon_indices, ngon_params, batch,
           &passes = *passes_](FrameGraph & graph, gpu::CommandEncoder & enc) {
            auto [vtx, vtx_slice] = graph.get_structured_buffer(ngon_vertices);
            auto [idx, idx_slice] = graph.get_structured_buffer(ngon_indices);
            auto [prm, prm_slice] = graph.get_structured_buffer(ngon_params);

            NgonPassParams const params{
              .framebuffer          = fb,
              .scissor              = c.clip_to_scissor(batch.clip),
              .viewport             = c.viewport_,
              .world_to_ndc         = c.world_to_ndc_,
              .vertices_ssbo        = vtx.descriptor_,
              .vertices_ssbo_offset = vtx_slice.offset,
              .indices_ssbo         = idx.descriptor_,
              .indices_ssbo_offset  = idx_slice.offset,
              .params_ssbo          = prm.descriptor_,
              .params_ssbo_offset   = prm_slice.offset,
              .textures             = sys->gpu.textures_,
              .first_instance       = batch.run.offset,
              .index_counts = c.ngon_index_counts_.view().slice(batch.run)};

            passes.ngon->encode(enc, params);
          });
        break;

      case Canvas::BatchType::Blur:
      {
        Canvas::Blur const & blur = c.blurs_[batch.run.offset];
        BlurRenderer::render(
          graph, fb, scratch_colors, scratch_ds, *passes_,
          BlurRenderParam{.rrect         = blur.rrect,
                          .area          = blur.area,
                          .spread_radius = blur.spread_radius,
                          .scissor       = c.clip_to_scissor(batch.clip),
                          .viewport      = c.viewport_,
                          .world_to_ndc  = c.world_to_ndc_});
      }
      break;

      case Canvas::BatchType::Pass:
      {
        Canvas::Pass & pass = c.passes_[batch.run.offset];
        pass.task(graph, *passes_, c, fb, scratch_colors, scratch_ds);
      }
      break;

      default:
        break;
    }
  }
}

}    // namespace ash
