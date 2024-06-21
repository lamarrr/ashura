
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

Result<Font, FontStatus> load_font(Span<u8 const> encoded, u32 face,
                                   AllocatorImpl const &allocator)
{
  u32   font_data_size = (u32) encoded.size();
  char *font_data;
  if (!allocator.nalloc(font_data_size, &font_data))
  {
    return Err{FontStatus::OutOfMemory};
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
    return Err{FontStatus::DecodingFailed};
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
    return Err{FontStatus::FaceNotFound};
  }

  hb_face_t *hb_face = hb_face_create(hb_blob, face);

  if (hb_face == nullptr)
  {
    return Err{FontStatus::DecodingFailed};
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
    return Err{FontStatus::DecodingFailed};
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
    return Err{FontStatus::DecodingFailed};
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
    return Err{FontStatus::DecodingFailed};
  }

  if (FT_Set_Char_Size(ft_face, PT_UNIT, PT_UNIT, 72, 72) != 0)
  {
    return Err{FontStatus::DecodingFailed};
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
      return Err{FontStatus::OutOfMemory};
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
      return Err{FontStatus::OutOfMemory};
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
      return Err{FontStatus::OutOfMemory};
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
  i32 const descent = -ft_face->size->metrics.descender;
  i32 const advance = ft_face->size->metrics.max_advance;

  Glyph *glyphs;
  if (!allocator.nalloc(num_glyphs, &glyphs))
  {
    return Err{FontStatus::OutOfMemory};
  }

  defer glyphs_del{[&] {
    if (glyphs != nullptr)
    {
      allocator.ndealloc(glyphs, num_glyphs);
    }
  }};

  for (u32 i = 0; i < num_glyphs; i++)
  {
    if (FT_Load_Glyph(ft_face, i, FT_LOAD_DEFAULT) == 0)
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
      glyphs[i] = Glyph{.is_valid = true, .metrics = m};
    }
    else
    {
      glyphs[i] = Glyph{.is_valid = false, .metrics = {}};
    }
  }

  FontImpl *f;
  if (!allocator.nalloc(1, &f))
  {
    return Err{FontStatus::OutOfMemory};
  }

  new (f)
      FontImpl{.allocator            = allocator,
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
               .metrics              = FontMetrics{
                                .ascent = ascent, .descent = descent, .advance = advance}};

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

  return Ok{(Font) f};
}

FontInfo get_font_info(Font font)
{
  FontImpl *f = (FontImpl *) font;
  return FontInfo{
      .postscript_name   = {f->postscript_name, f->postscript_name_size},
      .family_name       = {f->family_name, f->family_name_size},
      .style_name        = {f->style_name, f->style_name_size},
      .glyphs            = {f->glyphs, f->num_glyphs},
      .replacement_glyph = f->replacement_glyph,
      .space_glyph       = f->space_glyph,
      .ellipsis_glyph    = f->ellipsis_glyph,
      .metrics           = f->metrics};
}

void destroy_font(Font font)
{
  FontImpl *f = (FontImpl *) font;
  f->allocator.ndealloc(f->postscript_name, f->postscript_name_size);
  f->allocator.ndealloc(f->family_name, f->family_name_size);
  f->allocator.ndealloc(f->style_name, f->style_name_size);
  hb_font_destroy(f->hb_font);
  hb_face_destroy(f->hb_face);
  hb_blob_destroy(f->hb_blob);
  FT_Done_Face(f->ft_face);
  FT_Done_FreeType(f->ft_lib);
  f->allocator.ndealloc(f->glyphs, f->num_glyphs);
  f->allocator.ndealloc(f->font_data, f->font_data_size);
  f->allocator.ndealloc(f, 1);
}

bool rasterize_font(Font font, u32 font_height, FontAtlas &atlas,
                    AllocatorImpl const &allocator)
{
  constexpr u32 MIN_ATLAS_EXTENT = 512;
  static_assert(MIN_ATLAS_EXTENT > 0, "Font atlas extent must be non-zero");
  static_assert(MIN_ATLAS_EXTENT > 128,
                "Font atlas extent must be at least 128px");
  static_assert(MIN_ATLAS_EXTENT % 64 == 0,
                "Font atlas extent should be a multiple of 64");
  static_assert(MIN_ATLAS_EXTENT <= gfx::MAX_IMAGE_EXTENT_2D,
                "Font atlas extent too large for GPU platform");
  CHECK(font_height <= 1024);
  CHECK(font_height <= MIN_ATLAS_EXTENT / 8);

  Vec2U atlas_extent{MIN_ATLAS_EXTENT, MIN_ATLAS_EXTENT};

  FontImpl *f = (FontImpl *) font;

  if (!atlas.glyphs.resize_defaulted(f->num_glyphs))
  {
    return false;
  }

  if (FT_Set_Pixel_Sizes(f->ft_face, font_height, font_height) != 0)
  {
    return false;
  }

  u32 num_rasterized_glyphs = 0;

  for (u32 i = 0; i < f->num_glyphs; i++)
  {
    if (f->glyphs[i].is_valid)
    {
      FT_Error ft_error = FT_Load_Glyph(f->ft_face, i, FT_LOAD_DEFAULT);
      if (ft_error != 0)
      {
        continue;
      }

      Vec2U extent{f->ft_face->glyph->bitmap.width,
                   f->ft_face->glyph->bitmap.rows};

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
    rect_pack::rect *rects;
    if (!allocator.nalloc(num_rasterized_glyphs, &rects))
    {
      return false;
    }

    defer rects_del{[&] { allocator.ndealloc(rects, num_rasterized_glyphs); }};

    for (u32 g = 0, irect = 0; g < f->num_glyphs; g++)
    {
      Glyph const      &gl  = f->glyphs[g];
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

    CHECK(atlas_extent.x <= gfx::MAX_IMAGE_EXTENT_2D);
    CHECK(atlas_extent.y <= gfx::MAX_IMAGE_EXTENT_2D);

    rect_pack::Node *nodes;
    if (!allocator.nalloc(atlas_extent.x, &nodes))
    {
      return false;
    }

    defer nodes_del{[&] { allocator.ndealloc(nodes, atlas_extent.x); }};

    u32  num_packed = 0;
    bool all_packed = false;

    while (!all_packed)
    {
      // tries to pack all the glyph rects into the provided extent
      rect_pack::Context pack_context = rect_pack::init(
          atlas_extent.x, atlas_extent.y, nodes, atlas_extent.x, true);
      all_packed = rect_pack::pack_rects(pack_context, rects + num_packed,
                                         num_rasterized_glyphs - num_packed);
      auto [just_packed, unpacked] =
          partition(Span{rects, num_rasterized_glyphs}.slice(num_packed),
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
      g.uv[0]           = Vec2{g.area.offset.x / (f32) atlas_extent.x,
                     g.area.offset.y / (f32) atlas_extent.y};
      g.uv[1] =
          Vec2{(g.area.offset.x + g.area.extent.x) / (f32) atlas_extent.x,
               (g.area.offset.y + g.area.extent.y) / (f32) atlas_extent.y};
    }
  }

  u64 const atlas_area       = (u64) atlas_extent.x * (u64) atlas_extent.y;
  u64 const atlas_layer_size = atlas_area;
  u64 const atlas_size       = atlas_layer_size * num_layers;

  if (!atlas.channels.resize_defaulted(atlas_size))
  {
    return false;
  }

  ImageLayerSpan<u8, 1> atlas_span{.channels = to_span(atlas.channels),
                                   .width    = atlas_extent.x,
                                   .height   = atlas_extent.y,
                                   .layers   = num_layers};

  for (u32 i = 0; i < f->num_glyphs; i++)
  {
    Glyph const      &g  = f->glyphs[i];
    AtlasGlyph const &ag = atlas.glyphs[i];
    if (g.is_valid)
    {
      FT_GlyphSlot slot = f->ft_face->glyph;
      FT_Error     ft_error =
          FT_Load_Glyph(f->ft_face, i, FT_LOAD_DEFAULT | FT_LOAD_RENDER);
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

  return true;
}

}        // namespace ash