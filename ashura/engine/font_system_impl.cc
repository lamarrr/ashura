/// SPDX-License-Identifier: MIT
#include "ashura/engine/font_system_impl.h"
#include "ashura/engine/font_impl.h"
#include "ashura/engine/rect_pack.h"
#include "ashura/engine/systems.h"
#include "ashura/std/range.h"
#include "ashura/std/vec.h"

extern "C"
{
#include "SBAlgorithm.h"
#include "SBParagraph.h"
#include "SBScriptLocator.h"
#include "hb.h"
}

namespace ash
{

Dyn<FontSystem *> FontSystem::create(AllocatorRef allocator)
{
  hb_buffer_t * hb_buffer = hb_buffer_create();
  CHECK(hb_buffer != nullptr && hb_buffer_allocation_successful(hb_buffer), "");

  return cast<FontSystem *>(
    dyn<FontSystemImpl>(inplace, allocator, allocator, hb_buffer).unwrap());
}

FontSystemImpl::~FontSystemImpl()
{
  hb_buffer_destroy(hb_buffer_);
}

void FontSystemImpl::shutdown()
{
  while (!fonts_.is_empty())
  {
    unload(FontId{fonts_.to_id(0)});
  }
}

Result<Dyn<Font *>, FontLoadErr>
  FontSystemImpl::decode_(Str label_ref, Span<u8 const> encoded, u32 face)
{
  Vec<char> font_data{allocator_};
  if (!font_data.extend(encoded.as_char()))
  {
    return Err{FontLoadErr::OutOfMemory};
  }

  hb_blob_t * hb_blob =
    hb_blob_create(font_data.data(), font_data.size(), HB_MEMORY_MODE_READONLY,
                   nullptr, nullptr);

  if (hb_blob == nullptr)
  {
    return Err{FontLoadErr::DecodeFailed};
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
    return Err{FontLoadErr::FaceNotFound};
  }

  hb_face_t * hb_face = hb_face_create(hb_blob, face);

  if (hb_face == nullptr)
  {
    return Err{FontLoadErr::DecodeFailed};
  }

  defer hb_face_{[&] {
    if (hb_face != nullptr)
    {
      hb_face_destroy(hb_face);
    }
  }};

  hb_font_t * hb_font = hb_font_create(hb_face);

  if (hb_font == nullptr)
  {
    return Err{FontLoadErr::DecodeFailed};
  }

  hb_font_set_scale(hb_font, AU_UNIT, AU_UNIT);

  defer hb_font_{[&] {
    if (hb_font != nullptr)
    {
      hb_font_destroy(hb_font);
    }
  }};

  FT_Library ft_lib;
  if (FT_Error err = FT_Init_FreeType(&ft_lib); err != 0)
  {
    return Err{FontLoadErr::DecodeFailed};
  }

  defer ft_lib_{[&] {
    if (ft_lib != nullptr)
    {
      FT_Done_FreeType(ft_lib);
    }
  }};

  FT_Face ft_face;

  if (FT_Error err =
        FT_New_Memory_Face(ft_lib, (FT_Byte const *) font_data.data(),
                           (FT_Long) font_data.size(), 0, &ft_face);
      err != 0)
  {
    return Err{FontLoadErr::DecodeFailed};
  }

  if (FT_Error err = FT_Set_Char_Size(ft_face, AU_UNIT, AU_UNIT, 72, 72);
      err != 0)
  {
    return Err{FontLoadErr::DecodeFailed};
  }

  defer ft_face_{[&] {
    if (ft_face != nullptr)
    {
      FT_Done_Face(ft_face);
    }
  }};

  bool const has_color = FT_HAS_COLOR(ft_face);

  char const * ft_postscript_name = FT_Get_Postscript_Name(ft_face);

  FontImpl::Name postscript_name;
  FontImpl::Name family_name;
  FontImpl::Name style_name;

  if (ft_postscript_name != nullptr)
  {
    postscript_name.extend(cstr_span(ft_postscript_name)).unwrap();
  }

  if (ft_face->family_name != nullptr)
  {
    family_name.extend(cstr_span(ft_face->family_name)).unwrap();
  }

  if (ft_face->style_name != nullptr)
  {
    style_name.extend(cstr_span(ft_face->style_name)).unwrap();
  }

  u32 const num_glyphs        = (u32) ft_face->num_glyphs;
  // glyph 0 is selected if the replacement codepoint glyph is not found
  u32 const replacement_glyph = FT_Get_Char_Index(ft_face, 0xFFFD);
  u32 const ellipsis_glyph    = FT_Get_Char_Index(ft_face, 0x2026);
  u32 const space_glyph       = FT_Get_Char_Index(ft_face, ' ');

  // expressed on a AU_UNIT scale
  i32 const ascent  = ft_face->size->metrics.ascender;
  i32 const descent = -ft_face->size->metrics.descender;
  i32 const advance = ft_face->size->metrics.max_advance;

  Vec<GlyphMetrics> glyphs{allocator_};

  if (!glyphs.resize(num_glyphs))
  {
    return Err{FontLoadErr::OutOfMemory};
  }

  for (auto [i, metric] : enumerate<u32>(glyphs))
  {
    if (FT_Error err = FT_Load_Glyph(ft_face, i, FT_LOAD_DEFAULT); err != 0)
    {
      continue;
    }

    FT_GlyphSlot s = ft_face->glyph;

    // bin offsets are determined after binning and during rect packing
    metric = GlyphMetrics{
      .bearing{(i32) s->metrics.horiBearingX, (i32) -s->metrics.horiBearingY},
      .advance = (i32) s->metrics.horiAdvance,
      .extent{(i32) s->metrics.width,        (i32) s->metrics.height       }
    };
  }

  Vec<char> label{allocator_};

  if (!label.extend(label_ref))
  {
    return Err{FontLoadErr::OutOfMemory};
  }

  Result font = dyn<FontImpl>(
    inplace, allocator_, FontId::None, std::move(label), std::move(font_data),
    has_color, std::move(postscript_name), std::move(family_name),
    std::move(style_name), hb_blob, hb_face, hb_font, ft_lib, ft_face, face,
    std::move(glyphs), replacement_glyph, ellipsis_glyph, space_glyph,
    FontMetrics{.ascent = ascent, .descent = descent, .advance = advance});

  if (!font)
  {
    return Err{FontLoadErr::OutOfMemory};
  }

  hb_blob = nullptr;
  hb_face = nullptr;
  hb_font = nullptr;
  ft_lib  = nullptr;
  ft_face = nullptr;

  return Ok{cast<ash::Font *>(std::move(font.v()))};
}

Result<> FontSystemImpl::rasterize(Font & font_, u32 font_height)
{
  FontImpl &           font             = (FontImpl &) font_;
  static constexpr u32 MIN_ATLAS_EXTENT = 512;
  static_assert(MIN_ATLAS_EXTENT > 0, "Font atlas extent must be non-zero");
  static_assert(MIN_ATLAS_EXTENT >= 128,
                "Font atlas extent must be at least 128px");
  static_assert(MIN_ATLAS_EXTENT % 64 == 0,
                "Font atlas extent should be a multiple of 64");
  static_assert(MIN_ATLAS_EXTENT <= gpu::MAX_IMAGE_EXTENT_2D,
                "Font atlas extent too large for GPU platform");

  font.cpu_atlas.unwrap_none("CPU font atlas has already been loaded"_str);

  CpuFontAtlas atlas;

  u32 const num_glyphs = font.glyphs.size32();

  if (!atlas.glyphs.resize(num_glyphs))
  {
    return Err{};
  }

  if (FT_Error err = FT_Set_Pixel_Sizes(font.ft_face, font_height, font_height);
      err != 0)
  {
    return Err{};
  }

  static constexpr u16 GLYPH_PADDING = 1;

  Vec2U max_glyph_extent;

  for (auto [i, g] : enumerate<u32>(atlas.glyphs))
  {
    if (FT_Error err = FT_Load_Glyph(font.ft_face, i, FT_LOAD_DEFAULT);
        err != 0)
    {
      continue;
    }

    g.area.extent = Vec2U{font.ft_face->glyph->bitmap.width,
                          font.ft_face->glyph->bitmap.rows};

    max_glyph_extent.x = max(max_glyph_extent.x, g.area.extent.x);
    max_glyph_extent.y = max(max_glyph_extent.y, g.area.extent.y);
  }

  CHECK(max_glyph_extent.x <= MIN_ATLAS_EXTENT, "");
  CHECK(max_glyph_extent.y <= MIN_ATLAS_EXTENT, "");

  Vec2U const atlas_extent     = Vec2U::splat(MIN_ATLAS_EXTENT);
  Vec2 const  inv_atlas_extent = 1 / as_vec2(atlas_extent);

  u32 num_layers = 0;
  {
    Vec<rect_pack::rect> rects{allocator_};

    if (!rects.resize_uninit(num_glyphs))
    {
      return Err{};
    }

    for (auto [i, gl, ag, rect] :
         zip(range(size32(font.glyphs)), font.glyphs, atlas.glyphs, rects))
    {
      // added padding to avoid texture spilling due to accumulated
      // floating-point uv interpolation errors
      Vec2U padded_extent{};

      if (ag.area.extent.x != 0 && ag.area.extent.y != 0)
      {
        padded_extent = ag.area.extent + GLYPH_PADDING * 2;
      }

      rect = rect_pack::rect{.id         = i,
                             .extent     = as_vec2i(padded_extent),
                             .pos        = {},
                             .was_packed = false};
    }

    Vec<rect_pack::Node> nodes{allocator_};
    u32 const            num_nodes = atlas_extent.x;
    nodes.resize_uninit(num_nodes).unwrap();

    Span<rect_pack::rect> unpacked = rects;

    while (!unpacked.is_empty())
    {
      // tries to pack all the glyph rects into the provided extent
      rect_pack::Context ctx;
      rect_pack::init(ctx, as_vec2i(atlas_extent), nodes.data(),
                      (i32) num_nodes);
      rect_pack::pack_rects(ctx, unpacked.data(), (i32) unpacked.size32());

      auto [just_packed, still_unpacked] = partition(
        unpacked, [](rect_pack::rect const & r) { return r.was_packed; });

      CHECK(!just_packed.is_empty(), "");

      for (rect_pack::rect const & r : just_packed)
      {
        atlas.glyphs[r.id].layer = num_layers;
      }

      unpacked = still_unpacked;

      num_layers++;
    }

    for (auto [i, r] : enumerate(rects))
    {
      auto & g = atlas.glyphs[r.id];

      if (g.area.extent.x == 0 | g.area.extent.y == 0)
      {
        g.area.offset = Vec2U{};
      }
      else
      {
        // adjust back to original position from the padded position
        g.area.offset = as_vec2u(r.pos + GLYPH_PADDING);
      }

      g.uv[0] = as_vec2(g.area.offset) * inv_atlas_extent;
      g.uv[1] = as_vec2(g.area.end()) * inv_atlas_extent;
    }
  }

  u64 const atlas_layer_size = (u64) atlas_extent.x * (u64) atlas_extent.y * 4;
  u64 const atlas_size       = atlas_layer_size * num_layers;

  if (!atlas.channels.resize(atlas_size))
  {
    return Err{};
  }

  ImageLayerSpan<u8, 4> atlas_span{
    .channels = atlas.channels, .extent = atlas_extent, .layers = num_layers};

  for (auto [i, ag] : enumerate<u32>(atlas.glyphs))
  {
    if (FT_Error err = FT_Load_Glyph(
          font.ft_face, i, FT_LOAD_DEFAULT | FT_LOAD_COLOR | FT_LOAD_RENDER);
        err != 0)
    {
      continue;
    }

    FT_GlyphSlot slot = font.ft_face->glyph;

    /// we don't want to handle negative pitches
    CHECK(slot->bitmap.pitch >= 0, "");

    switch (slot->bitmap.pixel_mode)
    {
      case FT_PIXEL_MODE_GRAY:
      {
        ImageSpan<u8 const, 1> src{
          .channels{slot->bitmap.buffer,
                    slot->bitmap.rows * (u32) slot->bitmap.pitch},
          .extent{slot->bitmap.width,  slot->bitmap.rows      },
          .stride = (u32) slot->bitmap.pitch
        };

        copy_alpha_image_to_BGRA(
          src, atlas_span.layer(ag.layer).slice(ag.area.offset, ag.area.extent),
          (u8) 0xFFU, (u8) 0xFFU, (u8) 0xFFU);

        ag.has_color = false;
      }
      break;
      case FT_PIXEL_MODE_BGRA:
      {
        ImageSpan<u8 const, 4> src{
          .channels{slot->bitmap.buffer,
                    slot->bitmap.rows * (u32) slot->bitmap.pitch},
          .extent{slot->bitmap.width,  slot->bitmap.rows      },
          .stride = (u32) slot->bitmap.pitch / 4
        };

        copy_image(src, atlas_span.layer(ag.layer).slice(ag.area.offset,
                                                         ag.area.extent));

        ag.has_color = true;
      }
      break;
      default:
        CHECK(false, "Unrecognized pixel mode {}", slot->bitmap.pixel_mode);
    }
  }

  atlas.font_height = font_height;
  atlas.extent      = atlas_extent;
  atlas.num_layers  = num_layers;

  font.cpu_atlas = std::move(atlas);

  return Ok{};
}

FontId FontSystemImpl::upload_(Dyn<Font *> font_)
{
  FontImpl & font = (FontImpl &) *font_.get();
  CHECK(font.cpu_atlas.is_some(), "");
  CHECK(font.gpu_atlas.is_none(), "");

  CpuFontAtlas & atlas = font.cpu_atlas.v();

  CHECK(atlas.num_layers > 0, "");
  CHECK(atlas.extent.x > 0, "");
  CHECK(atlas.extent.y > 0, "");

  GpuFontAtlas gpu_atlas{.textures{allocator_},
                         .font_height = atlas.font_height,
                         .extent      = atlas.extent,
                         .glyphs{allocator_}};

  gpu_atlas.glyphs.extend(atlas.glyphs).unwrap();

  constexpr gpu::Format   format = gpu::Format::B8G8R8A8_UNORM;
  Vec<gpu::ImageViewInfo> view_infos;

  for (u32 i = 0; i < atlas.num_layers; i++)
  {
    view_infos
      .push(gpu::ImageViewInfo{.label             = font.label,
                               .view_type         = gpu::ImageViewType::Type2D,
                               .view_format       = format,
                               .mapping           = {},
                               .aspects           = gpu::ImageAspects::Color,
                               .first_mip_level   = 0,
                               .num_mip_levels    = 1,
                               .first_array_layer = i,
                               .num_array_layers  = 1})
      .unwrap();
  }

  ImageInfo image =
    sys->image
      .load_from_memory(font.label.clone().unwrap(),
                        gpu::ImageInfo{
                          .label  = font.label,
                          .type   = gpu::ImageType::Type2D,
                          .format = format,
                          .usage  = gpu::ImageUsage::Sampled |
                                   gpu::ImageUsage::TransferDst |
                                   gpu::ImageUsage::TransferSrc,
                          .aspects = gpu::ImageAspects::Color,
                          .extent{atlas.extent.x, atlas.extent.y, 1},
                          .mip_levels   = 1,
                          .array_layers = atlas.num_layers,
                          .sample_count = gpu::SampleCount::C1
  },
                        view_infos, atlas.channels)
      .unwrap();

  gpu_atlas.textures.extend(image.textures).unwrap();
  gpu_atlas.image = image.id;

  font.gpu_atlas = std::move(gpu_atlas);

  // unload CPU atlas
  font.cpu_atlas = none;

  FontId id = FontId{fonts_.push(std::move(font_)).unwrap()};

  FontImpl & f = (FontImpl &) *fonts_[(usize) id].v0;

  f.id = id;

  return id;
}

Future<Result<FontId, FontLoadErr>>
  FontSystemImpl::load_from_memory(Vec<char> label, Vec<u8> encoded,
                                   u32 font_height, u32 face)
{
  Future fut = future<Result<FontId, FontLoadErr>>(allocator_).unwrap();
  scheduler->once(
    [fut = fut.alias(), encoded = std::move(encoded), label = std::move(label),
     this, face, font_height]() mutable {
      decode_(label, encoded, face)
        .match(
          [&, this](Dyn<Font *> & font) {
            trace("Rasterizing font: {} @{}px"_str, label, font_height);
            rasterize(*font, font_height)
              .match(
                [&, this](Void) {
                  scheduler->once(
                    [font = std::move(font), this,
                     fut  = std::move(fut)]() mutable {
                      trace("Rasterized font {}, num layers = {}"_str,
                            font->info().label,
                            font->info().cpu_atlas.v().num_layers);

                      FontId id = upload_(std::move(font));

                      fut.yield(Ok{id}).unwrap();
                    },
                    Ready{}, TaskSchedule{.target = TaskTarget::Main});
                },
                [&](Void) {
                  fut.yield(Err{FontLoadErr::OutOfMemory}).unwrap();
                });
          },
          [&](FontLoadErr err) { fut.yield(Err{err}).unwrap(); });
    },
    Ready{}, TaskSchedule{.target = TaskTarget::Worker});

  return fut;
}

Future<Result<FontId, FontLoadErr>>
  FontSystemImpl::load_from_path(Vec<char> label, Str path, u32 font_height,
                                 u32 face)
{
  Future file_load_fut = sys->file.load_file(path);

  Future fut = future<Result<FontId, FontLoadErr>>(allocator_).unwrap();

  scheduler->once(
    [file_load_fut = file_load_fut.alias(), fut = fut.alias(), this,
     label = std::move(label), font_height, face]() mutable {
      file_load_fut.get().match(
        [&](Vec<u8> & encoded) {
          Future mem_load_fut = load_from_memory(
            std::move(label), std::move(encoded), font_height, face);

          scheduler->once(
            [fut = fut.alias(), mem_load_fut = mem_load_fut.alias()]() {
              fut.yield(mem_load_fut.get()).unwrap();
            },
            AwaitFutures{mem_load_fut.alias()},
            TaskSchedule{.target = TaskTarget::Worker});
        },
        [&](IoErr err) {
          fut
            .yield(Err{err == IoErr::InvalidFileOrDir ?
                         FontLoadErr::InvalidPath :
                         FontLoadErr::IoErr})
            .unwrap();
        });
    },
    AwaitFutures{file_load_fut.alias()});

  return fut;
}

FontInfo FontSystemImpl::get(FontId id)
{
  CHECK(fonts_.is_valid_id((usize) id), "");
  return fonts_[(usize) id].v0->info();
}

Option<FontInfo> FontSystemImpl::get(Str label)
{
  for (auto & font : fonts_.dense.v0)
  {
    if (mem::eq(label, font->info().label))
    {
      return font->info();
    }
  }

  return none;
}

void FontSystemImpl::unload(FontId id)
{
  Dyn<Font *> & f    = fonts_[(usize) id].v0;
  FontImpl &    font = (FontImpl &) *f;
  sys->image.unload(font.gpu_atlas.v().image);
  font.gpu_atlas = none;

  fonts_.erase((usize) id);
}

/// layout is output in AU_UNIT units. so it is independent of the actual
/// font-height and can be cached as necessary. text must have been sanitized
/// with invalid codepoints replaced before calling this.
/// @param script OpenType (ISO15924) Script
/// Tag. See: https://unicode.org/reports/tr24/#Relation_To_ISO15924
static inline Tuple<Span<hb_glyph_info_t const>,
                    Span<hb_glyph_position_t const>>
  shape(hb_font_t * font, hb_buffer_t * buffer, Str32 line, Slice codepoints,
        hb_script_t script, hb_direction_t direction, hb_language_t language,
        bool use_kerning, bool use_ligatures)
{
  // tags are opentype feature tags
  hb_feature_t const shaping_features[] = {
    // kerning operations
    {.tag   = HB_TAG('k', 'e', 'r', 'n'),
     .value = use_kerning,
     .start = HB_FEATURE_GLOBAL_START,
     .end   = HB_FEATURE_GLOBAL_END},
    // standard ligature glyph substitution
    {.tag   = HB_TAG('l', 'i', 'g', 'a'),
     .value = use_ligatures,
     .start = HB_FEATURE_GLOBAL_START,
     .end   = HB_FEATURE_GLOBAL_END},
    // contextual ligature glyph substitution
    {.tag   = HB_TAG('c', 'l', 'i', 'g'),
     .value = use_ligatures,
     .start = HB_FEATURE_GLOBAL_START,
     .end   = HB_FEATURE_GLOBAL_END}
  };

  hb_buffer_clear_contents(buffer);
  // invalid character replacement
  hb_buffer_set_replacement_codepoint(buffer,
                                      HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT);
  hb_buffer_set_script(buffer, script);
  hb_buffer_set_direction(buffer, direction);
  // OpenType BCP-47 language tag specifying locale-sensitive shaping operations
  // as defined in the font
  hb_buffer_set_language(buffer, language);
  hb_buffer_add_codepoints(buffer, (u32 const *) line.data(), (i32) line.size(),
                           (u32) codepoints.offset, (i32) codepoints.span);
  hb_shape(font, buffer, shaping_features, (u32) size(shaping_features));

  u32                         num_pos;
  hb_glyph_position_t const * glyph_pos =
    hb_buffer_get_glyph_positions(buffer, &num_pos);
  CHECK(!(glyph_pos == nullptr && num_pos > 0), "");

  u32                     num_info;
  hb_glyph_info_t const * glyph_info =
    hb_buffer_get_glyph_infos(buffer, &num_info);
  CHECK(!(glyph_info == nullptr && num_info > 0), "");

  CHECK(num_pos == num_info, "");

  return Tuple{
    Span{glyph_info, num_info},
    Span{glyph_pos,  num_pos }
  };
}

/// @brief only needs to be called if it contains multiple paragraphs
static inline void segment_paragraphs(Str32 text, Span<TextSegment> segments)
{
  auto const text_size = text.size();
  for (usize i = 0; i < text_size;)
  {
    while (i < text_size)
    {
      if (text[i] == '\r' && ((i + 1) < text_size) && text[i + 1] == '\n')
      {
        segments[i].linebreak_begin = true;
        if ((i + 2) < text_size)
        {
          segments[i + 2].paragraph_begin = true;
        }
        i += 2;
        break;
      }
      else if (text[i] == '\n' || text[i] == '\r')
      {
        segments[i].linebreak_begin = true;
        if ((i + 1) < text_size)
        {
          segments[i + 1].paragraph_begin = true;
        }
        i += 1;
        break;
      }
      i++;
    }
  }
}

/// @brief only needs to be called if it contains multiple scripts
/// outputs iso15924 or OpenType tags
static inline void segment_scripts(Str32 text, Span<TextSegment> segments)
{
  SBCodepointSequence codepoints{.stringEncoding = SBStringEncodingUTF32,
                                 .stringBuffer   = (void *) text.data(),
                                 .stringLength   = text.size()};

  SBScriptLocatorRef locator = SBScriptLocatorCreate();
  CHECK(locator != nullptr, "");
  SBScriptLocatorLoadCodepoints(locator, &codepoints);

  SBScriptAgent const * agent = SBScriptLocatorGetAgent(locator);
  CHECK(agent != nullptr, "");

  while (SBScriptLocatorMoveNext(locator) == SBTrue)
  {
    for (SBUInteger i = agent->offset; i < (agent->offset + agent->length); i++)
    {
      segments[i].script = TextScript{agent->script};
    }
  }

  SBScriptLocatorRelease(locator);
}

/// @brief only needs to be called if it is a bidirectional text
static inline void segment_levels(Str32 text, SBAlgorithmRef algorithm,
                                  TextDirection     base,
                                  Span<TextSegment> segments)
{
  // The embedding level is an integer value. LTR text segments have even
  // embedding levels (e.g., 0, 2, 4), and RTL text segments have odd embedding
  // levels (e.g., 1, 3, 5).
  auto const text_size = text.size();
  for (usize i = 0; i < text_size;)
  {
    auto first = i;
    while (i < text_size && !segments[i].linebreak_begin)
    {
      i++;
    }

    auto const length = i - first;

    if (length > 0)
    {
      SBParagraphRef paragraph = SBAlgorithmCreateParagraph(
        algorithm, first, length,
        (base == TextDirection::LeftToRight) ? SBLevelDefaultLTR :
                                               SBLevelDefaultRTL);
      CHECK(paragraph != nullptr, "");

      CHECK(SBParagraphGetLength(paragraph) == length, "");
      SBLevel const   base_level = SBParagraphGetBaseLevel(paragraph);
      SBLevel const * levels     = SBParagraphGetLevelsPtr(paragraph);
      CHECK(levels != nullptr, "");
      for (usize i = 0; i < length; i++)
      {
        segments[first + i].base_level = base_level;
        segments[first + i].level      = levels[i];
      }
      SBParagraphRelease(paragraph);
    }

    i++;
    while (i < text_size && !segments[i].paragraph_begin)
    {
      i++;
    }
  }
}

/// @brief only needs to be called if line breaking is required.
static inline void segment_wrap_points(Str32 text, Span<TextSegment> segments)
{
  for (auto [cp, segment] : zip(text, segments))
  {
    switch (cp)
    {
      case ' ':
        segment.whitespace = true;
        break;
      case '\t':
        segment.tab = true;
        break;
    }
  }

  for (auto [i, segment] : enumerate(segments))
  {
    segment.wrappable =
      (i == (text.size() - 1)) || segments[i + 1].is_wrap_point();
  }
}

static inline void insert_run(TextLayout & l, FontStyle const & s,
                              Slice codepoints, usize base_cluster,
                              FontMetrics const &             font_metrics,
                              TextSegment const &             base_segment,
                              Span<hb_glyph_info_t const>     infos,
                              Span<hb_glyph_position_t const> positions)
{
  auto const num_glyphs  = infos.size();
  auto const first_glyph = l.glyphs.size();

  l.glyphs.extend_uninit(num_glyphs).unwrap();

  i32 advance = 0;

  for (usize i = 0; i < num_glyphs; i++)
  {
    hb_glyph_info_t const &     info = infos[i];
    hb_glyph_position_t const & pos  = positions[i];
    GlyphShape                  shape{
                       .glyph   = info.codepoint,
                       .cluster = base_cluster + info.cluster,
                       .advance = pos.x_advance,
                       .offset  = {pos.x_offset, -pos.y_offset}
    };

    l.glyphs[first_glyph + i] = shape;
    advance += pos.x_advance;
  }

  TextRunType type = TextRunType::Char;

  if (base_segment.whitespace)
  {
    type = TextRunType::WhiteSpace;
  }
  else if (base_segment.tab)
  {
    type = TextRunType::Tab;
  }

  l.runs
    .push(TextRun{
      .codepoints  = codepoints,
      .style       = base_segment.style,
      .font_height = s.height,
      .line_height = max(s.line_height, 1.0F),
      .glyphs{first_glyph, num_glyphs},
      .metrics{.ascent  = font_metrics.ascent,
              .descent = font_metrics.descent,
              .advance = advance},
      .base_level = base_segment.base_level,
      .level      = base_segment.level,
      .wrappable  = base_segment.wrappable,
      .type       = type
  })
    .unwrap();
}

/// See Unicode Embedding Level Reordering:
/// https://www.unicode.org/reports/tr9/#L1 -
/// https://www.unicode.org/reports/tr9/#L2
static inline void reorder_line(Span<TextRun> runs)
{
  u8 max_level = 0;

  for (TextRun const & r : runs)
  {
    max_level = max(r.level, max_level);
  }

  u8 level = max_level;

  while (level > 0)
  {
    // re-order consecutive runs with embedding levels greater or equal than
    // the current embedding level
    for (usize i = 0; i < runs.size();)
    {
      while (i < runs.size() && runs[i].level < level)
      {
        i++;
      }

      usize const first = i;

      while (i < runs.size() && runs[i].level >= level)
      {
        i++;
      }

      reverse(runs.slice(first, i - first));
    }

    level--;
  }
}

/// see:
/// https://stackoverflow.com/questions/62374506/how-do-i-align-glyphs-along-the-baseline-with-freetype
///
void FontSystemImpl::layout_text(TextBlock const & block, f32 max_width,
                                 TextLayout & layout)
{
  segments_.clear();
  layout.clear();

  auto const text_size = block.text.size();
  CHECK(block.runs.size() == block.fonts.size(), "");
  CHECK(!block.runs.is_empty(), "No run styling provided for text");
  CHECK(block.runs.last() >= text_size,
        "Text runs need to span the entire text");

  segments_.clear();
  segments_.resize(text_size).unwrap();

  {
    usize run_start = 0;
    for (usize irun = 0; irun < block.runs.size(); irun++)
    {
      auto const run_end = min(block.runs[irun], text_size);
      for (usize i = run_start; i < run_end; i++)
      {
        segments_[i].style = irun;
      }
      run_start = run_end;
    }
  }

  segment_paragraphs(block.text, segments_);
  segment_scripts(block.text, segments_);
  segment_wrap_points(block.text, segments_);

  if (!block.text.is_empty())
  {
    SBCodepointSequence codepoints{.stringEncoding = SBStringEncodingUTF32,
                                   .stringBuffer   = (void *) block.text.data(),
                                   .stringLength   = text_size};
    SBAlgorithmRef      algorithm = SBAlgorithmCreate(&codepoints);
    CHECK(algorithm != nullptr, "");
    defer algorithm_{[&] { SBAlgorithmRelease(algorithm); }};
    segment_levels(block.text, algorithm, block.direction, segments_);
  }

  {
    hb_language_t language =
      block.language.is_empty() ?
        hb_language_get_default() :
        hb_language_from_string(block.language.data(),
                                (i32) block.language.size());

    // - the block never has empty paragraphs
    // - paragraphs never have empty lines; they may have empty codepoints or break codepoints
    // - lines never have empty runs; they may have empty codepoints
    // - runs may have empty codepoints

    usize p = 0;

    do
    {
      auto const paragraph_begin = p;

      while (p < text_size && !segments_[p].linebreak_begin)
      {
        p++;
      }

      auto const paragraph_end        = p;
      auto const paragraph_runs_begin = layout.runs.size();
      auto       i                    = paragraph_begin;

      do
      {
        auto const        run_begin = i;
        TextSegment const base_segment =
          (run_begin < paragraph_end) ? segments_[run_begin] :
                                        TextSegment{.style  = 0,
                                                    .script = TextScript::None,
                                                    .linebreak_begin = false,
                                                    .paragraph_begin = true,
                                                    .whitespace      = false,
                                                    .tab             = false,
                                                    .wrappable       = false,
                                                    .base_level      = 0,
                                                    .level           = 0};

        if (i < paragraph_end)
        {
          i++;
        }

        while (i < paragraph_end && base_segment.style == segments_[i].style &&
               base_segment.script == segments_[i].script &&
               base_segment.level == segments_[i].level &&
               !segments_[i].is_wrap_point())
        {
          i++;
        }

        FontStyle const & s = block.fonts[base_segment.style];
        FontImpl const &  f = (FontImpl const &) *fonts_[(usize) s.font].v0;

        auto const paragraph =
          block.text.slice(Slice::range(paragraph_begin, paragraph_end));
        Slice const paragraph_subset{run_begin - paragraph_begin,
                                     i - run_begin};

        auto [infos, positions] =
          shape(f.hb_font, hb_buffer_, paragraph, paragraph_subset,
                hb_script_from_iso15924_tag(
                  SBScriptGetOpenTypeTag(SBScript{(u8) base_segment.script})),
                ((base_segment.level & 0x1) == 0) ? HB_DIRECTION_LTR :
                                                    HB_DIRECTION_RTL,
                language, block.use_kerning, block.use_ligatures);

        Slice const codepoints = Slice::range(run_begin, i);

        insert_run(layout, s, codepoints, paragraph_begin, f.metrics,
                   base_segment, infos, positions);

      } while (i < paragraph_end);

      auto const paragraph_runs_end = layout.runs.size();

      // line-break or end of text
      auto const break_begin = p;

      if (p < text_size)
      {
        p++;
      }

      while (p < text_size && !segments_[p].paragraph_begin)
      {
        p++;
      }

      auto const break_end = p;

      layout.paragraphs
        .push(Paragraph{
          .runs       = Slice::range(paragraph_runs_begin, paragraph_runs_end),
          .codepoints = Slice::range(paragraph_begin, paragraph_end),
          .break_codepoints = Slice::range(break_begin, break_end)})
        .unwrap();

    } while (p < text_size);
  }

  Vec2 extent{};

  usize caret_iter = 0;

  for (auto & paragraph : layout.paragraphs)
  {
    auto const lines_begin = layout.lines.size();

    for (usize i = paragraph.runs.begin(); i < paragraph.runs.end();)
    {
      auto const   first             = i++;
      auto const & first_run         = layout.runs[first];
      u8 const     base_level        = first_run.base_level;
      f32 const    font_height       = block.font_scale * first_run.font_height;
      auto const   first_run_metrics = first_run.metrics.resolve(font_height);
      auto const & style             = block.fonts[first_run.style];
      auto const   advance =
        first_run_metrics.advance +
        (first_run.is_spacing() ? 0 : (block.font_scale * style.word_spacing));

      f32 width   = advance;
      f32 ascent  = first_run_metrics.ascent;
      f32 descent = first_run_metrics.descent;
      f32 line_height =
        max(font_height * first_run.line_height, first_run_metrics.height());

      while (i < paragraph.runs.end())
      {
        auto const & r = layout.runs[i];
        auto const   f = block.font_scale * r.font_height;
        auto const   m = r.metrics.resolve(f);
        auto const   l = max(f * r.line_height, m.height());
        auto const & s = block.fonts[r.style];
        auto const   a =
          m.advance +
          (r.is_spacing() ? 0 : (block.font_scale * s.word_spacing));

        if (block.wrap && r.wrappable && (width + a) > max_width)
        {
          break;
        }

        width += a;
        ascent      = max(ascent, m.ascent);
        descent     = max(descent, m.descent);
        line_height = max(line_height, l);
        i++;
      }

      auto const & last_run = layout.runs[i - 1];
      auto const   codepoints =
        Slice::range(first_run.codepoints.offset, last_run.codepoints.end());

      Slice const runs = Slice::range(first, i);

      auto const num_carets = codepoints.span + 1;
      auto const carets     = Slice{caret_iter, num_carets};

      Line line{
        .codepoints = codepoints,
        .carets     = carets,
        .runs       = runs,
        .metrics{.width   = width,
                 .height  = line_height,
                 .ascent  = ascent,
                 .descent = descent,
                 .level   = base_level}
      };

      layout.lines.push(line).unwrap();

      reorder_line(layout.runs.view().slice(first, i - first));

      extent.x = max(extent.x, width);
      extent.y += line_height;
      caret_iter += num_carets;
    }

    auto const lines_end = layout.lines.size();
    paragraph.lines      = Slice::range(lines_begin, lines_end);
  }

  layout.max_width      = max_width;
  layout.num_carets     = max(caret_iter, 1ULL);
  layout.num_codepoints = text_size;
  layout.extent         = extent;
  layout.laid_out       = true;
}

}    // namespace ash
