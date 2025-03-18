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

u32 FrameGraph::push_ssbo(Span<u8 const> data)
{
  CHECK(!uploaded_, "");
  ssbo_data_
    .resize_uninit(
      align_offset<usize>(gpu::BUFFER_OFFSET_ALIGNMENT, ssbo_data_.size()))
    .unwrap();
  auto const offset = ssbo_data_.size();
  ssbo_data_.extend(data).unwrap();
  auto const size = data.size();
  auto const idx  = ssbo_entries_.size();
  CHECK(ssbo_data_.size() <= U32_MAX, "");
  ssbo_entries_.push(Slice32{(u32) offset, (u32) size}).unwrap();

  return (u32) idx;
}

SSBOSpan FrameGraph::get_ssbo(u32 id)
{
  CHECK(uploaded_, "");
  Slice32 slice = ssbo_entries_.try_get(id).unwrap();
  return SSBOSpan{.ssbo = frame_data_[frame_index_].ssbo, .slice = slice};
}

void FrameGraph::add_pass(Pass pass)
{
  passes_.push(std::move(pass)).unwrap();
}

void FrameGraph::execute(Canvas const & canvas)
{
  FrameData & fd = frame_data_[frame_index_];

  fd.ssbo.assign(sys->gpu, ssbo_data_);

  uploaded_ = true;

  auto const timespan = sys->gpu.begin_timespan("gpu.frame");

  for (Pass const & pass : passes_)
  {
    auto const timespan = sys->gpu.begin_timespan(pass.label);
    auto const stat     = sys->gpu.begin_statistics(pass.label);
    pass.pass(*this, sys->gpu.encoder(), *pass_ctx_, canvas);
    stat.match([&](auto i) { sys->gpu.end_statistics(i); });
    timespan.match([&](auto i) { sys->gpu.end_timespan(i); });
  }

  timespan.match([&](auto i) { sys->gpu.end_timespan(i); });

  frame_index_ = (frame_index_ + 1) % sys->gpu.buffering_;
  arena_.reclaim();
  uploaded_ = false;
  ssbo_data_.clear();
  ssbo_entries_.clear();
  passes_.reset();
}

void FrameGraph::acquire()
{
  frame_data_.resize(sys->gpu.buffering_).unwrap();
}

void FrameGraph::release()
{
  for (FrameData & fd : frame_data_)
  {
    fd.ssbo.release(sys->gpu);
  }
}

void BlurRenderer::render(FrameGraph & graph, Framebuffer const & fb,
                          BlurRenderParam const & blur)
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
    graph.add_pass("Rect Blur"_str, [blur, fb](
                                      FrameGraph &, gpu::CommandEncoder & enc,
                                      PassContext & passes, Canvas const &) {
      BlurPassParams const params{
        .framebuffer = fb, .area = blur.area, .radius = blur.radius};
      passes.blur->encode(enc, params).match([&](FramebufferResult const & r) {
        enc.blit_image(
          r.fb.color.image, fb.color.image,
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

    graph.add_pass(
      "RRect Blur"_str,
      [rrect, blur, fb](FrameGraph & graph, gpu::CommandEncoder & enc,
                        PassContext & passes, Canvas const &) {
        BlurPassParams const params{
          .framebuffer = fb, .area = blur.area, .radius = blur.radius};
        FramebufferResult const result =
          passes.blur->encode(enc, params).unwrap();

        SSBOSpan const ssbo = graph.get_ssbo(rrect);
        passes.rrect->encode(
          enc, RRectPassParams{.framebuffer        = fb,
                               .scissor            = blur.scissor,
                               .viewport           = blur.viewport,
                               .world_to_view      = blur.world_to_view,
                               .params_ssbo        = ssbo.ssbo.descriptor,
                               .params_ssbo_offset = ssbo.slice.offset,
                               .textures           = result.fb.color.texture,
                               .first_instance     = 0,
                               .num_instances      = 1});
      });
  }
}

Renderer Renderer::create(AllocatorRef allocator)
{
  PassContext passes = PassContext::create(allocator);
  return Renderer{allocator,
                  dyn<PassContext>(allocator, std::move(passes)).unwrap()};
}

void Renderer::acquire()
{
  passes_->acquire();
  graph_.acquire();
}

void Renderer::release()
{
  graph_.release();
  passes_->release();
}

void Renderer::render_canvas(Framebuffer const & fb, Canvas const & c)
{
  ScopeTrace trace;

  u32 const rrect_params_id  = graph_.push_ssbo(c.rrect_params.view().as_u8());
  u32 const ngon_params_id   = graph_.push_ssbo(c.ngon_params.view().as_u8());
  u32 const ngon_vertices_id = graph_.push_ssbo(c.ngon_vertices.view().as_u8());
  u32 const ngon_indices_id  = graph_.push_ssbo(c.ngon_indices.view().as_u8());

  for (Canvas::Batch const & batch : c.batches)
  {
    switch (batch.type)
    {
      case Canvas::BatchType::RRect:
        graph_.add_pass(
          "RRect"_str, [fb = fb, scissor = c.clip_to_scissor(batch.clip),
                        viewport = c.viewport, world_to_view = c.world_to_view,
                        rrect_params_id,
                        batch](FrameGraph & graph, gpu::CommandEncoder & enc,
                               PassContext & passes, Canvas const &) {
            SSBOSpan const prm = graph.get_ssbo(rrect_params_id);

            RRectPassParams const params{.framebuffer   = fb,
                                         .scissor       = scissor,
                                         .viewport      = viewport,
                                         .world_to_view = world_to_view,
                                         .params_ssbo   = prm.ssbo.descriptor,
                                         .params_ssbo_offset = prm.slice.offset,
                                         .textures       = sys->gpu.textures_,
                                         .first_instance = batch.run.offset,
                                         .num_instances  = batch.run.span};

            passes.rrect->encode(enc, params);
          });
        break;

      case Canvas::BatchType::Ngon:
        graph_.add_pass(
          "Ngon"_str, [fb = fb, scissor = c.clip_to_scissor(batch.clip),
                       viewport = c.viewport, world_to_view = c.world_to_view,
                       ngon_vertices_id, ngon_indices_id, ngon_params_id,
                       batch](FrameGraph & graph, gpu::CommandEncoder & enc,
                              PassContext & passes, Canvas const & c) {
            SSBOSpan const vtx = graph.get_ssbo(ngon_vertices_id);
            SSBOSpan const idx = graph.get_ssbo(ngon_indices_id);
            SSBOSpan const prm = graph.get_ssbo(ngon_params_id);

            NgonPassParams const params{
              .framebuffer          = fb,
              .scissor              = scissor,
              .viewport             = viewport,
              .world_to_view        = world_to_view,
              .vertices_ssbo        = vtx.ssbo.descriptor,
              .vertices_ssbo_offset = vtx.slice.offset,
              .indices_ssbo         = idx.ssbo.descriptor,
              .indices_ssbo_offset  = idx.slice.offset,
              .params_ssbo          = prm.ssbo.descriptor,
              .params_ssbo_offset   = prm.slice.offset,
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
          graph_, fb,
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
        pass.task(graph_, *passes_, c, fb);
      }
      break;

      default:
        break;
    }
  }

  graph_.execute(c);
}

}    // namespace ash
