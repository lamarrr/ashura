/// SPDX-License-Identifier: MIT
#include "ashura/engine/renderer.h"
#include "ashura/engine/canvas.h"
#include "ashura/std/trace.h"

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

void PassContext::add_pass(Dyn<Pass *> pass)
{
  pass->acquire();
  all.push(std::move(pass)).unwrap();
}

void BlurRenderer::render(PassContext const & passes, FrameGraph & graph,
                          Framebuffer const & fb, BlurRenderParam const & blur)
{
  if (blur.area.extent.x == 0 || blur.area.extent.y == 0)
  {
    return;
  }

  if (blur.radius.x == 0 || blur.radius.y == 0)
  {
    return;
  }

  RectU const downsampled_area = {
    .offset{0, 0},
    .extent = blur.area.extent / BlurPass::DOWNSCALE_FACTOR
  };

  if (downsampled_area.extent.x == 0 || downsampled_area.extent.y == 0)
  {
    return;
  }

  if (blur.corner_radii.x <= 0 && blur.corner_radii.y <= 0 &&
      blur.corner_radii.z <= 0 && blur.corner_radii.w <= 0)
  {
    graph.add_pass("Rect Blur"_str, [blur, fb, &passes](
                                      FrameGraph &, gpu::CommandEncoder & enc) {
      BlurPassParams const params{
        .framebuffer = fb, .area = blur.area, .radius = blur.radius};
      passes.blur->encode(enc, params).match([&](auto & r) {
        enc.blit_image(
          r.color.image, fb.color.image,
          span({
            gpu::ImageBlit{.src_layers{.aspects   = gpu::ImageAspects::Color,
                                       .mip_level = 0,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1},
                           .src_area = as_boxu(r.rect),
                           .dst_layers{.aspects   = gpu::ImageAspects::Color,
                                       .mip_level = 0,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1},
                           .dst_area = as_boxu(params.area)}
        }),
          gpu::Filter::Linear);
      });
    });
  }
  else
  {
    // assumes sample framebuffer extent is same as framebuffer extent
    Vec2 const uv_scale = 1 / as_vec2(fb.extent().xy());

    RRectParam const rrects[] = {
      {.transform = blur.transform,
       .tint{norm(colors::WHITE), norm(colors::WHITE), norm(colors::WHITE),
             norm(colors::WHITE)},
       .radii = blur.corner_radii,
       .uv{as_vec2(downsampled_area.begin()) * uv_scale,
           as_vec2(downsampled_area.end()) * uv_scale},
       .tiling          = 1.0F,
       .aspect_ratio    = blur.aspect_ratio,
       .stroke          = 0,
       .thickness       = 0,
       .edge_smoothness = 0,
       .sampler         = SamplerId::LinearClamped,
       .albedo          = TextureId{0}}
    };

    u32 const rrect = graph.push_ssbo(span(rrects).as_u8());

    graph.add_pass("RRect Blur"_str, [rrect, blur, fb,
                                      &passes](FrameGraph &          graph,
                                               gpu::CommandEncoder & enc) {
      BlurPassParams const params{
        .framebuffer = fb, .area = blur.area, .radius = blur.radius};
      auto const result = passes.blur->encode(enc, params).unwrap();

      auto [sb, slice] = graph.get_structured_buffer(rrect);
      passes.rrect->encode(enc,
                           RRectPassParams{.framebuffer   = fb,
                                           .scissor       = blur.scissor,
                                           .viewport      = blur.viewport,
                                           .world_to_view = blur.world_to_view,
                                           .params_ssbo   = sb.descriptor_,
                                           .params_ssbo_offset = slice.offset,
                                           .textures = result.color.texture,
                                           .first_instance = 0,
                                           .num_instances  = 1});
    });
  }
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

void Renderer::render_canvas(Framebuffer const & fb, FrameGraph & graph,
                             Canvas const & c)
{
  ScopeTrace trace;

  auto const rrect_params  = graph.push_ssbo(c.rrect_params.view().as_u8());
  auto const ngon_params   = graph.push_ssbo(c.ngon_params.view().as_u8());
  auto const ngon_vertices = graph.push_ssbo(c.ngon_vertices.view().as_u8());
  auto const ngon_indices  = graph.push_ssbo(c.ngon_indices.view().as_u8());

  for (Canvas::Batch const & batch : c.batches)
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
                                         .viewport           = c.viewport,
                                         .world_to_view      = c.world_to_view,
                                         .params_ssbo        = prm.descriptor_,
                                         .params_ssbo_offset = slice.offset,
                                         .textures       = sys->gpu.textures_,
                                         .first_instance = batch.run.offset,
                                         .num_instances  = batch.run.span};

            passes.rrect->encode(enc, params);
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
              .viewport             = c.viewport,
              .world_to_view        = c.world_to_view,
              .vertices_ssbo        = vtx.descriptor_,
              .vertices_ssbo_offset = vtx_slice.offset,
              .indices_ssbo         = idx.descriptor_,
              .indices_ssbo_offset  = idx_slice.offset,
              .params_ssbo          = prm.descriptor_,
              .params_ssbo_offset   = prm_slice.offset,
              .textures             = sys->gpu.textures_,
              .first_instance       = batch.run.offset,
              .index_counts =
                c.ngon_index_counts.view().slice(batch.run.as_slice())};

            passes.ngon->encode(enc, params);
          });
        break;

      case Canvas::BatchType::Blur:
      {
        Canvas::Blur const & blur = c.blurs[batch.run.offset];
        BlurRenderer::render(
          *passes_, graph, fb,
          BlurRenderParam{.area          = blur.area,
                          .radius        = blur.radius,
                          .corner_radii  = blur.corner_radii,
                          .transform     = blur.transform,
                          .aspect_ratio  = blur.aspect_ratio,
                          .scissor       = c.clip_to_scissor(batch.clip),
                          .viewport      = c.viewport,
                          .world_to_view = c.world_to_view});
      }
      break;

      case Canvas::BatchType::Pass:
      {
        Canvas::Pass & pass = c.passes[batch.run.offset];
        pass.task(graph, *passes_, c, fb);
      }
      break;

      default:
        break;
    }
  }
}

}    // namespace ash
