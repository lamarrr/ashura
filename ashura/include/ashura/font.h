#pragma once
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <utility>

#include "ashura/image.h"
#include "ashura/loggers.h"
#include "ashura/primitives.h"
#include "ashura/rect_pack.h"
#include "freetype/freetype.h"
#include "harfbuzz/hb.h"
#include "stx/allocator.h"
#include "stx/enum.h"
#include "stx/limits.h"
#include "stx/memory.h"
#include "stx/span.h"
#include "stx/spinlock.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace ash
{
constexpr extent DEFAULT_MAX_FONT_ATLAS_EXTENT = extent{1920, 1080};

enum class FontLoadError : u8
{
  PathNotExist,
  InvalidFont,
  UnrecognizedFontName
};

struct Glyph
{
  bool        is_valid = false;
  u32         index    = 0;                          // the glyph index
  ash::offset offset;                                // offset into the atlas its glyph resides
  ash::extent extent;                                // extent of the glyph in the atlas
  f32         x      = 0;                            // defines x-offset from cursor position the glyph will be placed
  f32         ascent = 0;                            // defines ascent from baseline of the text
  vec2        advance;                               // advancement of the cursor after drawing this glyph
  f32         s0 = 0, t0 = 0, s1 = 0, t1 = 0;        // texture coordinates of this glyph in the atlas
};

struct Font
{
  static constexpr hb_tag_t KERNING_FEATURE             = HB_TAG('k', 'e', 'r', 'n');        // kerning operations
  static constexpr hb_tag_t LIGATURE_FEATURE            = HB_TAG('l', 'i', 'g', 'a');        // standard ligature substitution
  static constexpr hb_tag_t CONTEXTUAL_LIGATURE_FEATURE = HB_TAG('c', 'l', 'i', 'g');        // contextual ligature substitution

  stx::CStringView postscript_name;                                                          // ASCII. i.e. RobotoBold
  stx::CStringView family_name;                                                              // ASCII. i.e. Roboto
  stx::CStringView style_name;                                                               // ASCII. i.e. Bold
  hb_blob_t       *hb_blob           = nullptr;
  hb_face_t       *hb_face           = nullptr;
  hb_font_t       *hb_font           = nullptr;
  hb_buffer_t     *hb_scratch_buffer = nullptr;
  FT_Library       ft_lib            = nullptr;
  FT_Face          ft_face           = nullptr;
  bool             has_color         = false;
  stx::Memory      font_data;

  Font(stx::CStringView ipostscript_name, stx::CStringView ifamily_name, stx::CStringView istyle_name, hb_blob_t *ihb_blob,
       hb_face_t *ihb_face, hb_font_t *ihb_font, hb_buffer_t *ihb_scratch_buffer, FT_Library ift_lib, FT_Face ift_face,
       bool ihas_color, stx::Memory ifont_data) :
      postscript_name{ipostscript_name},
      family_name{ifamily_name},
      style_name{istyle_name},
      hb_blob{ihb_blob},
      hb_face{ihb_face},
      hb_font{ihb_font},
      hb_scratch_buffer{ihb_scratch_buffer},
      ft_lib{ift_lib},
      ft_face{ift_face},
      has_color{ihas_color},
      font_data{std::move(ifont_data)}
  {}

  STX_MAKE_PINNED(Font)

  ~Font()
  {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(hb_face);
    hb_font_destroy(hb_font);
    hb_buffer_destroy(hb_scratch_buffer);
    ASH_CHECK(FT_Done_Face(ft_face) == 0);
    ASH_CHECK(FT_Done_FreeType(ft_lib) == 0);
  }
};

inline stx::Result<stx::Rc<Font *>, FontLoadError> load_font_from_memory(stx::Memory memory, usize size)
{
  hb_blob_t *hb_blob = hb_blob_create(AS(char *, memory.handle), AS(uint, size), HB_MEMORY_MODE_READONLY, nullptr, nullptr);
  ASH_CHECK(hb_blob != nullptr);

  hb_face_t *hb_face = hb_face_create(hb_blob, 0);

  if (hb_face == nullptr)
  {
    hb_blob_destroy(hb_blob);
    return stx::Err(FontLoadError::InvalidFont);
  }

  hb_font_t *hb_font = hb_font_create(hb_face);

  if (hb_font == nullptr)
  {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(hb_face);
    return stx::Err(FontLoadError::InvalidFont);
  }

  hb_buffer_t *hb_scratch_buffer = hb_buffer_create();
  ASH_CHECK(hb_scratch_buffer != nullptr);

  FT_Library ft_lib;
  ASH_CHECK(FT_Init_FreeType(&ft_lib) == 0);

  FT_Face ft_face;

  if (FT_New_Memory_Face(ft_lib, AS(FT_Byte const *, memory.handle), AS(FT_Long, size), 0, &ft_face) != 0)
  {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(hb_face);
    hb_buffer_destroy(hb_scratch_buffer);
    ASH_CHECK(FT_Done_FreeType(ft_lib) == 0);
    return stx::Err(FontLoadError::InvalidFont);
  }

  return stx::Ok(stx::rc::make_inplace<Font>(stx::os_allocator, stx::CStringView{FT_Get_Postscript_Name(ft_face)},
                                             ft_face->family_name != nullptr ? stx::CStringView{ft_face->family_name} : stx::CStringView{},
                                             ft_face->style_name != nullptr ? stx::CStringView{ft_face->style_name} : stx::CStringView{},
                                             hb_face, hb_font, hb_scratch_buffer, ft_lib, ft_face, FT_HAS_COLOR(ft_face), std::move(memory))
                     .unwrap());
}

inline stx::Result<stx::Rc<Font *>, FontLoadError> load_font_from_file(stx::CStringView path)
{
  if (!std::filesystem::exists(path.c_str()))
  {
    return stx::Err(FontLoadError::PathNotExist);
  }

  std::FILE *file = std::fopen(path.c_str(), "rb");
  ASH_CHECK(file != nullptr);

  ASH_CHECK(std::fseek(file, 0, SEEK_END) == 0);

  long file_size = std::ftell(file);
  ASH_CHECK(file_size >= 0);

  stx::Memory memory = stx::mem::allocate(stx::os_allocator, file_size).unwrap();

  ASH_CHECK(std::fseek(file, 0, SEEK_SET) == 0);

  ASH_CHECK(std::fread(memory.handle, 1, file_size, file) == file_size);

  ASH_CHECK(std::fclose(file) == 0);

  return load_font_from_memory(std::move(memory), file_size);
}

/// stores codepoint glyphs for a font at a specific font height
struct FontAtlas
{
  stx::Vec<Glyph> glyphs;
  ash::extent     extent;                 // overall extent of the atlas
  f32             font_height = 0;        // font height at which the cache/atlas/glyphs will be rendered and cached
  gfx::image      texture     = 0;        // atlas containing the packed glyphs

  stx::Span<Glyph const> get(u32 glyph_index) const
  {
    if (glyph_index >= glyphs.size())
    {
      return {};
    }
    stx::Span glyph = glyphs.span().slice(glyph_index, 1);
    if (glyph.is_empty())
    {
      return glyph;
    }
    return glyph[0].is_valid ? glyph : glyph.slice(0, 0);
  }
};

inline std::pair<FontAtlas, ImageBuffer> render_font_atlas(Font const &font, f32 font_height, extent max_extent)
{
  ASH_LOG_INFO(FontRenderer, "Rendering CPU atlas for font: {}, Enumerating glyph indices...", font.postscript_name.c_str());
  // *64 to convert font height to 26.6 pixel format
  font_height = std::min(font_height, 1.0f);
  ASH_CHECK(FT_Set_Char_Size(font.ft_face, 0, AS(FT_F26Dot6, font_height * 64), 72, 72) == 0);

  stx::Vec<Glyph> glyphs;

  {
    FT_UInt glyph_index;

    FT_ULong codepoint = FT_Get_First_Char(font.ft_face, &glyph_index);

    while (glyph_index != 0)
    {
      if (FT_Load_Glyph(font.ft_face, glyph_index, 0) == 0)
      {
        u32 width  = font.ft_face->glyph->bitmap.width;
        u32 height = font.ft_face->glyph->bitmap.rows;

        // convert from 26.6 pixel format
        vec2 advance{font.ft_face->glyph->advance.x / 64.0f, font.ft_face->glyph->advance.y / 64.0f};

        glyphs
            .push(Glyph{.is_valid = true,
                        .index    = glyph_index,
                        .offset   = offset{},
                        .extent   = extent{width, height},
                        .x        = AS(f32, font.ft_face->glyph->bitmap_left),
                        .ascent   = AS(f32, font.ft_face->glyph->bitmap_top),
                        .advance  = advance,
                        .s0       = 0,
                        .t0       = 0,
                        .s1       = 0,
                        .t1       = 0})
            .unwrap();
      }
      codepoint = FT_Get_Next_Char(font.ft_face, codepoint, &glyph_index);
    }
  }

  ASH_LOG_INFO(FontRenderer, "Packing placement rectangles for font: {}", font.postscript_name.c_str());

  stx::Memory rects_mem = stx::mem::allocate(stx::os_allocator, sizeof(rp::rect) * glyphs.size()).unwrap();

  stx::Span rects{AS(rp::rect *, rects_mem.handle), glyphs.size()};

  for (usize i = 0; i < glyphs.size(); i++)
  {
    rects[i].glyph_index = glyphs[i].index;
    rects[i].w           = AS(i32, glyphs[i].extent.width + 2);
    rects[i].h           = AS(i32, glyphs[i].extent.height + 2);
  }

  stx::Memory nodes_memory = stx::mem::allocate(stx::os_allocator, sizeof(rp::Node) * max_extent.width).unwrap();

  rp::Context context = rp::init(max_extent.width, max_extent.height, AS(rp::Node *, nodes_memory.handle), max_extent.width, false);
  ASH_CHECK(rp::pack_rects(context, rects.data(), AS(i32, rects.size())));

  // NOTE: vulkan doesn't allow zero-extent images
  extent atlas_extent{1, 1};

  for (usize i = 0; i < rects.size(); i++)
  {
    atlas_extent.width  = std::max<u32>(atlas_extent.width, rects[i].x + rects[i].w);
    atlas_extent.height = std::max<u32>(atlas_extent.height, rects[i].y + rects[i].h);
  }

  rects.sort([](rp::rect const &a, rp::rect const &b) { return a.glyph_index < b.glyph_index; });

  glyphs.span().sort([](Glyph const &a, Glyph const &b) { return a.index < b.index; });

  for (usize i = 0; i < glyphs.size(); i++)
  {
    u32 x = AS(u32, rects[i].x) + 1;
    u32 y = AS(u32, rects[i].y) + 1;
    u32 w = glyphs[i].extent.width;
    u32 h = glyphs[i].extent.height;

    glyphs[i].offset = offset{x, y};
    // TODO(lamarrr): 0.5f offset to prevent texture bleeding when sampling
    // borders
    // https://gamedev.stackexchange.com/questions/46963/how-to-avoid-texture-bleeding-in-a-texture-atlas
    glyphs[i].s0 = (AS(f32, x) + 0.5f) / atlas_extent.width;
    glyphs[i].s1 = (AS(f32, x + w) - 0.5f) / atlas_extent.width;
    glyphs[i].t0 = (AS(f32, y) + 0.5f) / atlas_extent.height;
    glyphs[i].t1 = (AS(f32, y + h) - 0.5f) / atlas_extent.height;
  }

  {
    usize iter    = 0;
    usize nglyphs = glyphs.size();
    for (u32 next_index = 0; iter < nglyphs; next_index++, iter++)
    {
      for (; next_index < glyphs[iter].index; next_index++)
      {
        glyphs
            .push(Glyph{.is_valid = false,
                        .index    = next_index,
                        .offset   = {},
                        .extent   = {},
                        .x        = 0,
                        .ascent   = 0,
                        .advance  = {},
                        .s0       = 0,
                        .t0       = 0,
                        .s1       = 0,
                        .t1       = 0})
            .unwrap();
      }
    }
  }

  glyphs.span().sort([](Glyph const &a, Glyph const &b) { return a.index < b.index; });

  ASH_LOG_INFO(FontRenderer, "Finished packing placement rectangles for font: {}, rendering {}x{} CPU atlas...", font.postscript_name.c_str(), atlas_extent.width, atlas_extent.height);

  ImageFormat format = font.has_color ? ImageFormat::Bgra : ImageFormat::Antialiasing;

  usize nchannels = font.has_color ? 4 : 1;

  usize size = atlas_extent.area() * nchannels;

  stx::Memory buffer_mem = stx::mem::allocate(stx::os_allocator, size).unwrap();

  u8 *buffer = AS(u8 *, buffer_mem.handle);

  std::memset(buffer, 0, size);

  usize stride = atlas_extent.width * nchannels;

  for (Glyph const &glyph : glyphs)
  {
    auto fill_transparent = [&] {
      for (usize j = glyph.offset.y; j < glyph.offset.y + glyph.extent.height; j++)
      {
        u8 *out = buffer + j * stride + glyph.offset.x * nchannels;
        std::fill(out, out + glyph.extent.width * stride, 0);
      }
    };

    if (glyph.is_valid)
    {
      if (FT_Load_Glyph(font.ft_face, glyph.index, font.has_color ? (FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_COLOR) : (FT_LOAD_DEFAULT | FT_LOAD_RENDER)) != 0)
      {
        ASH_LOG_ERR(FontRenderer, "Failed to render glyph at index: {} for font: {}", glyph.index, font.postscript_name.c_str());
        fill_transparent();
        continue;
      }

      FT_Pixel_Mode pixel_mode = AS(FT_Pixel_Mode, font.ft_face->glyph->bitmap.pixel_mode);

      if (pixel_mode != FT_PIXEL_MODE_GRAY && pixel_mode != FT_PIXEL_MODE_BGRA)
      {
        ASH_LOG_ERR(FontRenderer, "Encountered unsupported pixel mode: {} for font: {}", AS(i32, pixel_mode), font.postscript_name.c_str());
        fill_transparent();
        continue;
      }

      u8 const *bitmap = font.ft_face->glyph->bitmap.buffer;

      // copy the rendered glyph to the atlas
      if ((pixel_mode == FT_PIXEL_MODE_GRAY && !font.has_color) || (pixel_mode == FT_PIXEL_MODE_BGRA && font.has_color))
      {
        for (usize j = glyph.offset.y; j < glyph.offset.y + glyph.extent.height; j++)
        {
          u8 *out = buffer + j * stride + glyph.offset.x * nchannels;
          std::copy(bitmap, bitmap + stride, out);
          bitmap += font.ft_face->glyph->bitmap.pitch;
        }
      }
      else
      {
        ASH_LOG_ERR(FontRenderer, "Unsupported multi-colored font detected in font {}. some glyphs will be missing!", font.postscript_name.c_str());
        fill_transparent();
      }
    }
    else
    {
      fill_transparent();
    }
  }

  ASH_LOG_INFO(FontRenderer, "Finished rendering CPU atlas for font: {}", font.postscript_name.c_str());

  return std::make_pair(FontAtlas{.glyphs = std::move(glyphs), .extent = atlas_extent, .font_height = font_height, .texture = 0},
                        ImageBuffer{.memory = std::move(buffer_mem), .extent = atlas_extent, .format = format});
}

struct FontSpec
{
  stx::String name;                          // name to use to recognise the font
  stx::String path;                          // local file system path of the typeface resource
  f32         atlas_font_height = 26;        // font height to render and cache the font atlas' glyphs at
};

struct BundledFont
{
  stx::Rc<Font *>          font;
  stx::Option<ImageBuffer> buffer;        // stx::None once uploaded to GPU
  FontAtlas                atlas;
  bool                     gpu_backed = false;
};

struct FontBundleEntry
{
  FontSpec                                             spec;
  stx::Future<stx::Result<BundledFont, FontLoadError>> font;
};

struct FontBundle
{
  stx::Vec<FontBundleEntry> fonts;

  explicit FontBundle(stx::Vec<FontSpec> font_specs);

  void add_fonts(stx::Vec<FontSpec> font_specs);

  stx::Result<FontBundleEntry const *, FontLoadError> get(std::string_view font);
};

}        // namespace ash
