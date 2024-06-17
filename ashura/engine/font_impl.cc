
#define STBIRDEF extern "C" inline

#include "ashura/engine/font_impl.h"
#include "ashura/engine/font.h"
#include "ashura/gfx/image.h"
#include "ashura/std/allocator.h"
#include "ashura/std/arena_allocator.h"
#include "ashura/std/error.h"
#include "ashura/std/io.h"
#include "ashura/std/range.h"
#include "ashura/std/types.h"
#include "ashura/utils/rect_pack.h"

namespace ash
{

static_assert(PT_UNIT % 64 == 0);

constexpr u32 FONT_ATLAS_EXTENT = 512;

static_assert(FONT_ATLAS_EXTENT != 0);
static_assert(FONT_ATLAS_EXTENT > 128);
static_assert(FONT_ATLAS_EXTENT % 64 == 0);
static_assert(FONT_ATLAS_EXTENT <= gfx::MAX_IMAGE_EXTENT_2D);

bool load_font(Span<u8 const> encoded, u32 face,
               Span<UnicodeRange const> ranges, AllocatorImpl const &allocator,
               FontImpl **f)
{
  u32   font_data_size = (u32) encoded.size();
  char *font_data;
  if (!allocator.nalloc(font_data_size, &font_data))
  {
    return false;
  }

  defer font_data_del{[&] {
    if (font_data != nullptr)
    {
      allocator.ndealloc(font_data, font_data_size);
    }
  }};

  mem::copy(encoded.as_char(), font_data);

  hb_blob_t *hb_blob = hb_blob_create(
      font_data, font_data_size, HB_MEMORY_MODE_READONLY, nullptr, nullptr);

  if (hb_blob == nullptr)
  {
    return false;
  }

  defer hb_blob_del{[&] {
    if (hb_blob != nullptr)
    {
      hb_blob_destroy(hb_blob);
    }
  }};

  u32 num_faces = hb_face_count(hb_blob);

  if (face >= num_faces)
  {
    return false;
  }

  hb_face_t *hb_face = hb_face_create(hb_blob, face);

  if (hb_face == nullptr)
  {
    return false;
  }

  defer hb_face_del{[&] {
    if (hb_face != nullptr)
    {
      hb_face_destroy(hb_face);
    }
  }};

  hb_font_t *hb_font = hb_font_create(hb_face);

  if (hb_font == nullptr)
  {
    return false;
  }

  hb_font_set_scale(hb_font, PT_UNIT, PT_UNIT);

  defer hb_font_del{[&] {
    if (hb_font != nullptr)
    {
      hb_font_destroy(hb_font);
    }
  }};

  FT_Library ft_lib;
  if (FT_Init_FreeType(&ft_lib) != 0)
  {
    return false;
  }

  defer ft_lib_del{[&] {
    if (ft_lib != nullptr)
    {
      FT_Done_FreeType(ft_lib);
    }
  }};

  FT_Face ft_face;

  if (FT_New_Memory_Face(ft_lib, (FT_Byte const *) font_data,
                         (FT_Long) font_data_size, 0, &ft_face) != 0)
  {
    return false;
  }

  if (FT_Set_Char_Size(ft_face, PT_UNIT, PT_UNIT, 300, 300) != 0)
  {
    return false;
  }

  defer ft_face_del{[&] {
    if (ft_face != nullptr)
    {
      FT_Done_Face(ft_face);
    }
  }};

  char const *ft_postscript_name   = FT_Get_Postscript_Name(ft_face);
  char       *postscript_name      = nullptr;
  usize       postscript_name_size = 0;

  if (ft_postscript_name != nullptr)
  {
    postscript_name_size = strlen(ft_postscript_name);
    if (!allocator.nalloc(postscript_name_size, &postscript_name))
    {
      return false;
    }
    mem::copy(ft_postscript_name, postscript_name, postscript_name_size);
  }

  defer postscript_name_del{[&] {
    if (postscript_name != nullptr)
    {
      allocator.ndealloc(postscript_name, postscript_name_size);
    }
  }};

  char *family_name      = nullptr;
  usize family_name_size = 0;

  if (ft_face->family_name != nullptr)
  {
    family_name_size = strlen(ft_face->family_name);
    if (!allocator.nalloc(family_name_size, &family_name))
    {
      return false;
    }
    mem::copy(ft_face->family_name, family_name, family_name_size);
  }

  defer family_name_del{[&] {
    if (family_name != nullptr)
    {
      allocator.ndealloc(family_name, family_name_size);
    }
  }};

  char *style_name      = nullptr;
  usize style_name_size = 0;

  if (ft_face->style_name != nullptr)
  {
    style_name_size = strlen(ft_face->style_name);
    if (!allocator.nalloc(style_name_size, &style_name))
    {
      return false;
    }
    mem::copy(ft_face->style_name, style_name, style_name_size);
  }

  defer style_name_del{[&] {
    if (style_name != nullptr)
    {
      allocator.ndealloc(style_name, style_name_size);
    }
  }};

  u32 const num_glyphs = (u32) ft_face->num_glyphs;
  // glyph 0 is selected if the replacement codepoint glyph is not found
  u32 const replacement_glyph = FT_Get_Char_Index(ft_face, 0xFFFD);
  u32 const ellipsis_glyph    = FT_Get_Char_Index(ft_face, 0x2026);
  u32 const space_glyph       = FT_Get_Char_Index(ft_face, ' ');

  // expressed on a PT_UNIT scale
  i32 const ascent  = ft_face->size->metrics.ascender;
  i32 const descent = ft_face->size->metrics.descender;
  i32 const advance = ft_face->size->metrics.max_advance;

  Glyph *glyphs;
  if (!allocator.nalloc(num_glyphs, &glyphs))
  {
    return false;
  }

  defer glyphs_del{[&] {
    if (glyphs != nullptr)
    {
      allocator.ndealloc(glyphs, num_glyphs);
    }
  }};

  for (u32 glyph_idx = 0; glyph_idx < num_glyphs; glyph_idx++)
  {
    bool const is_needed =
        glyph_idx == replacement_glyph ? true : ranges.is_empty();

    if (FT_Load_Glyph(ft_face, glyph_idx, FT_LOAD_DEFAULT) == 0)
    {
      FT_GlyphSlot s = ft_face->glyph;

      GlyphMetrics m;

      // expressed on a PT_UNIT scale
      m.bearing.x = s->metrics.horiBearingX;
      m.bearing.y = s->metrics.horiBearingY;
      m.advance   = s->metrics.horiAdvance;
      m.extent.x  = s->metrics.width;
      m.extent.y  = s->metrics.height;
      m.descent   = max(m.extent.y - m.bearing.y, 0);

      // bin offsets are determined after binning and during rect packing
      glyphs[glyph_idx] =
          Glyph{.is_valid  = true,
                .is_needed = is_needed,
                .metrics   = m,
                .layer     = 0,
                .area      = {.offset = Vec2U{0, 0},
                              .extent = Vec2U{s->bitmap.width, s->bitmap.rows}}};
    }
    else
    {
      glyphs[glyph_idx] = Glyph{.is_valid  = false,
                                .is_needed = is_needed,
                                .metrics   = {},
                                .layer     = 0,
                                .area      = {}};
    }
  }

  {
    // Iterate through all the characters in the font's CMAP that map a unicode
    // codepoint to a glyph
    FT_UInt  glyph_idx    = 0;
    FT_ULong unicode_char = FT_Get_First_Char(ft_face, &glyph_idx);
    do
    {
      for (UnicodeRange r : ranges)
      {
        if (unicode_char >= r.first && unicode_char <= r.last)
        {
          glyphs[glyph_idx].is_needed = true;
          break;
        }
      }

      unicode_char = FT_Get_Next_Char(ft_face, unicode_char, &glyph_idx);
    } while (glyph_idx != 0);
  }

  if (!allocator.nalloc(1, f))
  {
    return false;
  }

  new (*f) FontImpl{.allocator            = allocator,
                    .postscript_name      = postscript_name,
                    .postscript_name_size = postscript_name_size,
                    .family_name          = family_name,
                    .family_name_size     = family_name_size,
                    .style_name           = style_name,
                    .style_name_size      = style_name_size,
                    .hb_blob              = hb_blob,
                    .hb_face              = hb_face,
                    .hb_font              = hb_font,
                    .ft_lib               = ft_lib,
                    .ft_face              = ft_face,
                    .face                 = face,
                    .font_data            = font_data,
                    .font_data_size       = font_data_size,
                    .num_glyphs           = num_glyphs,
                    .replacement_glyph    = replacement_glyph,
                    .ellipsis_glyph       = ellipsis_glyph,
                    .space_glyph          = space_glyph,
                    .glyphs               = glyphs,
                    .ascent               = ascent,
                    .descent              = descent,
                    .advance              = advance};

  postscript_name = nullptr;
  family_name     = nullptr;
  style_name      = nullptr;
  hb_blob         = nullptr;
  hb_face         = nullptr;
  hb_font         = nullptr;
  ft_lib          = nullptr;
  ft_face         = nullptr;
  font_data       = nullptr;
  glyphs          = nullptr;

  return true;
}

void destroy_font(FontImpl *f, RenderContext &c)
{
  hb_blob_destroy(f->hb_blob);
  hb_face_destroy(f->hb_face);
  hb_font_destroy(f->hb_font);
  FT_Done_Face(f->ft_face);
  FT_Done_FreeType(f->ft_lib);
  f->allocator.ndealloc(f->postscript_name, f->postscript_name_size);
  f->allocator.ndealloc(f->family_name, f->family_name_size);
  f->allocator.ndealloc(f->style_name, f->style_name_size);
  f->allocator.ndealloc(f->glyphs, f->num_glyphs);
  f->allocator.ndealloc(f->font_data, f->font_data_size);
  for (u32 i = 0; i < f->num_layers; i++)
  {
    c.device->destroy_image_view(c.device.self, f->views[i]);
  }
  c.device->destroy_image(c.device.self, f->image);

  f->allocator.ndealloc(f->atlas, f->atlas_size);

  if (f->views != nullptr)
  {
    f->allocator.ndealloc(f->views, f->num_layers);
  }

  if (f->textures != nullptr)
  {
    for (u32 i = 0; i < f->num_layers; i++)
    {
      c.release_texture_slot(f->textures[i]);
    }

    f->allocator.ndealloc(f->textures, f->num_layers);
  }
  f->allocator.ndealloc(f, 1);
}

bool rasterize_font(FontImpl &f, i32 font_height,
                    AllocatorImpl const &allocator)
{
  CHECK(font_height <= 1024);
  CHECK(font_height <= (i32) (FONT_ATLAS_EXTENT / 2 - 16));

  if (FT_Set_Pixel_Sizes(f.ft_face, font_height, font_height) != 0)
  {
    return false;
  }

  u32 num_loaded_glyphs = 0;

  for (u32 i = 0; i < f.num_glyphs; i++)
  {
    if (f.glyphs[i].is_valid && f.glyphs[i].is_needed)
    {
      num_loaded_glyphs++;
    }
  }

  u32 num_layers = 0;
  {
    rect_pack::rect *rects;
    if (!allocator.nalloc(num_loaded_glyphs, &rects))
    {
      return false;
    }

    defer rects_del{[&] { allocator.ndealloc(rects, num_loaded_glyphs); }};

    for (u32 glyph_idx = 0, irect = 0; glyph_idx < f.num_glyphs; glyph_idx++)
    {
      Glyph const &g = f.glyphs[glyph_idx];
      // only assign packing rects to the valid and needed glyphs
      if (g.is_valid && g.is_needed)
      {
        rect_pack::rect &r = rects[irect];
        r.glyph_index      = glyph_idx;
        r.x                = 0;
        r.y                = 0;
        // added padding to avoid texture spilling due to accumulated
        // floating-point uv interpolation errors
        r.w = (i32) (g.area.extent.x + 2);
        r.h = (i32) (g.area.extent.y + 2);
        irect++;
      }
    }

    rect_pack::Node *nodes;
    if (!allocator.nalloc(FONT_ATLAS_EXTENT, &nodes))
    {
      return false;
    }

    defer nodes_del{[&] { allocator.ndealloc(nodes, FONT_ATLAS_EXTENT); }};

    u32  num_packed = 0;
    bool all_packed = false;

    while (!all_packed)
    {
      // tries to pack all the glyph rects into the provided extent
      rect_pack::Context pack_context = rect_pack::init(
          FONT_ATLAS_EXTENT, FONT_ATLAS_EXTENT, nodes, FONT_ATLAS_EXTENT, true);
      all_packed = rect_pack::pack_rects(pack_context, rects + num_packed,
                                         num_loaded_glyphs - num_packed);
      auto [just_packed, unpacked] =
          partition(Span{rects, num_loaded_glyphs}.slice(num_packed),
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

    for (u32 i = 0; i < num_loaded_glyphs; i++)
    {
      rect_pack::rect r = rects[i];
      Glyph          &g = f.glyphs[r.glyph_index];
      g.area.offset.x   = (u32) r.x + 1;
      g.area.offset.y   = (u32) r.y + 1;
      g.layer           = r.layer;
      g.uv[0]           = Vec2{g.area.offset.x / (f32) FONT_ATLAS_EXTENT,
                     g.area.offset.y / (f32) FONT_ATLAS_EXTENT};
      g.uv[1] =
          Vec2{(g.area.offset.x + g.area.extent.x) / (f32) FONT_ATLAS_EXTENT,
               (g.area.offset.y + g.area.extent.y) / (f32) FONT_ATLAS_EXTENT};
    }
  }

  u64 const atlas_area  = (u64) FONT_ATLAS_EXTENT * (u64) FONT_ATLAS_EXTENT;
  u32 const atlas_pitch = FONT_ATLAS_EXTENT * 4;
  u64 const atlas_layer_size = atlas_area * 4;
  u64 const atlas_size       = atlas_layer_size * num_layers;
  u8       *atlas;

  if (!f.allocator.nalloc_zeroed(atlas_size, &atlas))
  {
    return false;
  }

  defer atlas_buffer_del{[&] {
    if (atlas != nullptr)
    {
      f.allocator.ndealloc(atlas, atlas_size);
    }
  }};

  for (u32 glyph_idx = 0; glyph_idx < f.num_glyphs; glyph_idx++)
  {
    Glyph const &g = f.glyphs[glyph_idx];
    if (g.is_valid && g.is_needed)
    {
      FT_Error ft_error = FT_Load_Glyph(f.ft_face, glyph_idx, FT_LOAD_DEFAULT);
      if (ft_error != 0)
      {
        continue;
      }

      FT_GlyphSlot slot = f.ft_face->glyph;
      ft_error          = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
      if (ft_error != 0)
      {
        continue;
      }

      CHECK(slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
      /// we don't want to handle negative pitches
      CHECK(slot->bitmap.pitch >= 0);

      gfx::ImageSpan<u8 const> src{
          .span = {slot->bitmap.buffer, slot->bitmap.rows * slot->bitmap.pitch},
          .format = gfx::Format::R8_UNORM,
          .pitch  = (u32) slot->bitmap.pitch,
          .width  = slot->bitmap.width,
          .height = slot->bitmap.rows};

      gfx::ImageSpan<u8> dst{
          .span   = {atlas + atlas_layer_size * g.layer, atlas_layer_size},
          .format = gfx::Format::B8G8R8A8_UNORM,
          .pitch  = atlas_pitch,
          .width  = FONT_ATLAS_EXTENT,
          .height = FONT_ATLAS_EXTENT};

      gfx::copy_alpha_image_to_BGRA(src, dst, U8_MAX, U8_MAX, U8_MAX);
    }
  }

  f.font_height  = font_height;
  f.atlas        = atlas;
  f.atlas_size   = atlas_size;
  f.atlas_extent = FONT_ATLAS_EXTENT;
  f.num_layers   = num_layers;
  atlas          = nullptr;

  return true;
}

void upload_font_to_device(FontImpl &f, RenderContext &c,
                           AllocatorImpl const &allocator)
{
  gfx::CommandEncoderImpl const &enc = c.encoder();
  gfx::DeviceImpl const         &d   = c.device;
  CHECK(f.num_layers > 0);
  u32 const  extent = f.atlas_extent;
  gfx::Image image =
      d->create_image(d.self,
                      gfx::ImageDesc{.label = "Font Atlas Image"_span,
                                     .type  = gfx::ImageType::Type2D,
                                     .usage = gfx::ImageUsage::Sampled |
                                              gfx::ImageUsage::InputAttachment |
                                              gfx::ImageUsage::Storage |
                                              gfx::ImageUsage::TransferSrc |
                                              gfx::ImageUsage::TransferDst,
                                     .aspects      = gfx::ImageAspects::Color,
                                     .extent       = {extent, extent, 1},
                                     .mip_levels   = 1,
                                     .array_layers = f.num_layers,
                                     .sample_count = gfx::SampleCount::Count1})
          .unwrap();

  gfx::ImageView *views;
  CHECK(f.allocator.nalloc(f.num_layers, &views));

  for (u32 i = 0; i < f.num_layers; i++)
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

  CHECK(allocator.nalloc(f.num_layers, &copies));

  defer copies_del{[&] { allocator.ndealloc(copies, f.num_layers); }};

  gfx::Buffer staging_buffer =
      d->create_buffer(
           d.self, gfx::BufferDesc{.label = "Font Atlas Staging Buffer"_span,
                                   .size  = f.atlas_size,
                                   .host_mapped = true,
                                   .usage = gfx::BufferUsage::TransferSrc |
                                            gfx::BufferUsage::TransferDst})
          .unwrap();

  u8 *map = (u8 *) d->map_buffer_memory(d.self, staging_buffer).unwrap();
  mem::copy(f.atlas, map, f.atlas_size);
  d->flush_mapped_buffer_memory(d.self, staging_buffer,
                                {.offset = 0, .size = gfx::WHOLE_SIZE})
      .unwrap();
  d->unmap_buffer_memory(d.self, staging_buffer);

  for (u32 i = 0; i < f.num_layers; i++)
  {
    u64 offset = (u64) extent * (u64) extent * 4 * (u64) i;
    copies[i]  = gfx::BufferImageCopy{
         .buffer_offset       = offset,
         .buffer_row_length   = 0,
         .buffer_image_height = 0,
         .image_layers        = {.aspects           = gfx::ImageAspects::Color,
                                 .mip_level         = 0,
                                 .first_array_layer = i,
                                 .num_array_layers  = 1},
         .image_offset        = {0, 0, 0},
         .image_extent        = {extent, extent, 1}};
  }

  enc->copy_buffer_to_image(enc.self, staging_buffer, image,
                            Span{copies, f.num_layers});

  f.views  = views;
  f.format = gfx::Format::B8G8R8A8_UNORM;
  c.release(staging_buffer);

  CHECK(allocator.nalloc(f.num_layers, &f.textures));

  for (u32 i = 0; i < f.num_layers; i++)
  {
    f.textures[i] = c.alloc_texture_slot();
    d->update_descriptor_set(
        d.self,
        gfx::DescriptorSetUpdate{
            .set     = c.texture_views,
            .binding = 0,
            .element = f.textures[i],
            .images  = to_span({gfx::ImageBinding{.image_view = f.views[i]}})});
  }
}

}        // namespace ash