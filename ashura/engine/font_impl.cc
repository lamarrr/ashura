
/// SPDX-License-Identifier: MIT
#include "ashura/engine/font_impl.h"

namespace ash
{

Result<Dyn<Font *>, FontErr> Font::decode(Span<u8 const> encoded, u32 face,
                                          AllocatorImpl allocator)
{
  Vec<char> font_data{allocator};
  if (!font_data.extend_copy(encoded.as_char()))
  {
    return Err{FontErr::OutOfMemory};
  }

  hb_blob_t *hb_blob =
      hb_blob_create(font_data.data(), font_data.size(),
                     HB_MEMORY_MODE_READONLY, nullptr, nullptr);

  if (hb_blob == nullptr)
  {
    return Err{FontErr::DecodingFailed};
  }

  defer hb_blob_{[&] {
    if (hb_blob != nullptr)
    {
      hb_blob_destroy(hb_blob);
    }
  }};

  u32 num_faces = hb_face_count(hb_blob);

  if (face >= num_faces)
  {
    return Err{FontErr::FaceNotFound};
  }

  hb_face_t *hb_face = hb_face_create(hb_blob, face);

  if (hb_face == nullptr)
  {
    return Err{FontErr::DecodingFailed};
  }

  defer hb_face_{[&] {
    if (hb_face != nullptr)
    {
      hb_face_destroy(hb_face);
    }
  }};

  hb_font_t *hb_font = hb_font_create(hb_face);

  if (hb_font == nullptr)
  {
    return Err{FontErr::DecodingFailed};
  }

  hb_font_set_scale(hb_font, AU_UNIT, AU_UNIT);

  defer hb_font_{[&] {
    if (hb_font != nullptr)
    {
      hb_font_destroy(hb_font);
    }
  }};

  FT_Library ft_lib;
  if (FT_Init_FreeType(&ft_lib) != 0)
  {
    return Err{FontErr::DecodingFailed};
  }

  defer ft_lib_{[&] {
    if (ft_lib != nullptr)
    {
      FT_Done_FreeType(ft_lib);
    }
  }};

  FT_Face ft_face;

  if (FT_New_Memory_Face(ft_lib, (FT_Byte const *) font_data.data(),
                         (FT_Long) font_data.size(), 0, &ft_face) != 0)
  {
    return Err{FontErr::DecodingFailed};
  }

  if (FT_Set_Char_Size(ft_face, AU_UNIT, AU_UNIT, 72, 72) != 0)
  {
    return Err{FontErr::DecodingFailed};
  }

  defer ft_face_{[&] {
    if (ft_face != nullptr)
    {
      FT_Done_Face(ft_face);
    }
  }};

  char const *ft_postscript_name   = FT_Get_Postscript_Name(ft_face);
  usize       postscript_name_size = 0;

  Vec<char> postscript_name{allocator};

  if (ft_postscript_name != nullptr)
  {
    postscript_name_size = strlen(ft_postscript_name);
    if (!postscript_name.extend_copy(
            Span{ft_postscript_name, postscript_name_size}))
    {
      return Err{FontErr::OutOfMemory};
    }
  }

  Vec<char> family_name{allocator};
  usize     family_name_size = 0;

  if (ft_face->family_name != nullptr)
  {
    family_name_size = strlen(ft_face->family_name);
    if (!family_name.extend_copy(Span{ft_face->family_name, family_name_size}))
    {
      return Err{FontErr::OutOfMemory};
    }
  }

  Vec<char> style_name{allocator};
  usize     style_name_size = 0;

  if (ft_face->style_name != nullptr)
  {
    style_name_size = strlen(ft_face->style_name);
    if (!style_name.extend_copy(Span{ft_face->style_name, style_name_size}))
    {
      return Err{FontErr::OutOfMemory};
    }
  }

  u32 const num_glyphs = (u32) ft_face->num_glyphs;
  // glyph 0 is selected if the replacement codepoint glyph is not found
  u32 const replacement_glyph = FT_Get_Char_Index(ft_face, 0xFFFD);
  u32 const ellipsis_glyph    = FT_Get_Char_Index(ft_face, 0x2026);
  u32 const space_glyph       = FT_Get_Char_Index(ft_face, ' ');

  // expressed on a AU_UNIT scale
  i32 const ascent  = ft_face->size->metrics.ascender;
  i32 const descent = -ft_face->size->metrics.descender;
  i32 const advance = ft_face->size->metrics.max_advance;

  Vec<Glyph> glyphs{allocator};

  if (!glyphs.resize_uninit(num_glyphs))
  {
    return Err{FontErr::OutOfMemory};
  }

  for (u32 i = 0; i < num_glyphs; i++)
  {
    if (FT_Load_Glyph(ft_face, i, FT_LOAD_DEFAULT) == 0)
    {
      FT_GlyphSlot s = ft_face->glyph;

      GlyphMetrics m{.bearing{(i32) s->metrics.horiBearingX,
                              (i32) -s->metrics.horiBearingY},
                     .advance = (i32) s->metrics.horiAdvance,
                     .extent{(i32) s->metrics.width, (i32) s->metrics.height}};

      // bin offsets are determined after binning and during rect packing
      glyphs[i] = Glyph{.is_valid = true, .metrics = m};
    }
    else
    {
      glyphs[i] = Glyph{.is_valid = false, .metrics = {}};
    }
  }

  Result font = dyn_inplace<FontImpl>(
      allocator, std::move(font_data), std::move(postscript_name),
      std::move(family_name), std::move(style_name), hb_blob, hb_face, hb_font,
      ft_lib, ft_face, face, std::move(glyphs), replacement_glyph,
      ellipsis_glyph, space_glyph,
      FontMetrics{.ascent = ascent, .descent = descent, .advance = advance});

  if (!font)
  {
    return Err{FontErr::OutOfMemory};
  }

  hb_blob = nullptr;
  hb_face = nullptr;
  hb_font = nullptr;
  ft_lib  = nullptr;
  ft_face = nullptr;

  Font *font_base = font.value().get();

  return Ok{transmute(std::move(font.value()), font_base)};
}

}        // namespace ash