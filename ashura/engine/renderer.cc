/// SPDX-License-Identifier: MIT
#include "ashura/engine/renderer.h"
#include "ashura/std/math.h"
#include "ashura/std/range.h"

namespace ash
{

void PassContext::init(RenderContext &ctx)
{
  bloom.init(ctx);
  blur.init(ctx);
  ngon.init(ctx);
  pbr.init(ctx);
  rrect.init(ctx);
  custom.for_each(
      [&](Span<char const>, RenderPassImpl const &p) { p.init(p.pass, ctx); });
}

void PassContext::uninit(RenderContext &ctx)
{
  bloom.uninit(ctx);
  blur.uninit(ctx);
  ngon.uninit(ctx);
  pbr.uninit(ctx);
  rrect.uninit(ctx);
  custom.for_each([&](Span<char const>, RenderPassImpl const &p) {
    p.uninit(p.pass, ctx);
  });
  custom.uninit();
}

void SSBO::uninit(RenderContext &ctx)
{
  ctx.device->destroy_descriptor_set(ctx.device.self, ssbo);
  ctx.device->destroy_buffer(ctx.device.self, buffer);
}

void SSBO::reserve(RenderContext &ctx, u64 p_size, Span<char const> label)
{
  p_size = max(p_size, (u64) 1);
  if (buffer != nullptr && size >= p_size)
  {
    return;
  }

  ctx.device->destroy_buffer(ctx.device.self, buffer);

  buffer = ctx.device
               ->create_buffer(
                   ctx.device.self,
                   gfx::BufferDesc{.label       = label,
                                   .size        = p_size,
                                   .host_mapped = true,
                                   .usage = gfx::BufferUsage::TransferSrc |
                                            gfx::BufferUsage::TransferDst |
                                            gfx::BufferUsage::UniformBuffer |
                                            gfx::BufferUsage::StorageBuffer})
               .unwrap();

  if (ssbo == nullptr)
  {
    ssbo =
        ctx.device->create_descriptor_set(ctx.device.self, ctx.ssbo_layout, {})
            .unwrap();
  }

  ctx.device->update_descriptor_set(
      ctx.device.self,
      gfx::DescriptorSetUpdate{
          .set     = ssbo,
          .binding = 0,
          .element = 0,
          .buffers = to_span({gfx::BufferBinding{
              .buffer = buffer, .offset = 0, .size = p_size}})});

  size = p_size;
}

void SSBO::copy(RenderContext &ctx, Span<u8 const> src, Span<char const> label)
{
  reserve(ctx, (u64) src.size(), label);
  u8 *data = (u8 *) map(ctx);
  mem::copy(src, data);
  flush(ctx);
  unmap(ctx);
}

void *SSBO::map(RenderContext &ctx)
{
  return ctx.device->map_buffer_memory(ctx.device.self, buffer).unwrap();
}

void SSBO::unmap(RenderContext &ctx)
{
  ctx.device->unmap_buffer_memory(ctx.device.self, buffer);
}

void SSBO::flush(RenderContext &ctx)
{
  ctx.device
      ->flush_mapped_buffer_memory(ctx.device.self, buffer,
                                   gfx::MemoryRange{0, gfx::WHOLE_SIZE})
      .unwrap();
}

void CanvasResources::uninit(RenderContext &ctx)
{
  vertices.uninit(ctx);
  indices.uninit(ctx);
  ngon_params.uninit(ctx);
  rrect_params.uninit(ctx);
}

void CanvasRenderer::init(RenderContext &)
{
}

void CanvasRenderer::uninit(RenderContext &ctx)
{
  for (u32 i = 0; i < ctx.buffering; i++)
  {
    resources[i].uninit(ctx);
  }
}

void CanvasRenderer::begin(RenderContext &ctx, PassContext &passes,
                           Canvas const &canvas, gfx::RenderingInfo const &,
                           gfx::DescriptorSet)
{
  (void) passes;
  CanvasResources &r = resources[ctx.ring_index()];
  r.vertices.copy(ctx, to_span(canvas.vertices).as_u8(),
                  "Canvas Vertices"_span);
  r.indices.copy(ctx, to_span(canvas.indices).as_u8(), "Canvas Indices"_span);
  r.ngon_params.copy(ctx, to_span(canvas.ngon_params).as_u8(),
                     "Ngon Params"_span);
  r.rrect_params.copy(ctx, to_span(canvas.rrect_params).as_u8(),
                      "RRect Params"_span);
}

constexpr RectU clip_to_scissor(gfx::Viewport const &viewport,
                                CRect const &clip, Vec2U surface_extent)
{
  Rect  rect{viewport.offset + clip.center - clip.extent / 2, clip.extent};
  Vec2I offset_i{(i32) rect.offset.x, (i32) rect.offset.y};
  Vec2I extent_i{(i32) rect.extent.x, (i32) rect.extent.y};

  RectU scissor{.offset = {(u32) max(0, offset_i.x), (u32) max(0, offset_i.y)},
                .extent = {(u32) max(0, extent_i.x), (u32) max(0, extent_i.y)}};

  scissor.offset.x = min(scissor.offset.x, surface_extent.x);
  scissor.offset.y = min(scissor.offset.y, surface_extent.y);
  scissor.extent.x = min(surface_extent.x - scissor.offset.x, scissor.extent.x);
  scissor.extent.y = min(surface_extent.y - scissor.offset.y, scissor.extent.y);

  return scissor;
}

void CanvasRenderer::render(RenderContext &ctx, PassContext &passes,
                            gfx::RenderingInfo const &info,
                            gfx::Viewport const      &viewport,
                            gfx::Extent               surface_extent,
                            gfx::DescriptorSet texture, Canvas const &canvas,
                            u32 first, u32 num)
{
  CanvasResources &r = resources[ctx.ring_index()];

  for (CanvasPassRun const &run : to_span(canvas.pass_runs).slice(first, num))
  {
    gfx::Rect scissor = clip_to_scissor(viewport, run.clip, surface_extent);
    switch (run.type)
    {
      case CanvasPassType::Blur:
      {
        for (CanvasBlurParam const &params :
             to_span(canvas.blur_params).slice(run.first, run.count))
        {
          passes.blur.add_pass(
              ctx, BlurPassParams{.image_view = info.color_attachments[0].view,
                                  .extent     = surface_extent,
                                  .texture_view = texture,
                                  .texture      = 0,
                                  .passes       = params.num_passes,
                                  .area = clip_to_scissor(viewport, params.area,
                                                          surface_extent)});
        }
      }
      break;
      case CanvasPassType::Custom:
      {
        for (CustomCanvasPass pass :
             to_span(canvas.custom_passes).slice(run.first, run.count))
        {
          pass(ctx, passes, info, texture);
        }
      }
      break;
      case CanvasPassType::Ngon:
      {
        passes.ngon.add_pass(
            ctx,
            NgonPassParams{.rendering_info = info,
                           .scissor        = scissor,
                           .viewport       = viewport,
                           .vertices_ssbo  = r.vertices.ssbo,
                           .indices_ssbo   = r.indices.ssbo,
                           .params_ssbo    = r.ngon_params.ssbo,
                           .textures       = ctx.texture_views,
                           .index_counts   = to_span(canvas.ngon_index_counts)
                                               .slice(run.first, run.count)});
      }
      break;
      case CanvasPassType::RRect:
      {
        passes.rrect.add_pass(
            ctx, RRectPassParams{.rendering_info = info,
                                 .scissor        = scissor,
                                 .viewport       = viewport,
                                 .params_ssbo    = r.rrect_params.ssbo,
                                 .textures       = ctx.texture_views,
                                 .first_instance = run.first,
                                 .num_instances  = run.count});
      }
      break;
      default:
        break;
    }
  }
}

}        // namespace ash