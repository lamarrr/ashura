#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <utility>

#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/rect_pack.h"
#include "freetype/freetype.h"
#include "harfbuzz/hb.h"
#include "stx/allocator.h"
#include "stx/limits.h"
#include "stx/memory.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace ash {

enum class FontLoadError : u8 { InvalidPath };

enum class TextAlign : u8 { Left, Center, Right };

enum class TextDirection : u8 { LeftToRight, RightToLeft };

// TODO(lamarrr): implement
enum class TextOverflow : u8 { None, Ellipsis };

struct TextStyle {
  f32 font_height = 16;

  /// multiplied by font_height
  f32 line_height = 1.2f;
  f32 letter_spacing = 1;
  f32 word_spacing = 4;
  u32 tab_size = 8;
  bool use_kerning = true;

  /// use standard and contextual ligature substitution
  bool use_ligatures = true;
  bool underline = false;
  bool strikethrough = false;  // TODO(lamarrr): implement
  color foreground_color = colors::BLACK;
  color background_color = colors::TRANSPARENT;
  color underline_color = colors::TRANSPARENT;
  f32 underline_thickness = 1;
};

/// A text run is a sequence of characters sharing a single property set. Any
/// change to the format, such as font style, foreground color, font family, or
/// any other formatting effect, breaks the text run.
struct TextRun {
  /// utf-8-encoded text
  stx::Span<char const> text;
  usize font = 0;
  TextStyle style;
  TextDirection direction = TextDirection::LeftToRight;
  hb_script_t script = HB_SCRIPT_LATIN;
  hb_language_t language = hb_language_from_string("en", 2);
};

struct Paragraph {
  stx::Span<TextRun const> runs;
  TextAlign align = TextAlign::Left;
  TextOverflow overflow = TextOverflow::None;
};

struct Font {
  /// kerning operations
  static constexpr hb_tag_t KERNING_FEATURE = HB_TAG('k', 'e', 'r', 'n');

  /// standard ligature substitution
  static constexpr hb_tag_t LIGATURE_FEATURE = HB_TAG('l', 'i', 'g', 'a');

  /// contextual ligature substitution
  static constexpr hb_tag_t CONTEXTUAL_LIGATURE_FEATURE =
      HB_TAG('c', 'l', 'i', 'g');

  hb_face_t* hbface = nullptr;
  hb_font_t* hbfont = nullptr;
  hb_buffer_t* hbscratch_buffer = nullptr;
  FT_Library ftlib = nullptr;
  FT_Face ftface = nullptr;
  stx::Memory font_data;

  Font(hb_face_t* ahbface, hb_font_t* ahbfont, hb_buffer_t* ahbscratch_buffer,
       FT_Library aftlib, FT_Face aftface, stx::Memory afont_data)
      : hbface{ahbface},
        hbfont{ahbfont},
        hbscratch_buffer{ahbscratch_buffer},
        ftlib{aftlib},
        ftface{aftface},
        font_data{std::move(afont_data)} {}

  STX_MAKE_PINNED(Font)

  ~Font() {
    hb_face_destroy(hbface);
    hb_font_destroy(hbfont);
    hb_buffer_destroy(hbscratch_buffer);
    ASH_CHECK(FT_Done_Face(ftface) == 0);
    ASH_CHECK(FT_Done_FreeType(ftlib) == 0);
  }
};

inline stx::Rc<Font*> load_font_from_memory(stx::Memory memory, usize size) {
  hb_blob_t* hbblob = hb_blob_create(AS(char*, memory.handle), AS(uint, size),
                                     HB_MEMORY_MODE_READONLY, nullptr, nullptr);
  ASH_CHECK(hbblob != nullptr);

  hb_face_t* hbface = hb_face_create(hbblob, 0);
  ASH_CHECK(hbface != nullptr);

  hb_font_t* hbfont = hb_font_create(hbface);
  ASH_CHECK(hbfont != nullptr);

  hb_buffer_t* hbscratch_buffer = hb_buffer_create();
  ASH_CHECK(hbscratch_buffer != nullptr);

  FT_Library ftlib;
  ASH_CHECK(FT_Init_FreeType(&ftlib) == 0);

  FT_Face ftface;
  ASH_CHECK(FT_New_Memory_Face(ftlib, AS(FT_Byte const*, memory.handle),
                               AS(FT_Long, size), 0, &ftface) == 0);

  return stx::rc::make_inplace<Font>(stx::os_allocator, hbface, hbfont,
                                     hbscratch_buffer, ftlib, ftface,
                                     std::move(memory))
      .unwrap();
}

inline stx::Result<stx::Rc<Font*>, FontLoadError> load_font_from_file(
    stx::String const& path) {
  if (!std::filesystem::exists(path.c_str())) {
    return stx::Err(FontLoadError::InvalidPath);
  }

  std::ifstream stream{path.c_str(), std::ios::ate | std::ios_base::binary};

  usize size = stream.tellg();
  stream.seekg(0);

  stx::Memory memory = stx::mem::allocate(stx::os_allocator, size).unwrap();

  stream.read(AS(char*, memory.handle), size);

  stream.close();

  return stx::Ok(load_font_from_memory(std::move(memory), size));
}

namespace gfx {

struct Glyph {
  bool is_valid = false;

  /// the glyph index
  u32 index = 0;

  /// unicode codepoint this glyph represents
  u32 codepoint = 0;

  /// offset into the atlas its glyph resides
  offset offset;

  /// extent of the glyph in the atlas
  extent extent;

  /// defines x-offset from cursor position the glyph will be placed
  f32 x = 0;

  /// defines ascent from baseline of the text
  f32 ascent = 0;

  /// advancement of the cursor after drawing this glyph
  vec2 advance;

  /// texture coordinates of this glyph in the atlas
  f32 s0 = 0, t0 = 0, s1 = 0, t1 = 0;
};

/// stores codepoint glyphs for a font at a specific font height
struct FontAtlas {
  stx::Vec<Glyph> glyphs{stx::os_allocator};

  /// overall extent of the atlas
  extent extent;

  /// font height at which the cache/atlas/glyphs will be rendered and cached
  u32 font_height = 26;

  /// atlas containing the packed glyphs
  image image = 0;

  stx::Span<Glyph const> get(u32 glyph_index) const {
    if (glyph_index >= glyphs.size()) return {};
    stx::Span glyph = glyphs.span().slice(glyph_index, 1);
    if (glyph.is_empty()) return glyph;
    return glyph[0].is_valid ? glyph : glyph.slice(0, 0);
  }
};

inline std::pair<FontAtlas, RgbaImageBuffer> render_atlas(Font const& font,
                                                          u32 font_height,
                                                          extent max_extent) {
  /// *64 to convert font height to 26.6 pixel format
  ASH_CHECK(font_height > 0);
  ASH_CHECK(FT_Set_Char_Size(font.ftface, 0, font_height * 64, 72, 72) == 0);

  stx::Vec<Glyph> glyphs{stx::os_allocator};

  {
    FT_UInt glyph_index;

    FT_ULong codepoint = FT_Get_First_Char(font.ftface, &glyph_index);

    while (glyph_index != 0) {
      if (FT_Load_Glyph(font.ftface, glyph_index, 0) == 0) {
        u32 width = font.ftface->glyph->bitmap.width;
        u32 height = font.ftface->glyph->bitmap.rows;

        // convert from 26.6 pixel format
        vec2 advance{font.ftface->glyph->advance.x / 64.0f,
                     font.ftface->glyph->advance.y / 64.0f};

        glyphs
            .push(Glyph{.is_valid = true,
                        .index = glyph_index,
                        .codepoint = codepoint,
                        .offset = {},
                        .extent = {width, height},
                        .x = AS_F32(font.ftface->glyph->bitmap_left),
                        .ascent = AS_F32(font.ftface->glyph->bitmap_top),
                        .advance = advance,
                        .s0 = 0,
                        .t0 = 0,
                        .s1 = 0,
                        .t1 = 0})
            .unwrap();
      }
      codepoint = FT_Get_Next_Char(font.ftface, codepoint, &glyph_index);
    }
  }

  Glyph* last = std::unique(
      glyphs.begin(), glyphs.end(),
      [](Glyph const& a, Glyph const& b) { return a.index == b.index; });

  glyphs.resize(AS(size_t, last - glyphs.begin())).unwrap();

  stx::Memory rects_mem =
      stx::mem::allocate(stx::os_allocator, sizeof(rp::rect) * glyphs.size())
          .unwrap();

  stx::Span rects{AS(rp::rect*, rects_mem.handle), glyphs.size()};

  for (usize i = 0; i < glyphs.size(); i++) {
    rects[i].glyph_index = glyphs[i].index;
    rects[i].w = AS_I32(glyphs[i].extent.width + 2);
    rects[i].h = AS_I32(glyphs[i].extent.height + 2);
  }

  stx::Memory nodes_memory =
      stx::mem::allocate(stx::os_allocator, sizeof(rp::Node) * max_extent.width)
          .unwrap();

  rp::Context context =
      rp::init(max_extent.width, max_extent.height,
               AS(rp::Node*, nodes_memory.handle), max_extent.width, false);
  ASH_CHECK(rp::pack_rects(context, rects.data(), AS_I32(rects.size())));

  extent atlas_extent;

  for (usize i = 0; i < rects.size(); i++) {
    atlas_extent.width =
        std::max<u32>(atlas_extent.width, rects[i].x + rects[i].w);
    atlas_extent.height =
        std::max<u32>(atlas_extent.height, rects[i].y + rects[i].h);
  }

  rects.sort([](rp::rect const& a, rp::rect const& b) {
    return a.glyph_index < b.glyph_index;
  });

  glyphs.span().sort(
      [](Glyph const& a, Glyph const& b) { return a.index < b.index; });

  for (usize i = 0; i < glyphs.size(); i++) {
    u32 x = AS_U32(rects[i].x) + 1;
    u32 y = AS_U32(rects[i].y) + 1;
    u32 w = glyphs[i].extent.width;
    u32 h = glyphs[i].extent.height;

    glyphs[i].offset = {x, y};
    glyphs[i].s0 = AS_F32(x) / atlas_extent.width;
    glyphs[i].s1 = AS_F32(x + w) / atlas_extent.width;
    glyphs[i].t0 = AS_F32(y) / atlas_extent.height;
    glyphs[i].t1 = AS_F32(y + h) / atlas_extent.height;
  }

  {
    usize iter = 0;
    for (u32 next_index = 0; iter < glyphs.size(); next_index++, iter++) {
      for (; next_index < glyphs[iter].index; next_index++) {
        glyphs
            .push(Glyph{.is_valid = false,
                        .index = next_index,
                        .codepoint = 0,
                        .offset = {},
                        .extent = {},
                        .x = 0,
                        .ascent = 0,
                        .s0 = 0,
                        .t0 = 0,
                        .s1 = 0,
                        .t1 = 0})
            .unwrap();
      }
    }
  }

  glyphs.span().sort(
      [](Glyph const& a, Glyph const& b) { return a.index < b.index; });

  // NOTE: vulkan doesn't allow zero-extent images
  atlas_extent.width = std::max<u32>(1, atlas_extent.width);
  atlas_extent.height = std::max<u32>(1, atlas_extent.height);

  stx::Memory buffer_mem =
      stx::mem::allocate(stx::os_allocator, atlas_extent.area() * 4).unwrap();

  u8* buffer = AS(u8*, buffer_mem.handle);

  std::memset(buffer, 0, atlas_extent.area() * 4);

  bool const is_colored_font = FT_HAS_COLOR(font.ftface);

  for (Glyph const& glyph : glyphs) {
    ASH_CHECK(
        FT_Load_Glyph(font.ftface, glyph.index,
                      is_colored_font
                          ? (FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_COLOR)
                          : (FT_LOAD_DEFAULT | FT_LOAD_RENDER)) == 0);

    uchar pixel_mode = font.ftface->glyph->bitmap.pixel_mode;

    ASH_CHECK(pixel_mode == FT_PIXEL_MODE_GRAY ||
              pixel_mode == FT_PIXEL_MODE_BGRA);

    u8* bitmap = font.ftface->glyph->bitmap.buffer;

    // copy the rendered glyph to the atlas
    if (pixel_mode == FT_PIXEL_MODE_GRAY) {
      for (usize j = glyph.offset.y;
           j < glyph.offset.y + font.ftface->glyph->bitmap.rows; j++) {
        for (usize i = glyph.offset.x * 4;
             i < (glyph.offset.x + font.ftface->glyph->bitmap.width) * 4;
             i += 4) {
          buffer[j * atlas_extent.width * 4 + i + 0] = 0xFF;
          buffer[j * atlas_extent.width * 4 + i + 1] = 0xFF;
          buffer[j * atlas_extent.width * 4 + i + 2] = 0xFF;
          buffer[j * atlas_extent.width * 4 + i + 3] = *bitmap;
          bitmap++;
        }
      }
    } else if (pixel_mode == FT_PIXEL_MODE_BGRA) {
      for (usize j = glyph.offset.y;
           j < glyph.offset.y + font.ftface->glyph->bitmap.rows; j++) {
        for (usize i = glyph.offset.x * 4;
             i < (glyph.offset.x + font.ftface->glyph->bitmap.width) * 4;
             i += 4) {
          buffer[j * atlas_extent.width * 4 + i + 0] = bitmap[2];
          buffer[j * atlas_extent.width * 4 + i + 1] = bitmap[1];
          buffer[j * atlas_extent.width * 4 + i + 2] = bitmap[0];
          buffer[j * atlas_extent.width * 4 + i + 3] = bitmap[3];
          bitmap += 4;
        }
      }
    }
  }

  return std::make_pair(
      FontAtlas{.glyphs = std::move(glyphs),
                .extent = atlas_extent,
                .font_height = font_height,
                .image = 0},
      RgbaImageBuffer{.memory = std::move(buffer_mem), .extent = atlas_extent});
}

struct CachedFont {
  stx::Rc<Font*> font;
  FontAtlas atlas;
};

struct SubwordGlyph {
  usize font = 0;
  u32 glyph = 0;
};

/// this is part of a word that is styled by its run
struct RunSubWord {
  stx::Span<char const> text;
  usize run = 0;
  usize nspaces = 0;
  usize nline_breaks = 0;
  f32 width = 0;
  usize glyph_start = 0;
  usize nglyphs = 0;
  bool is_wrapped = false;
};

struct TextLayout {
  f32 width = 0;
  f32 height = 0;
  f32 min_width = 0;
  f32 min_height = 0;
};

}  // namespace gfx
}  // namespace ash
