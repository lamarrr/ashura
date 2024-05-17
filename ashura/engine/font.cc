
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIRDEF extern "C" inline

#include "ashura/engine/font.h"
#include "ashura/gfx/image.h"
#include "ashura/renderer/renderer.h"
#include "ashura/std/allocator.h"
#include "ashura/std/arena_allocator.h"
#include "ashura/std/io.h"
#include "ashura/std/range.h"
#include "ashura/std/types.h"
#include "ashura/utils/rect_pack.h"
#include "freetype/freetype.h"
#include "freetype/ftsystem.h"
#include "harfbuzz/hb.h"

namespace ash
{

constexpr u32 FONT_ATLAS_EXTENT = 1024;

static_assert(FONT_ATLAS_EXTENT != 0);
static_assert(FONT_ATLAS_EXTENT >= 16);
static_assert(FONT_ATLAS_EXTENT <= 8192);

struct FontEntry
{
  ArenaPool  pool                 = {};
  char      *postscript_name      = {};        // ASCII. i.e. RobotoBold
  usize      postscript_name_size = 0;         // ASCII. i.e. RobotoBold
  char      *family_name          = {};        // ASCII. i.e. Roboto
  usize      family_name_size     = 0;         // ASCII. i.e. Roboto
  char      *style_name           = {};        // ASCII. i.e. Bold
  usize      style_name_size      = 0;         // ASCII. i.e. Bold
  hb_blob_t *hb_blob              = nullptr;
  hb_face_t *hb_face              = nullptr;
  hb_font_t *hb_font              = nullptr;
  FT_Library ft_lib               = nullptr;
  FT_Face    ft_face              = nullptr;
  u32        selected_face        = 0;
  char      *font_data            = nullptr;
  u32        font_data_size       = 0;
  u32        num_glyphs           = 0;
  u32        replacement_glyph    = 0;
  u32        ellipsis_glyph       = 0;
  u32        space_glyph          = 0;

  // RASTERIZED ATLAS INFO
  u32             font_height = 0;
  Glyph          *glyphs      = nullptr;
  u8             *atlas       = nullptr;
  u64             atlas_size  = 0;
  u32             num_layers  = 0;
  f32             ascent      = 0;
  f32             descent     = 0;
  f32             advance     = 0;
  gfx::Image      image       = nullptr;
  gfx::ImageView *views       = nullptr;
};

void destroy_font(FontEntry *e)
{
  hb_blob_destroy(e->hb_blob);
  hb_face_destroy(e->hb_face);
  hb_font_destroy(e->hb_font);
  FT_Done_Face(e->ft_face);
  FT_Done_FreeType(e->ft_lib);
  e->pool.reset();
  // unload from gpu
}

bool load_font_from_memory(FontEntry *e, Span<u8 const> encoded_data,
                           u32 selected_face)
{
  u32   font_data_size = (u32) encoded_data.size();
  char *font_data      = e->pool.allocate_typed<char>(font_data_size);
  if (font_data == nullptr && font_data_size != 0)
  {
    return false;
  }

  defer font_data_del{[&] {
    if (font_data != nullptr)
    {
      e->pool.deallocate_typed(font_data, font_data_size);
    }
  }};

  mem::copy(encoded_data.as_char(), font_data);

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

  if (selected_face >= num_faces)
  {
    return false;
  }

  hb_face_t *hb_face = hb_face_create(hb_blob, selected_face);

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
    postscript_name      = e->pool.allocate_typed<char>(postscript_name_size);
    if (postscript_name == nullptr && postscript_name_size != 0)
    {
      return false;
    }
    mem::copy(ft_postscript_name, postscript_name, postscript_name_size);
  }

  defer postscript_name_del{[&] {
    if (postscript_name != nullptr)
    {
      e->pool.deallocate_typed(postscript_name, postscript_name_size);
    }
  }};

  char *family_name      = nullptr;
  usize family_name_size = 0;

  if (ft_face->family_name != nullptr)
  {
    family_name_size = strlen(ft_face->family_name);
    family_name      = e->pool.allocate_typed<char>(family_name_size);
    if (family_name == nullptr && family_name_size != 0)
    {
      return false;
    }
    mem::copy(ft_face->family_name, family_name, family_name_size);
  }

  defer family_name_del{[&] {
    if (family_name != nullptr)
    {
      e->pool.deallocate_typed(family_name, family_name_size);
    }
  }};

  char *style_name      = nullptr;
  usize style_name_size = 0;

  if (ft_face->style_name != nullptr)
  {
    style_name_size = strlen(ft_face->style_name);
    style_name      = e->pool.allocate_typed<char>(style_name_size);
    if (style_name == nullptr && style_name_size != 0)
    {
      return false;
    }
    mem::copy(ft_face->style_name, style_name, style_name_size);
  }

  defer style_name_del{[&] {
    if (style_name != nullptr)
    {
      e->pool.deallocate_typed(style_name, style_name_size);
    }
  }};

  u32 const num_glyphs = (u32) ft_face->num_glyphs;
  // glyph index 0 is selected if the glyph for the replacement codepoint is not
  // found
  u32 const replacement_glyph =
      FT_Get_Char_Index(ft_face, HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT);
  u32 const ellipsis_glyph = FT_Get_Char_Index(ft_face, 0x2026);
  u32 const space_glyph    = FT_Get_Char_Index(ft_face, ' ');

  new (e) FontEntry{.postscript_name      = postscript_name,
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
                    .selected_face        = selected_face,
                    .font_data            = font_data,
                    .font_data_size       = font_data_size,
                    .num_glyphs           = num_glyphs,
                    .replacement_glyph    = replacement_glyph,
                    .ellipsis_glyph       = ellipsis_glyph,
                    .space_glyph          = space_glyph};

  postscript_name = nullptr;
  family_name     = nullptr;
  style_name      = nullptr;
  hb_blob         = nullptr;
  hb_face         = nullptr;
  hb_font         = nullptr;
  ft_lib          = nullptr;
  ft_face         = nullptr;
  font_data       = nullptr;

  return true;
}

bool load_font_from_file(FontEntry *entry, AllocatorImpl const &allocator,
                         Span<char const> path, u32 selected_face)
{
  Vec<u8> data{allocator};
  defer   data_del{[&] { data.reset(); }};
  if (read_file(path, data) != IoError::None)
  {
    return false;
  }

  return load_font_from_memory(entry, to_span(data), selected_face);
}

bool render_font_atlas(FontEntry *e, u32 font_height,
                       Span<UnicodeRange const> ranges,
                       AllocatorImpl const     &allocator)
{
  CHECK(font_height <= (FONT_ATLAS_EXTENT - 16));

  // NOTE: all *64 or << 6, /64 or >> 6 are to convert to and from 26.6 pixel
  // format used in Freetype and Harfbuzz metrics
  if (FT_Set_Char_Size(e->ft_face, 0, (FT_F26Dot6) (font_height << 6), 72,
                       72) != 0)
  {
    return false;
  }

  f32 const ascent = (e->ft_face->size->metrics.ascender / 64.0f) / font_height;
  f32 const descent =
      (e->ft_face->size->metrics.descender / -64.0f) / font_height;
  f32 const advance =
      (e->ft_face->size->metrics.max_advance / 64.0f) / font_height;

  Glyph *glyphs = e->pool.allocate_typed<Glyph>(e->num_glyphs);
  if (glyphs == nullptr && e->num_glyphs != 0)
  {
    return false;
  }

  defer glyphs_del{[&] {
    if (glyphs != nullptr)
    {
      e->pool.deallocate_typed(glyphs, e->num_glyphs);
    }
  }};

  for (u32 glyph_index = 0; glyph_index < e->num_glyphs; glyph_index++)
  {
    bool const is_needed =
        glyph_index == e->replacement_glyph ? true : ranges.is_empty();

    if (FT_Load_Glyph(e->ft_face, glyph_index, FT_LOAD_DEFAULT) == 0)
    {
      FT_GlyphSlot slot = e->ft_face->glyph;

      GlyphMetrics metrics;

      // convert from 26.6 pixel format
      metrics.bearing.x = (slot->metrics.horiBearingX / 64.0f) / font_height;
      metrics.bearing.y = (slot->metrics.horiBearingY / 64.0f) / font_height;
      metrics.advance   = (slot->metrics.horiAdvance / 64.0f) / font_height;
      metrics.extent.x  = (slot->metrics.width / 64.0f) / font_height;
      metrics.extent.y  = (slot->metrics.height / 64.0f) / font_height;
      metrics.descent   = max(metrics.extent.y - metrics.bearing.y, 0.0f);

      // bin offsets are determined after binning and during rect packing
      glyphs[glyph_index] =
          Glyph{.is_valid  = true,
                .is_needed = is_needed,
                .metrics   = metrics,
                .layer     = 0,
                .offset    = Vec2U{0, 0},
                .extent    = Vec2U{slot->bitmap.width, slot->bitmap.rows}};
    }
    else
    {
      glyphs[glyph_index] = Glyph{.is_valid  = false,
                                  .is_needed = is_needed,
                                  .metrics   = {},
                                  .layer     = 0,
                                  .offset    = {},
                                  .extent    = {}};
    }
  }

  {
    // Iterate through all the characters in the font's CMAP that map a unicode
    // codepoint to a glyph
    FT_UInt  glyph_index  = 0;
    FT_ULong unicode_char = FT_Get_First_Char(e->ft_face, &glyph_index);
    do
    {
      for (UnicodeRange range : ranges)
      {
        if (unicode_char >= range.first && unicode_char <= range.last)
        {
          glyphs[glyph_index].is_needed = true;
          break;
        }
      }

      unicode_char = FT_Get_Next_Char(e->ft_face, unicode_char, &glyph_index);
    } while (glyph_index != 0);
  }

  u32 num_loaded_glyphs = 0;

  for (u32 i = 0; i < e->num_glyphs; i++)
  {
    if (glyphs[i].is_valid && glyphs[i].is_needed)
    {
      num_loaded_glyphs++;
    }
  }

  u32 num_layers = 0;
  {
    rect_pack::rect *rects =
        allocator.allocate_typed<rect_pack::rect>(num_loaded_glyphs);
    if (rects == nullptr && num_loaded_glyphs != 0)
    {
      return false;
    }

    defer rects_del{
        [&] { allocator.deallocate_typed(rects, num_loaded_glyphs); }};

    for (u32 glyph_index = 0, irect = 0; glyph_index < e->num_glyphs;
         glyph_index++)
    {
      Glyph const &g = glyphs[glyph_index];
      // only assign packing rects to the valid and needed glyphs
      if (g.is_valid && g.is_needed)
      {
        rect_pack::rect &r = rects[irect];
        r.glyph_index      = glyph_index;
        r.x                = 0;
        r.y                = 0;
        // added padding to avoid texture spilling due to accumulated
        // floating-point uv interpolation errors
        r.w = (i32) (g.extent.x + 2);
        r.h = (i32) (g.extent.y + 2);
        irect++;
      }
    }

    rect_pack::Node *nodes =
        allocator.allocate_typed<rect_pack::Node>(FONT_ATLAS_EXTENT);

    if (nodes == nullptr && FONT_ATLAS_EXTENT != 0)
    {
      return false;
    }

    defer nodes_del{
        [&] { allocator.deallocate_typed(nodes, FONT_ATLAS_EXTENT); }};

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
      num_packed += just_packed.span;
      num_layers++;
    }

    // sanity check. ideally all should have been packed
    CHECK(all_packed);

    for (u32 i = 0; i < num_loaded_glyphs; i++)
    {
      rect_pack::rect r = rects[i];
      Glyph          &g = glyphs[r.glyph_index];
      g.offset.x        = (u32) r.x + 1;
      g.offset.y        = (u32) r.y + 1;
      g.layer           = r.layer;
      g.uv0             = Vec2{g.offset.x / (f32) FONT_ATLAS_EXTENT,
                   g.offset.y / (f32) FONT_ATLAS_EXTENT};
      g.uv1 = Vec2{(g.offset.x + g.extent.x) / (f32) FONT_ATLAS_EXTENT,
                   (g.offset.y + g.extent.y) / (f32) FONT_ATLAS_EXTENT};
    }
  }

  u64 const atlas_area  = (u64) FONT_ATLAS_EXTENT * (u64) FONT_ATLAS_EXTENT;
  u32 const atlas_pitch = FONT_ATLAS_EXTENT * 4;
  u64 const atlas_layer_size = atlas_area * 4;
  u64 const atlas_size       = atlas_layer_size * num_layers;
  u8       *atlas            = e->pool.allocate_zeroed_typed<u8>(atlas_size);

  if (atlas == nullptr && atlas_size != 0)
  {
    return false;
  }

  defer atlas_buffer_del{[&] {
    if (atlas != nullptr)
    {
      e->pool.deallocate_typed(atlas, atlas_size);
    }
  }};

  for (u32 glyph_index = 0; glyph_index < e->num_glyphs; glyph_index++)
  {
    Glyph const &g = glyphs[glyph_index];
    if (g.is_valid && g.is_needed)
    {
      FT_Error ft_error =
          FT_Load_Glyph(e->ft_face, glyph_index, FT_LOAD_DEFAULT);
      if (ft_error != 0)
      {
        continue;
      }

      FT_GlyphSlot slot = e->ft_face->glyph;
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

  e->font_height = font_height;
  e->glyphs      = glyphs;
  e->atlas       = atlas;
  e->atlas_size  = atlas_size;
  e->num_layers  = num_layers;
  e->ascent      = ascent;
  e->descent     = descent;
  e->advance     = advance;

  glyphs = nullptr;
  atlas  = nullptr;

  return true;
}

// copy to GPU on main thread
gfx::Status font_to_gpu(FontEntry *e, Renderer &r,
                        gfx::CommandEncoderImpl const &enc,
                        gfx::DeviceImpl const         &device,
                        AllocatorImpl const           &allocator)
{
  CHECK(e->num_layers > 0);
  gfx::Image image =
      device
          ->create_image(
              device.self,
              gfx::ImageDesc{
                  .label = "Font Atlas Image"_span,
                  .type  = gfx::ImageType::Type2D,
                  .usage = gfx::ImageUsage::Sampled |
                           gfx::ImageUsage::InputAttachment |
                           gfx::ImageUsage::Storage |
                           gfx::ImageUsage::TransferSrc |
                           gfx::ImageUsage::TransferDst,
                  .aspects      = gfx::ImageAspects::Color,
                  .extent       = {FONT_ATLAS_EXTENT, FONT_ATLAS_EXTENT, 1},
                  .mip_levels   = 1,
                  .array_layers = e->num_layers,
                  .sample_count = gfx::SampleCount::Count1})
          .unwrap();

  gfx::ImageView *views = e->pool.allocate_typed<gfx::ImageView>(e->num_layers);
  if (views == nullptr)
  {
    return gfx::Status::OutOfHostMemory;
  }

  defer views_del{[&] {
    if (views != nullptr)
    {
      e->pool.deallocate_typed(views, e->num_layers);
    }
  }};

  for (u32 i = 0; i < e->num_layers; i++)
  {
    views[i] =
        device
            ->create_image_view(
                device.self,
                gfx::ImageViewDesc{.label       = "Font Atlas Image View"_span,
                                   .image       = image,
                                   .view_type   = gfx::ImageViewType::Type2D,
                                   .view_format = gfx::Format::B8G8R8A8_UNORM,
                                   .mapping     = {},
                                   .aspects     = gfx::ImageAspects::Color,
                                   .first_mip_level   = 0,
                                   .num_mip_levels    = 1,
                                   .first_array_layer = i,
                                   .num_array_layers  = 1})
            .unwrap();
  }

  gfx::BufferImageCopy *copies =
      allocator.allocate_typed<gfx::BufferImageCopy>(e->num_layers);

  if (copies == nullptr)
  {
    return gfx::Status::OutOfHostMemory;
  }

  defer copies_del{[&] { allocator.deallocate_typed(copies, e->num_layers); }};

  // staging buffer, add copy command
  // copy to staging buffer

  for (u32 i = 0; i < e->num_layers; i++)
  {
    u64 offset =
        (u64) FONT_ATLAS_EXTENT * (u64) FONT_ATLAS_EXTENT * 4 * (u64) i;
    copies[i] = gfx::BufferImageCopy{
        .buffer_offset       = 0x000 + offset,
        .buffer_row_length   = 0,
        .buffer_image_height = 0,
        .image_layers =
            gfx::ImageSubresourceLayers{.aspects   = gfx::ImageAspects::Color,
                                        .mip_level = 0,
                                        .first_array_layer = i,
                                        .num_array_layers  = 1},
        .image_offset = {0, 0, 0},
        .image_extent = {FONT_ATLAS_EXTENT, FONT_ATLAS_EXTENT, 1}};
  }

  enc->copy_buffer_to_image(enc.self, nullptr, image,
                            Span{copies, e->num_layers});

  e->views = views;

  return gfx::Status::Success;
}

}        // namespace ash