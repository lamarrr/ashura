/// SPDX-License-Identifier: MIT
#include "ashura/engine/font.h"
#include "ashura/std/error.h"

namespace ash
{

void FontAtlasResource::init(RenderContext &c, FontAtlas const &atlas,
                             AllocatorImpl const &allocator)
{
  gfx::CommandEncoderImpl const &enc = c.encoder();
  gfx::DeviceImpl const         &d   = c.device;
  CHECK(atlas.num_layers > 0);
  CHECK(atlas.extent.x > 0);
  CHECK(atlas.extent.y > 0);

  image =
      d->create_image(
           d.self, gfx::ImageDesc{.label  = "Font Atlas Image"_span,
                                  .type   = gfx::ImageType::Type2D,
                                  .format = gfx::Format::B8G8R8A8_UNORM,
                                  .usage  = gfx::ImageUsage::Sampled |
                                           gfx::ImageUsage::InputAttachment |
                                           gfx::ImageUsage::Storage |
                                           gfx::ImageUsage::TransferSrc |
                                           gfx::ImageUsage::TransferDst,
                                  .aspects = gfx::ImageAspects::Color,
                                  .extent = {atlas.extent.x, atlas.extent.y, 1},
                                  .mip_levels   = 1,
                                  .array_layers = atlas.num_layers,
                                  .sample_count = gfx::SampleCount::Count1})
          .unwrap();

  CHECK(views.resize_defaulted(atlas.num_layers));

  for (u32 i = 0; i < atlas.num_layers; i++)
  {
    views[i] =
        d->create_image_view(
             d.self,
             gfx::ImageViewDesc{.label           = "Font Atlas Image View"_span,
                                .image           = image,
                                .view_type       = gfx::ImageViewType::Type2D,
                                .view_format     = gfx::Format::B8G8R8A8_UNORM,
                                .mapping         = {},
                                .aspects         = gfx::ImageAspects::Color,
                                .first_mip_level = 0,
                                .num_mip_levels  = 1,
                                .first_array_layer = i,
                                .num_array_layers  = 1})
            .unwrap();
  }

  gfx::BufferImageCopy *copies;

  CHECK(allocator.nalloc(atlas.num_layers, &copies));

  defer copies_del{[&] { allocator.ndealloc(copies, atlas.num_layers); }};

  u64 const   atlas_size = atlas.channels.size() * (u64) 4;
  gfx::Buffer staging_buffer =
      d->create_buffer(
           d.self, gfx::BufferDesc{.label = "Font Atlas Staging Buffer"_span,
                                   .size  = atlas_size,
                                   .host_mapped = true,
                                   .usage = gfx::BufferUsage::TransferSrc |
                                            gfx::BufferUsage::TransferDst})
          .unwrap();

  u8 *map = (u8 *) d->map_buffer_memory(d.self, staging_buffer).unwrap();

  ImageLayerSpan<u8, 4> dst{.channels = {map, atlas_size},
                            .width    = atlas.extent.x,
                            .height   = atlas.extent.y,
                            .layers   = atlas.num_layers};

  for (u32 i = 0; i < atlas.num_layers; i++)
  {
    copy_alpha_image_to_BGRA(atlas.span().get_layer(i).as_const(),
                             dst.get_layer(i), U8_MAX, U8_MAX, U8_MAX);
  }

  d->flush_mapped_buffer_memory(d.self, staging_buffer,
                                {.offset = 0, .size = gfx::WHOLE_SIZE})
      .unwrap();
  d->unmap_buffer_memory(d.self, staging_buffer);

  for (u32 layer = 0; layer < atlas.num_layers; layer++)
  {
    u64 offset = (u64) atlas.extent.x * (u64) atlas.extent.y * 4 * (u64) layer;
    copies[layer] = gfx::BufferImageCopy{
        .buffer_offset       = offset,
        .buffer_row_length   = atlas.extent.x,
        .buffer_image_height = atlas.extent.y,
        .image_layers        = {.aspects           = gfx::ImageAspects::Color,
                                .mip_level         = 0,
                                .first_array_layer = layer,
                                .num_array_layers  = 1},
        .image_offset        = {0, 0, 0},
        .image_extent        = {atlas.extent.x, atlas.extent.y, 1}};
  }

  enc->copy_buffer_to_image(enc.self, staging_buffer, image,
                            Span{copies, atlas.num_layers});

  format = gfx::Format::B8G8R8A8_UNORM;
  c.release(staging_buffer);

  CHECK(textures.resize_defaulted(atlas.num_layers));
  CHECK(glyphs.extend_copy(span(atlas.glyphs)));

  for (u32 i = 0; i < atlas.num_layers; i++)
  {
    textures[i] = c.alloc_texture_slot();
    d->update_descriptor_set(
        d.self,
        gfx::DescriptorSetUpdate{
            .set     = c.texture_views,
            .binding = 0,
            .element = textures[i],
            .images  = span({gfx::ImageBinding{.image_view = views[i]}})});
  }
}

void FontAtlasResource::release(RenderContext &c)
{
  for (u32 slot : textures)
  {
    c.release_texture_slot(slot);
  }

  for (gfx::ImageView view : views)
  {
    c.release(view);
  }
  c.release(image);

  uninit();
}

}        // namespace ash
