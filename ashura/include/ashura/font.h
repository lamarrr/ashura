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
#include "ashura/stb_image_resize.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "freetype/ftstroke.h"
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
constexpr extent DEFAULT_MAX_ATLAS_BIN_EXTENT = extent{1024, 1024};

enum class FontLoadError : u8
{
  PathNotExist,
  InvalidFont,
  UnrecognizedFontName
};

struct Font
{
  stx::CStringView postscript_name;        // ASCII. i.e. RobotoBold
  stx::CStringView family_name;            // ASCII. i.e. Roboto
  stx::CStringView style_name;             // ASCII. i.e. Bold
  hb_blob_t       *hb_blob   = nullptr;
  hb_face_t       *hb_face   = nullptr;
  hb_font_t       *hb_font   = nullptr;
  hb_buffer_t     *hb_buffer = nullptr;
  FT_Library       ft_lib    = nullptr;
  FT_Face          ft_face   = nullptr;
  stx::Memory      font_data;
  usize            font_data_size = 0;

  Font(stx::CStringView ipostscript_name, stx::CStringView ifamily_name, stx::CStringView istyle_name, hb_blob_t *ihb_blob,
       hb_face_t *ihb_face, hb_font_t *ihb_font, hb_buffer_t *ihb_buffer, FT_Library ift_lib, FT_Face ift_face,
       stx::Memory ifont_data, usize ifont_data_size) :
      postscript_name{ipostscript_name},
      family_name{ifamily_name},
      style_name{istyle_name},
      hb_blob{ihb_blob},
      hb_face{ihb_face},
      hb_font{ihb_font},
      hb_buffer{ihb_buffer},
      ft_lib{ift_lib},
      ft_face{ift_face},
      font_data{std::move(ifont_data)},
      font_data_size{ifont_data_size}
  {}

  STX_MAKE_PINNED(Font)

  ~Font()
  {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(hb_face);
    hb_font_destroy(hb_font);
    hb_buffer_destroy(hb_buffer);
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

  hb_buffer_t *hb_buffer = hb_buffer_create();
  ASH_CHECK(hb_buffer != nullptr);

  FT_Library ft_lib;
  ASH_CHECK(FT_Init_FreeType(&ft_lib) == 0);

  FT_Face ft_face;

  if (FT_New_Memory_Face(ft_lib, AS(FT_Byte const *, memory.handle), AS(FT_Long, size), 0, &ft_face) != 0)
  {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(hb_face);
    hb_buffer_destroy(hb_buffer);
    ASH_CHECK(FT_Done_FreeType(ft_lib) == 0);
    return stx::Err(FontLoadError::InvalidFont);
  }

  return stx::Ok(stx::rc::make_inplace<Font>(stx::os_allocator, stx::CStringView{FT_Get_Postscript_Name(ft_face)},
                                             ft_face->family_name != nullptr ? stx::CStringView{ft_face->family_name} : stx::CStringView{},
                                             ft_face->style_name != nullptr ? stx::CStringView{ft_face->style_name} : stx::CStringView{},
                                             hb_blob, hb_face, hb_font, hb_buffer, ft_lib, ft_face, std::move(memory), size)
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

/// atlas containing the packed glyphs
/// This enable support of large glyphs. We load all glyphs of a font into memory.
/// GPUs have texture size limits so we try to bin the font textures.
///
struct FontAtlasBin
{
  gfx::image texture = 0;
  extent     extent;
};

/// see: https://stackoverflow.com/questions/62374506/how-do-i-align-glyphs-along-the-baseline-with-freetype
///
/// NOTE: using stubs enables us to perform fast constant lookups of glyph indices by ensuring the array is filled and sorted by glyph index from 0 -> nglyphs_found_in_font-1
struct Glyph
{
  bool         is_valid = false;
  u32          bin      = 0;
  offset       offset;             // offset into the atlas this glyph is placed
  extent       extent;             // extent of the glyph in the atlas
  vec2         bearing;            // offset from cursor baseline to start drawing glyph from
  f32          descent = 0;        // distance from baseline to the bottom of the glyph
  vec2         advance;            // advancement of the cursor after drawing this glyph
  texture_rect bin_region;         // texture coordinates of this glyph in the atlas bin
};

/// stores codepoint glyphs for a font at a specific font height
/// For info on SDF Text Rendering,
/// See:
/// - https://www.youtube.com/watch?v=1b5hIMqz_wM
/// - https://cdn.cloudflare.steamstatic.com/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
///
/// In SDFs each pixel is encoded with its distance to the edge of a shape.
/// The inner portion of the glyph has a value at the midpoint of the text, i.e. encoded 127 + distance away from the glyph boundary.
/// The outer portion of the glyph however is encoded with a value lower than the midpoint. i.e. encoded 0-127.
///
struct FontAtlas
{
  stx::Vec<Glyph>        glyphs;
  f32                    font_height = 0;        // font height at which the this atlas was rendered
  f32                    ascent      = 0;
  f32                    descent     = 0;
  stx::Vec<FontAtlasBin> bins;

  stx::Span<Glyph const> get(usize glyph_index) const
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

struct SdfFontSpec
{
  u32    spread               = 8;                                   // spread factor of the SDF field
  u32    height               = 4096;                                // initial height of the 1-bit alpha texture from which the SDF is calculated from
  u32    downsampled_height   = 64;                                  // the height at which the SDF texture is cached at
  extent max_atlas_bin_extent = DEFAULT_MAX_ATLAS_BIN_EXTENT;        // maximum extent of each atlas bin
};

struct FontSpec
{
  stx::String name;        // name to use to match the font
  stx::String path;        // local file system path of the typeface resource
  SdfFontSpec sdf_spec;
  bool        use_caching = true;
};

struct BundledFont
{
  FontSpec        spec;
  stx::Rc<Font *> font;
  FontAtlas       atlas;
};

/// Generates 8-bit Signed Distance Field from a 1-bit Alpha Image.
///
/// output_width = width + sdf_spread * 2
/// output_height = height + sdf_spread * 2
///
inline void generate_sdf_from_mono(u8 const *const src, u32 const src_pitch, u32 const width, u32 const height, u32 const sdf_spread, u8 *const output, u32 const output_pitch)
{
  for (i64 i = 0; i < height + sdf_spread * 2; i++)
  {
    for (i64 j = 0; j < width + sdf_spread * 2; j++)
    {
      i64 isrc = i - sdf_spread;
      i64 jsrc = j - sdf_spread;

      u8 is_inside = (isrc < 0 || isrc >= height || jsrc < 0 || jsrc >= width) ? 0 : (src[isrc * src_pitch + (jsrc / 8)] >> (7 - (jsrc % 8))) & 1;

      // the squared distance to the nearest neigbor that has a different position along the shape
      i64 square_distance = sdf_spread * sdf_spread;

      for (i64 ifield = std::max(isrc - sdf_spread, (i64) 0); ifield < std::min(isrc + sdf_spread + 1, (i64) height); ifield++)
      {
        for (i64 jfield = std::max(jsrc - sdf_spread, (i64) 0); jfield < std::min(jsrc + sdf_spread + 1, (i64) width); jfield++)
        {
          u8 neighbor_is_inside = (src[ifield * src_pitch + (jfield / 8)] >> (7 - (jfield % 8))) & 1;
          if (neighbor_is_inside != is_inside)
          {
            i64 neighbor_square_distance = (ifield - isrc) * (ifield - isrc) + (jfield - jsrc) * (jfield - jsrc);
            square_distance              = std::min(square_distance, neighbor_square_distance);
          }
        }
      }

      i64 distance                 = (u8) (127 * std::sqrt((float) square_distance) / sdf_spread);
      i64 signed_distance          = 127 + (is_inside ? distance : -distance);
      output[i * output_pitch + j] = (u8) signed_distance;
    }
  }
}

inline std::pair<FontAtlas, ImageBuffer> render_font_atlas(Font const &font, SdfFontSpec const &spec, std::string_view cache_directory)
{
  FT_Library ft_lib;
  ASH_CHECK(FT_Init_FreeType(&ft_lib) == 0);

  FT_Face ft_face;
  ASH_CHECK(FT_New_Memory_Face(ft_lib, AS(FT_Byte const *, font.font_data.handle), AS(FT_Long, font.font_data_size), 0, &ft_face) == 0);

  // *64 to convert font height to 26.6 pixel format
  ASH_CHECK(FT_Set_Char_Size(ft_face, 0, spec.downsampled_height * 64, 72, 72) == 0);

  f32 const downsample_scale = AS(f32, spec.downsampled_height) / spec.height;

  stx::Vec<Glyph> glyphs;

  ASH_LOG_INFO(FontRenderer, "Rendering CPU atlas for font: {}, Fetching glyph metrics", font.postscript_name.c_str());

  for (usize glyph_index = 0; glyph_index < ft_face->num_glyphs; glyph_index++)
  {
    if (FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT) == 0)
    {
      FT_GlyphSlot slot   = ft_face->glyph;
      u32          width  = slot->bitmap.width;
      u32          height = slot->bitmap.rows;

      // convert from 26.6 pixel format
      vec2 bearing{slot->metrics.horiBearingX / 64.0f, slot->metrics.horiBearingY / 64.0f};
      vec2 advance{slot->advance.x / 64.0f, slot->advance.y / 64.0f};

      f32 descent = std::max(AS(f32, height) - bearing.y, 0.0f);

      glyphs
          .push(Glyph{.is_valid       = true,
                      .offset         = {},
                      .extent         = extent{width, height},
                      .bearing        = bearing,
                      .descent        = descent,
                      .advance        = advance,
                      .texture_region = {}})
          .unwrap();
    }
    else
    {
      glyphs
          .push(Glyph{.is_valid       = false,
                      .offset         = {},
                      .extent         = {},
                      .bearing        = {},
                      .descent        = 0,
                      .advance        = {},
                      .texture_region = {}})
          .unwrap();
    }
  }

  ASH_LOG_INFO(FontRenderer, "Packing {} placement rectangles for font: {}", glyphs.size(), font.postscript_name.c_str());

  stx::Memory rects_mem = stx::mem::allocate(stx::os_allocator, sizeof(rect_packer::rect) * glyphs.size()).unwrap();

  stx::Span rects{AS(rect_packer::rect *, rects_mem.handle), glyphs.size()};

  for (usize i = 0; i < glyphs.size(); i++)
  {
    rects[i].glyph_index = i;
    rects[i].x           = 0;
    rects[i].y           = 0;
    rects[i].w           = AS(i32, glyphs[i].extent.width + 2);
    rects[i].h           = AS(i32, glyphs[i].extent.height + 2);
  }

  stx::Memory nodes_memory = stx::mem::allocate(stx::os_allocator, sizeof(rect_packer::Node) * max_extent.width).unwrap();

  rect_packer::Context context = rect_packer::init(max_extent.width, max_extent.height, AS(rect_packer::Node *, nodes_memory.handle), max_extent.width, false);

  u32 used_area = 0;

  if (!rect_packer::pack_rects(context, rects.data(), AS(i32, rects.size())))
  {
    ASH_LOG_WARN(FontRenderer, "Not all glyphs were packed into atlas for font: {}", font.postscript_name.c_str());
    for (rect_packer::rect &pack_rect : rects)
    {
      if (!pack_rect.was_packed)
      {
        pack_rect.x = 0;
        pack_rect.y = 0;
      }
      else
      {
        used_area += pack_rect.w * pack_rect.h;
      }
    }
  }

  // NOTE: vulkan doesn't allow zero-extent images
  extent atlas_extent{1, 1};

  for (usize i = 0; i < rects.size(); i++)
  {
    atlas_extent.width  = std::max<u32>(atlas_extent.width, rects[i].x + rects[i].w);
    atlas_extent.height = std::max<u32>(atlas_extent.height, rects[i].y + rects[i].h);
  }

  rects.sort([](rect_packer::rect const &a, rect_packer::rect const &b) { return a.glyph_index < b.glyph_index; });

  for (usize i = 0; i < glyphs.size(); i++)
  {
    u32 x = AS(u32, rects[i].x) + 1;
    u32 y = AS(u32, rects[i].y) + 1;
    u32 w = glyphs[i].extent.width;
    u32 h = glyphs[i].extent.height;

    glyphs[i].offset               = offset{x, y};
    glyphs[i].texture_region.uv0.x = AS(f32, x) / atlas_extent.width;
    glyphs[i].texture_region.uv1.x = AS(f32, x + w) / atlas_extent.width;
    glyphs[i].texture_region.uv0.y = AS(f32, y) / atlas_extent.height;
    glyphs[i].texture_region.uv1.y = AS(f32, y + h) / atlas_extent.height;
  }

  u32 total_area         = atlas_extent.width * atlas_extent.height;
  f32 packing_efficiency = AS(f32, used_area) / total_area;

  ASH_LOG_INFO(FontRenderer, "Finished packing placement rectangles for font: {} with {} efficiency, rendering {}x{} CPU atlas...", font.postscript_name.c_str(), packing_efficiency, atlas_extent.width, atlas_extent.height);

  usize atlas_size = atlas_extent.area();

  ASH_CHECK(FT_Set_Char_Size(ft_face, 0, spec.height * 64, 72, 72) == 0);
 u32 scratch_width = AS(u32, (ft_face->bbox.xMax - ft_face->bbox.xMin) >> 6);
u32 scratch_height=  AS(u32, (ft_face->bbox.yMax - ft_face->bbox.yMin) >> 6);

 stx::Memory scratch_buffer_mem = stx::mem::allocate(stx::os_allocator, atlas_size).unwrap();

  stx::Memory buffer_mem = stx::mem::allocate(stx::os_allocator, atlas_size).unwrap();

  u8 *buffer = AS(u8 *, buffer_mem.handle);

  stx::Span{buffer, atlas_size}.fill(0);

  usize atlas_stride = atlas_extent.width;

  for (usize glyph_index = 0; glyph_index < glyphs.size(); glyph_index++)
  {
    Glyph const &glyph = glyphs[glyph_index];
    if (glyph.is_valid)
    {
      if (FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT) != 0)
      {
        ASH_LOG_ERR(FontRenderer, "Failed to Load glyph at index: {} for font: {}", glyph_index, font.postscript_name.c_str());
        continue;
      }

      FT_GlyphSlot slot = ft_face->glyph;

      if (FT_Render_Glyph(slot, FT_RENDER_MODE_SDF) != 0)
      {
        ASH_LOG_ERR(FontRenderer, "Failed to render SDF glyph at index: {} for font: {}", glyph_index, font.postscript_name.c_str());
        continue;
      }

      ASH_CHECK(slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO);
      ASH_CHECK(glyph.extent.width >= slot->bitmap.width);
      ASH_CHECK(glyph.extent.height >= slot->bitmap.rows);

      u8 const *bitmap           = slot->bitmap.buffer;
      usize     bitmap_row_bytes = slot->bitmap.width;

      // copy the rendered glyph to the atlas
      for (usize j = glyph.offset.y; j < glyph.offset.y + glyph.extent.height; j++)
      {
        u8 *out = buffer + j * atlas_stride + glyph.offset.x;
        std::copy(bitmap, bitmap + bitmap_row_bytes, out);
        bitmap += slot->bitmap.pitch;
      }
    }
  }

  ASH_LOG_INFO(FontRenderer, "Finished rendering CPU SDF atlas for font: {}", font.postscript_name.c_str());

  f32 ascent  = ft_face->size->metrics.ascender / 64.0f;
  f32 descent = ft_face->size->metrics.descender / -64.0f;

  ASH_CHECK(FT_Done_Face(ft_face) == 0);
  ASH_CHECK(FT_Done_FreeType(ft_lib) == 0);

  return std::make_pair(FontAtlas{.glyphs      = std::move(glyphs),
                                  .extent      = atlas_extent,
                                  .font_height = font_height,
                                  .ascent      = ascent,
                                  .descent     = descent,
                                  .texture     = 0},
                        ImageBuffer{.memory = std::move(buffer_mem), .extent = atlas_extent, .format = ImageFormat::R8});
}

}        // namespace ash
