/// SPDX-License-Identifier: MIT
#pragma once

#define STBIRDEF extern "C" inline

#include "ashura/engine/font.h"
#include "ashura/engine/font_atlas.h"
#include "ashura/engine/rect_pack.h"
#include "ashura/gpu/image.h"
#include "ashura/std/allocator.h"
#include "ashura/std/allocators.h"
#include "ashura/std/error.h"
#include "ashura/std/range.h"
#include "ashura/std/types.h"

extern "C"
{
#include "freetype/freetype.h"
#include "freetype/ftsystem.h"
#include "hb-ft.h"
#include "hb.h"
}

namespace ash
{

struct FontImpl : Font
{
  Vec<char> font_data;

  /// @brief Postscript name, name of the font face, ASCII. i.e. RobotoBold
  Vec<char> postscript_name;

  /// @brief Font family name, ASCII. i.e. Roboto
  Vec<char> family_name;

  /// @brief Font family style name, ASCII. i.e. Bold
  Vec<char> style_name;

  hb_blob_t *hb_blob;

  hb_face_t *hb_face;

  hb_font_t *hb_font;

  FT_Library ft_lib;

  FT_Face ft_face;

  u32 face;

  Vec<Glyph> glyphs;

  u32 replacement_glyph;

  u32 ellipsis_glyph;

  u32 space_glyph;

  FontMetrics metrics;

  Option<CpuFontAtlas> cpu_atlas = None;

  Option<GpuFontAtlas> gpu_atlas = None;

  FontImpl(Vec<char> font_data, Vec<char> postscript_name,
           Vec<char> family_name, Vec<char> style_name, hb_blob_t *hb_blob,
           hb_face_t *hb_face, hb_font_t *hb_font, FT_Library ft_lib,
           FT_Face ft_face, u32 face, Vec<Glyph> glyphs, u32 replacement_glyph,
           u32 ellipsis_glyph, u32 space_glyph, FontMetrics metrics) :
      font_data{std::move(font_data)},
      postscript_name{std::move(postscript_name)},
      family_name{std::move(family_name)},
      style_name{std::move(style_name)},
      hb_blob{hb_blob},
      hb_face{hb_face},
      hb_font{hb_font},
      ft_lib{ft_lib},
      ft_face{ft_face},
      face{face},
      glyphs{std::move(glyphs)},
      replacement_glyph{replacement_glyph},
      ellipsis_glyph{ellipsis_glyph},
      space_glyph{space_glyph},
      metrics{metrics}
  {
  }

  FontImpl(FontImpl const &) = delete;

  FontImpl(FontImpl &&) = delete;

  FontImpl &operator=(FontImpl const &) = delete;

  FontImpl &operator=(FontImpl &&) = delete;

  virtual ~FontImpl() override
  {
    gpu_atlas.expect_none("GPU font atlas has not been unloaded");
    hb_font_destroy(hb_font);
    hb_face_destroy(hb_face);
    hb_blob_destroy(hb_blob);
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_lib);
  }

  virtual FontInfo info() override
  {
    FontInfo info{.postscript_name   = postscript_name,
                  .family_name       = family_name,
                  .style_name        = style_name,
                  .glyphs            = glyphs,
                  .replacement_glyph = replacement_glyph,
                  .space_glyph       = space_glyph,
                  .ellipsis_glyph    = ellipsis_glyph,
                  .metrics           = metrics};

    if (cpu_atlas.is_some())
    {
      info.cpu_atlas = Some<CpuFontAtlas const *>{&cpu_atlas.value()};
    }

    if (gpu_atlas.is_some())
    {
      info.gpu_atlas = Some<GpuFontAtlas const *>{&gpu_atlas.value()};
    }

    return info;
  }

  virtual Result<> rasterize(u32 font_height, AllocatorImpl allocator) override
  {
    constexpr u32 MIN_ATLAS_EXTENT = 512;
    static_assert(MIN_ATLAS_EXTENT > 0, "Font atlas extent must be non-zero");
    static_assert(MIN_ATLAS_EXTENT > 128,
                  "Font atlas extent must be at least 128px");
    static_assert(MIN_ATLAS_EXTENT % 64 == 0,
                  "Font atlas extent should be a multiple of 64");
    static_assert(MIN_ATLAS_EXTENT <= gpu::MAX_IMAGE_EXTENT_2D,
                  "Font atlas extent too large for GPU platform");
    CHECK(font_height <= 1024);
    CHECK(font_height <= MIN_ATLAS_EXTENT / 8);

    Vec2U atlas_extent{MIN_ATLAS_EXTENT, MIN_ATLAS_EXTENT};

    cpu_atlas.expect_none("CPU font atlas has already been loaded");

    CpuFontAtlas atlas;

    if (!atlas.glyphs.resize_defaulted(glyphs.size32()))
    {
      return Err{};
    }

    if (FT_Set_Pixel_Sizes(ft_face, font_height, font_height) != 0)
    {
      return Err{};
    }

    u32 num_rasterized_glyphs = 0;

    for (u32 i = 0; i < glyphs.size32(); i++)
    {
      if (glyphs[i].is_valid)
      {
        FT_Error ft_error = FT_Load_Glyph(ft_face, i, FT_LOAD_DEFAULT);
        if (ft_error != 0)
        {
          continue;
        }

        Vec2U extent{ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows};

        if (extent.x == 0 || extent.y == 0)
        {
          continue;
        }

        atlas.glyphs[i].area.extent = extent;

        num_rasterized_glyphs++;
      }
    }

    u32 num_layers = 0;
    {
      Vec<rect_pack::rect> rects{allocator};

      if (!rects.resize_uninit(num_rasterized_glyphs))
      {
        return Err{};
      }

      for (u32 g = 0, irect = 0; g < glyphs.size32(); g++)
      {
        Glyph const      &gl  = glyphs[g];
        AtlasGlyph const &agl = atlas.glyphs[g];
        // only assign packing rects to the valid glyphs
        if (gl.is_valid && agl.area.extent.x != 0 && agl.area.extent.y != 0)
        {
          rect_pack::rect &r = rects[irect];
          r.glyph_index      = g;
          r.x                = 0;
          r.y                = 0;
          // added padding to avoid texture spilling due to accumulated
          // floating-point uv interpolation errors
          r.w            = (i32) (agl.area.extent.x + 2);
          r.h            = (i32) (agl.area.extent.y + 2);
          atlas_extent.x = max(atlas_extent.x, agl.area.extent.x + 2);
          atlas_extent.y = max(atlas_extent.y, agl.area.extent.y + 2);
          irect++;
        }
      }

      CHECK(atlas_extent.x <= gpu::MAX_IMAGE_EXTENT_2D);
      CHECK(atlas_extent.y <= gpu::MAX_IMAGE_EXTENT_2D);

      Vec<rect_pack::Node> nodes{allocator};

      if (!nodes.resize_uninit(atlas_extent.x))
      {
        return Err{};
      }

      u32  num_packed = 0;
      bool all_packed = false;

      while (!all_packed)
      {
        // tries to pack all the glyph rects into the provided extent
        rect_pack::Context pack_context = rect_pack::init(
            atlas_extent.x, atlas_extent.y, nodes.data(), atlas_extent.x, true);
        all_packed =
            rect_pack::pack_rects(pack_context, rects.data() + num_packed,
                                  num_rasterized_glyphs - num_packed);
        auto [just_packed, unpacked] =
            partition(span(rects).slice(num_packed),
                      [](rect_pack::rect const &r) { return r.was_packed; });
        for (u32 i = num_packed; i < (num_packed + just_packed.span); i++)
        {
          rects[i].layer = num_layers;
        }
        CHECK(just_packed.span != 0);
        num_packed += (u32) just_packed.span;
        num_layers++;
      }

      // sanity check. ideally all should have been packed
      CHECK(all_packed);

      for (u32 i = 0; i < num_rasterized_glyphs; i++)
      {
        rect_pack::rect r = rects[i];
        AtlasGlyph     &g = atlas.glyphs[r.glyph_index];
        g.area.offset.x   = (u32) r.x + 1;
        g.area.offset.y   = (u32) r.y + 1;
        g.layer           = r.layer;
        g.uv[0]           = as_vec2(g.area.offset) / as_vec2(atlas_extent);
        g.uv[1]           = as_vec2(g.area.end()) / as_vec2(atlas_extent);
      }
    }

    u64 const atlas_area       = (u64) atlas_extent.x * (u64) atlas_extent.y;
    u64 const atlas_layer_size = atlas_area;
    u64 const atlas_size       = atlas_layer_size * num_layers;

    if (!atlas.channels.resize_defaulted(atlas_size))
    {
      return Err{};
    }

    ImageLayerSpan<u8, 1> atlas_span{.channels = atlas.channels,
                                     .width    = atlas_extent.x,
                                     .height   = atlas_extent.y,
                                     .layers   = num_layers};

    for (u32 i = 0; i < glyphs.size32(); i++)
    {
      Glyph const      &g  = glyphs[i];
      AtlasGlyph const &ag = atlas.glyphs[i];
      if (g.is_valid)
      {
        FT_GlyphSlot slot     = ft_face->glyph;
        FT_Error     ft_error = FT_Load_Glyph(
            ft_face, i, FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
        if (ft_error != 0)
        {
          continue;
        }

        CHECK(slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
        /// we don't want to handle negative pitches
        CHECK(slot->bitmap.pitch >= 0);

        ImageSpan<u8 const, 1> src{
            .channels = {slot->bitmap.buffer,
                         slot->bitmap.rows * (u32) slot->bitmap.pitch},
            .width    = slot->bitmap.width,
            .height   = slot->bitmap.rows,
            .stride   = (u32) slot->bitmap.pitch};

        copy_image(src, atlas_span.get_layer(ag.layer).slice(ag.area.offset,
                                                             ag.area.extent));
      }
    }

    atlas.font_height = font_height;
    atlas.extent      = atlas_extent;
    atlas.num_layers  = num_layers;

    cpu_atlas = Some{std::move(atlas)};

    return Ok{};
  }

  virtual void upload_to_device(GpuContext &c, AllocatorImpl allocator) override
  {
    gpu::CommandEncoder &enc = c.encoder();
    gpu::Device         &dev = *c.device;

    CHECK(cpu_atlas.is_some());
    CHECK(gpu_atlas.is_none());

    CpuFontAtlas const &atlas = cpu_atlas.value();

    CHECK(atlas.num_layers > 0);
    CHECK(atlas.extent.x > 0);
    CHECK(atlas.extent.y > 0);

    gpu::Image image =
        dev.create_image(
               gpu::ImageInfo{.label  = "Font Atlas Image"_str,
                              .type   = gpu::ImageType::Type2D,
                              .format = gpu::Format::B8G8R8A8_UNORM,
                              .usage  = gpu::ImageUsage::Sampled |
                                       gpu::ImageUsage::InputAttachment |
                                       gpu::ImageUsage::Storage |
                                       gpu::ImageUsage::TransferSrc |
                                       gpu::ImageUsage::TransferDst,
                              .aspects    = gpu::ImageAspects::Color,
                              .extent     = {atlas.extent.x, atlas.extent.y, 1},
                              .mip_levels = 1,
                              .array_layers = atlas.num_layers,
                              .sample_count = gpu::SampleCount::Count1})
            .unwrap();

    Vec<gpu::ImageView> views;

    views.resize_defaulted(atlas.num_layers).unwrap();

    for (u32 i = 0; i < atlas.num_layers; i++)
    {
      views[i] =
          dev.create_image_view(
                 gpu::ImageViewInfo{.label       = "Font Atlas Image View"_str,
                                    .image       = image,
                                    .view_type   = gpu::ImageViewType::Type2D,
                                    .view_format = gpu::Format::B8G8R8A8_UNORM,
                                    .mapping     = {},
                                    .aspects     = gpu::ImageAspects::Color,
                                    .first_mip_level   = 0,
                                    .num_mip_levels    = 1,
                                    .first_array_layer = i,
                                    .num_array_layers  = 1})
              .unwrap();
    }

    u64 const   atlas_size = atlas.channels.size() * (u64) 4;
    gpu::Buffer staging_buffer =
        dev.create_buffer(
               gpu::BufferInfo{.label       = "Font Atlas Staging Buffer"_str,
                               .size        = atlas_size,
                               .host_mapped = true,
                               .usage       = gpu::BufferUsage::TransferSrc |
                                        gpu::BufferUsage::TransferDst})
            .unwrap();

    u8 *map = (u8 *) dev.map_buffer_memory(staging_buffer).unwrap();

    ImageLayerSpan<u8, 4> dst{.channels = {map, atlas_size},
                              .width    = atlas.extent.x,
                              .height   = atlas.extent.y,
                              .layers   = atlas.num_layers};

    for (u32 i = 0; i < atlas.num_layers; i++)
    {
      copy_alpha_image_to_BGRA(cpu_atlas.value().span().get_layer(i).as_const(),
                               dst.get_layer(i), U8_MAX, U8_MAX, U8_MAX);
    }

    dev.flush_mapped_buffer_memory(staging_buffer,
                                   {.offset = 0, .size = gpu::WHOLE_SIZE})
        .unwrap();
    dev.unmap_buffer_memory(staging_buffer);

    Vec<gpu::BufferImageCopy> copies{allocator};

    copies.resize_uninit(atlas.num_layers).unwrap();

    for (u32 layer = 0; layer < atlas.num_layers; layer++)
    {
      u64 offset =
          (u64) atlas.extent.x * (u64) atlas.extent.y * 4 * (u64) layer;
      copies[layer] = gpu::BufferImageCopy{
          .buffer_offset       = offset,
          .buffer_row_length   = atlas.extent.x,
          .buffer_image_height = atlas.extent.y,
          .image_layers        = {.aspects           = gpu::ImageAspects::Color,
                                  .mip_level         = 0,
                                  .first_array_layer = layer,
                                  .num_array_layers  = 1},
          .image_offset        = {0, 0, 0},
          .image_extent        = {atlas.extent.x, atlas.extent.y, 1}};
    }

    enc.copy_buffer_to_image(staging_buffer, image, copies);

    gpu::Format format = gpu::Format::B8G8R8A8_UNORM;
    c.release(staging_buffer);

    Vec<u32>        textures;
    Vec<AtlasGlyph> glyphs;

    textures.resize_defaulted(atlas.num_layers).unwrap();
    glyphs.extend_copy(atlas.glyphs).unwrap();

    for (u32 i = 0; i < atlas.num_layers; i++)
    {
      textures[i] = c.alloc_texture_slot();
      dev.update_descriptor_set(gpu::DescriptorSetUpdate{
          .set     = c.texture_views,
          .binding = 0,
          .element = textures[i],
          .images  = span({gpu::ImageBinding{.image_view = views[i]}})});
    }

    gpu_atlas = Some{GpuFontAtlas{.image       = image,
                                  .views       = std::move(views),
                                  .textures    = std::move(textures),
                                  .font_height = atlas.font_height,
                                  .num_layers  = atlas.num_layers,
                                  .extent      = atlas.extent,
                                  .glyphs      = std::move(glyphs),
                                  .format      = format}};
  }

  virtual void unload_from_device(GpuContext &c) override
  {
    CHECK_DESC(gpu_atlas.is_some(),
               "Requested font to be unloaded from GPU with no GPU atlas");
    for (u32 slot : gpu_atlas.value().textures)
    {
      c.release_texture_slot(slot);
    }

    for (gpu::ImageView view : gpu_atlas.value().views)
    {
      c.release(view);
    }
    c.release(gpu_atlas.value().image);

    gpu_atlas = None;
  }
};

}        // namespace ash
