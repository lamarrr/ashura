#pragma once
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <utility>

#include "ashura/image.h"
#include "ashura/version.h"
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
  stx::String  postscript_name;        // ASCII. i.e. RobotoBold
  stx::String  family_name;            // ASCII. i.e. Roboto
  stx::String  style_name;             // ASCII. i.e. Bold
  hb_blob_t   *hb_blob       = nullptr;
  hb_face_t   *hb_face       = nullptr;
  hb_font_t   *hb_font       = nullptr;
  hb_buffer_t *hb_buffer     = nullptr;
  u32          nfaces        = 0;
  u32          selected_face = 0;
  stx::Vec<u8> data;

  Font(stx::String ipostscript_name, stx::String ifamily_name, stx::String istyle_name,
       hb_blob_t *ihb_blob, hb_face_t *ihb_face, hb_font_t *ihb_font, hb_buffer_t *ihb_buffer,
       stx::Vec<u8> idata, u32 infaces, u32 iselected_face) :
      postscript_name{std::move(ipostscript_name)},
      family_name{std::move(ifamily_name)},
      style_name{std::move(istyle_name)},
      hb_blob{ihb_blob},
      hb_face{ihb_face},
      hb_font{ihb_font},
      hb_buffer{ihb_buffer},
      data{std::move(idata)},
      nfaces{infaces},
      selected_face{iselected_face}
  {}

  STX_MAKE_PINNED(Font)

  ~Font()
  {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(hb_face);
    hb_font_destroy(hb_font);
    hb_buffer_destroy(hb_buffer);
  }
};

inline stx::Result<stx::Rc<Font *>, FontLoadError> load_font_from_memory(stx::Vec<u8> data)
{
  hb_blob_t *hb_blob = hb_blob_create(data.span().as_char().data(), AS(uint, data.size()), HB_MEMORY_MODE_READONLY, nullptr, nullptr);

  ASH_CHECK(hb_blob != nullptr);

  u32 selected_face = 0;
  u32 nfaces        = hb_face_count(hb_blob);

  hb_face_t *hb_face = hb_face_create(hb_blob, selected_face);

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

  if (FT_New_Memory_Face(ft_lib, data.data(), AS(FT_Long, data.size()), 0, &ft_face) != 0)
  {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(hb_face);
    hb_buffer_destroy(hb_buffer);
    ASH_CHECK(FT_Done_FreeType(ft_lib) == 0);
    return stx::Err(FontLoadError::InvalidFont);
  }

  char const *p_postscript_name = FT_Get_Postscript_Name(ft_face);
  stx::String postscript_name   = p_postscript_name == nullptr ? "" : stx::string::make(stx::os_allocator, postscript_name).unwrap();
  stx::String family_name       = ft_face->family_name == nullptr ? "" : stx::string::make(stx::os_allocator, ft_face->family_name).unwrap();
  stx::String style_name        = ft_face->style_name == nullptr ? "" : stx::string::make(stx::os_allocator, ft_face->style_name).unwrap();

  ASH_CHECK(FT_Done_Face(ft_face) == 0);
  ASH_CHECK(FT_Done_FreeType(ft_lib) == 0);

  return stx::Ok(stx::rc::make_inplace<Font>(stx::os_allocator, std::move(postscript_name), std::move(family_name), std::move(style_name),
                                             hb_blob, hb_face, hb_font, hb_buffer, std::move(data), nfaces, selected_face)
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

  stx::Vec<u8> data;
  data.unsafe_resize_uninitialized(file_size);

  ASH_CHECK(std::fseek(file, 0, SEEK_SET) == 0);

  ASH_CHECK(std::fread(data.data(), 1, file_size, file) == file_size);

  ASH_CHECK(std::fclose(file) == 0);

  return load_font_from_memory(std::move(data));
}

/// atlas containing the packed glyphs
/// This enable support of large glyphs. We load all glyphs of a font into memory.
/// GPUs have texture size limits so we try to bin the font textures.
///
struct FontAtlasBin
{
  gfx::image texture = 0;
  extent     extent;
  usize      used_area = 0;
};

struct GlyphMetrics
{
  vec2 bearing;            // offset from cursor baseline to start drawing glyph from
  f32  descent = 0;        // distance from baseline to the bottom of the glyph
  vec2 advance;            // advancement of the cursor after drawing this glyph
};

/// see: https://stackoverflow.com/questions/62374506/how-do-i-align-glyphs-along-the-baseline-with-freetype
///
/// NOTE: using stubs enables us to perform fast constant lookups of glyph indices by ensuring the array is filled and sorted by glyph index from 0 -> nglyphs_found_in_font-1
struct Glyph
{
  bool         is_valid = false;
  u32          bin      = 0;
  texture_rect bin_region;        // texture coordinates of this glyph in the atlas bin
  offset       offset;            // offset into the atlas this glyph's SDF is placed
  extent       extent;            // extent of the glyph's SDF in the atlas
  GlyphMetrics metrics;
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
  u32                    replacement_glyph = 0;        // glyph for replacement character 0xFFFD. only valid if font is not empty
  u32                    font_height       = 0;        // font height at which the this atlas was rendered
  u32                    sdf_spread        = 0;        // signed distance field spread factor at downsampled_font_height
  f32                    ascent            = 0;        // maximum ascent of the font's glyphs at font_height
  f32                    descent           = 0;        // maximum descent of the font's glyphs at font_height
  stx::Vec<FontAtlasBin> bins;
};

// TODO(lamarrr): we'll need an SDF offset for placement of the glyphs
struct SdfFontSpec
{
  u32    font_height          = 64;                                  // the height at which the SDF texture is cached at
  u32    sdf_spread           = 8;                                   // spread width of the SDF field
  u32    sdf_upscale_factor   = 16;                                  // factor to upsacle the 1-bit alpha texture from which the SDF is calculated from
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

      i64 distance        = (u8) (127 * std::sqrt((float) square_distance) / sdf_spread);
      i64 signed_distance = 127 + (is_inside ? distance : -distance);

      output[i * output_pitch + j] = (u8) signed_distance;
    }
  }
}

// fonts are identified by their post script names
inline std::pair<FontAtlas, stx::Vec<ImageBuffer>> load_font_sdf_from_file(Font const &font, std::string_view cache_directory, Version engine_version);

inline std::pair<FontAtlas, stx::Vec<ImageBuffer>> render_font_atlas(Font const &font, SdfFontSpec const &spec)
{
  FT_Library ft_lib;
  ASH_CHECK(FT_Init_FreeType(&ft_lib) == 0);

  FT_Face ft_face;
  ASH_CHECK(FT_New_Memory_Face(ft_lib, font.data.data(), AS(FT_Long, font.data.size()), font.selected_face, &ft_face) == 0);

  // *64 to convert font height to 26.6 pixel format
  ASH_CHECK(FT_Set_Char_Size(ft_face, 0, spec.font_height << 6, 72, 72) == 0);

  stx::Vec<Glyph> glyphs;

  u32 const replacement_glyph = FT_Get_Char_Index(ft_face, HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT);

  ASH_LOG_INFO(FontRenderer, "Fetching Glyph Metrics For Font: {}", font.postscript_name.c_str());

  usize nvalid_glyphs = 0;

  for (usize glyph_index = 0; glyph_index < ft_face->num_glyphs; glyph_index++)
  {
    if (FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT) == 0) [[likely]]
    {
      FT_GlyphSlot slot   = ft_face->glyph;
      u32          width  = slot->bitmap.width;
      u32          height = slot->bitmap.rows;

      GlyphMetrics metrics;

      // convert from 26.6 pixel format
      metrics.bearing.x = slot->metrics.horiBearingX / 64.0f;
      metrics.bearing.y = slot->metrics.horiBearingY / 64.0f;

      metrics.descent = std::max(AS(f32, height) - metrics.bearing.y, 0.0f);

      metrics.advance.x = slot->advance.x / 64.0f;
      metrics.advance.y = slot->advance.y / 64.0f;

      u32 sdf_width  = width + spec.sdf_spread * 2;
      u32 sdf_height = height + spec.sdf_spread * 2;

      glyphs
          .push(Glyph{.is_valid   = true,
                      .bin        = 0,
                      .bin_region = {},
                      .offset     = {},
                      .extent     = extent{sdf_width, sdf_height},
                      .metrics    = metrics})
          .unwrap();

      nvalid_glyphs++;
    }
    else
    {
      glyphs
          .push(Glyph{.is_valid   = false,
                      .bin        = 0,
                      .bin_region = {},
                      .offset     = {},
                      .extent     = {},
                      .metrics    = {}})
          .unwrap();
    }
  }

  ASH_LOG_INFO(FontRenderer, "Bin Packing Glyphs For Font: {}", font.postscript_name.c_str());

  stx::Vec<rect_packer::rect> rects;
  rects.unsafe_resize_uninitialized(nvalid_glyphs).unwrap();

  for (usize glyph_index = 0, irect = 0; glyph_index < glyphs.size(); glyph_index++)
  {
    if (glyphs[glyph_index].is_valid) [[likely]]
    {
      rects[irect].glyph_index = glyph_index;
      rects[irect].x           = 0;
      rects[irect].y           = 0;

      // added padding to avoid texture spilling due to accumulated uv interpolation errors
      rects[irect].w = AS(i32, glyphs[glyph_index].extent.width + 2);
      rects[irect].h = AS(i32, glyphs[glyph_index].extent.height + 2);
      irect++;
    }
  }

  stx::Vec<rect_packer::Node> nodes;
  nodes.unsafe_resize_uninitialized(spec.max_atlas_bin_extent.width).unwrap();

  rect_packer::Context pack_context = rect_packer::init(spec.max_atlas_bin_extent.width,
                                                        spec.max_atlas_bin_extent.height,
                                                        nodes.data(),
                                                        spec.max_atlas_bin_extent.width,
                                                        false);

  stx::Vec<FontAtlasBin> bins;
  usize                  total_used_area = 0;
  usize                  total_area      = 0;

  {
    u32                          bin            = 0;
    stx::Span<rect_packer::rect> unpacked_rects = rects;
    bool                         was_all_packed = rects.is_empty();

    while (!unpacked_rects.is_empty())
    {
      // tries to pack all the glyph rects into the provided extent
      was_all_packed               = rect_packer::pack_rects(pack_context, unpacked_rects.data(), AS(i32, unpacked_rects.size()));
      auto [just_packed, unpacked] = unpacked_rects.partition([](rect_packer::rect const &r) { return r.was_packed; });
      unpacked_rects               = unpacked;

      // NOTE: vulkan doesn't allow zero-extent images
      extent bin_extent{1, 1};
      usize  used_area = 0;

      for (rect_packer::rect const &rect : just_packed)
      {
        bin_extent.width  = std::max(bin_extent.width, (u32) (rect.x + rect.w));
        bin_extent.height = std::max(bin_extent.height, (u32) (rect.y + rect.h));
        used_area += rect.w * rect.h;
      }

      for (rect_packer::rect const &rect : just_packed)
      {
        u32 x = AS(u32, rect.x) + 1;
        u32 y = AS(u32, rect.y) + 1;

        Glyph &glyph = glyphs[rect.glyph_index];

        u32 w = glyph.extent.width - 1;
        u32 h = glyph.extent.height - 1;

        glyph.bin              = bin;
        glyph.offset           = offset{x, y};
        glyph.bin_region.uv0.x = AS(f32, x) / bin_extent.width;
        glyph.bin_region.uv1.x = AS(f32, x + w) / bin_extent.width;
        glyph.bin_region.uv0.y = AS(f32, y) / bin_extent.height;
        glyph.bin_region.uv1.y = AS(f32, y + h) / bin_extent.height;
      }

      bins.push(FontAtlasBin{.texture = gfx::WHITE_IMAGE, .extent = bin_extent, .used_area = used_area}).unwrap();
      bin++;

      total_used_area += used_area;
      total_area += bin_extent.area();
    }

    ASH_CHECK(was_all_packed);        // sanity check
  }

  f32 const   packing_efficiency = AS(f32, total_used_area) / total_area;
  usize const total_wasted_area  = total_area - total_used_area;

  ASH_LOG_INFO(FontRenderer,
               "Finished Bin Packing Glyphs For Font: {} Into {} Bins With {} Efficiency (Wasted = {}, Used = {}, Total = {}) ",
               font.postscript_name.c_str(), bins.size(), packing_efficiency, total_wasted_area, total_used_area, total_area);

  u32 const upscaled_font_height = spec.sdf_upscale_factor * spec.font_height;
  u32 const upscaled_spread      = spec.sdf_spread * spec.sdf_upscale_factor;

  ASH_CHECK(FT_Set_Char_Size(ft_face, 0, upscaled_font_height << 6, 72, 72) == 0);

  u32 const scratch_width  = AS(u32, (ft_face->bbox.xMax - ft_face->bbox.xMin) >> 6);
  u32 const scratch_height = AS(u32, (ft_face->bbox.yMax - ft_face->bbox.yMin) >> 6);

  u32 const scratch_sdf_width  = scratch_width + upscaled_spread * 2;
  u32 const scratch_sdf_height = scratch_height + upscaled_spread * 2;

  ImageBuffer scratch_buffer = ImageBuffer::make(extent{scratch_sdf_width, scratch_sdf_height}, ImageFormat::R8).unwrap();

  stx::Vec<ImageBuffer> bin_buffers;

  for (FontAtlasBin const &bin : bins)
  {
    ImageBuffer buffer = ImageBuffer::make(bin.extent, ImageFormat::R8).unwrap();
    buffer.span().fill(0);
    bin_buffers.push(std::move(buffer)).unwrap();
  }

  for (usize glyph_index = 0; glyph_index < glyphs.size(); glyph_index++)
  {
    Glyph const &glyph = glyphs[glyph_index];
    if (glyph.is_valid)
    {
      FT_Error ft_error = FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT);
      if (ft_error != 0) [[unlikely]]
      {
        ASH_LOG_ERR(FontRenderer, "Failed To Load Glyph At Index: {} For Font: {} (FT_Error = {})", glyph_index, font.postscript_name.c_str(), ft_error);
        continue;
      }

      FT_GlyphSlot slot = ft_face->glyph;
      ft_error          = FT_Render_Glyph(slot, FT_RENDER_MODE_MONO);
      if (ft_error != 0) [[unlikely]]
      {
        ASH_LOG_ERR(FontRenderer, "Failed To Render Glyph At Index: {} for font: {}", glyph_index, font.postscript_name.c_str(), ft_error);
        continue;
      }

      ASH_CHECK(slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO);
      /// TODO(lamarrr): the offsets will be off and need adjustment?

      ImageBuffer &bin_buffer = bin_buffers[glyph.bin];

      u32 const upscaled_sdf_width  = slot->bitmap.width + upscaled_spread * 2;
      u32 const upscaled_sdf_height = slot->bitmap.rows + upscaled_spread * 2;

      generate_sdf_from_mono(slot->bitmap.buffer,
                             slot->bitmap.pitch,
                             slot->bitmap.width,
                             slot->bitmap.rows,
                             upscaled_spread,
                             scratch_buffer.data(),
                             upscaled_sdf_width);

      u8 *out = bin_buffer.data() + glyph.offset.y * bin_buffer.pitch() + glyph.offset.x;

      // TODO(lamarrr): scaling is needed for the spread
      stbir_resize_uint8(scratch_buffer.data(),
                         upscaled_sdf_width,
                         upscaled_sdf_height,
                         upscaled_sdf_width,
                         out,
                         glyph.extent.width,
                         glyph.extent.height,
                         bin_buffer.pitch(),
                         1);
    }
  }

  ASH_LOG_INFO(FontRenderer, "Finished Caching SDF Atlas Bins For Font: {}", font.postscript_name.c_str());

  // *64 or << 6 to convert font height to 26.6 pixel format
  ASH_CHECK(FT_Set_Char_Size(ft_face, 0, spec.font_height << 6, 72, 72) == 0);

  f32 const ascent  = ft_face->size->metrics.ascender / 64.0f;
  f32 const descent = ft_face->size->metrics.descender / -64.0f;

  ASH_CHECK(FT_Done_Face(ft_face) == 0);
  ASH_CHECK(FT_Done_FreeType(ft_lib) == 0);

  return std::make_pair(FontAtlas{
                            .glyphs            = std::move(glyphs),
                            .replacement_glyph = replacement_glyph,
                            .font_height       = spec.font_height,
                            .sdf_spread        = spec.sdf_spread,
                            .ascent            = ascent,
                            .descent           = descent},
                        std::move(bin_buffers));
}

}        // namespace ash
