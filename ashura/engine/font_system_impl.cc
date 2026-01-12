/// SPDX-License-Identifier: MIT
#include "ashura/engine/font_system_impl.h"
#include "ashura/engine/file_system.h"
#include "ashura/engine/rect_pack.h"
#include "ashura/engine/systems.h"
#include "ashura/std/range.h"
#include "ashura/std/trace.h"
#include "ashura/std/vec.h"

extern "C"
{
#include "SheenBidi/SBAlgorithm.h"
#include "SheenBidi/SBParagraph.h"
#include "SheenBidi/SBScriptLocator.h"
#include "hb.h"
}

namespace ash
{

Dyn<FontSys> IFontSys::create(Allocator allocator, FileSys file_sys,
                              ImageSys image_sys, Scheduler scheduler)
{
    return cast<FontSys>(dyn<FontSysImpl>(inplace, allocator, allocator,
                                          file_sys, image_sys, scheduler)
                           .unwrap());
}

FontImpl::~FontImpl()
{
    gpu_atlas.unwrap_none("GPU font atlas has not been unloaded"_str);
    hb_font_destroy(hb_font);
    hb_face_destroy(hb_face);
    hb_blob_destroy(hb_blob);
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_lib);
}

FontInfo FontImpl::info()
{
    FontInfo info{.label             = label,
                  .has_color         = has_color,
                  .postscript_name   = postscript_name,
                  .family_name       = family_name,
                  .style_name        = style_name,
                  .glyphs            = glyphs,
                  .replacement_glyph = replacement_glyph,
                  .space_glyph       = space_glyph,
                  .ellipsis_glyph    = ellipsis_glyph,
                  .metrics           = metrics};

    if (gpu_atlas.is_some())
    {
        info.gpu_atlas = gpu_atlas.v();
    }

    return info;
}

FontSysImpl::~FontSysImpl()
{
}

AwaitFuturesVec FontSysImpl::init()
{
    static constexpr u8 ROBOTO_FONT_DATA[] = {
#embed "assets/fonts/Roboto/Roboto-Regular.ttf"
    };
    static constexpr u32 FONT_RASTER_HEIGHT = 64U;
    static constexpr u32 FONT_FACE          = 0U;

    auto fut =
      load_from_memory("Default"_str, static_rc(span(ROBOTO_FONT_DATA)),
                       FONT_RASTER_HEIGHT, FONT_FACE);

    Vec<AnyFuture> await_futures{allocator_};
    await_futures.push(std::move(fut)).unwrap();
    return AwaitFuturesVec{std::move(await_futures)};
}

void FontSysImpl::shutdown()
{
    WriteGuard guard{rw_lock_};
    while (!fonts_.is_empty())
    {
        unload_(fonts_.to_id(0));
    }
}

Result<Dyn<Font>, SysErr> FontSysImpl::decode_(Str            label_ref,
                                               Span<u8 const> encoded, u32 face)
{
    tracing::ScopeTrace trace;
    Vec<char>           font_data{allocator_};
    if (!font_data.append(encoded.as_char()))
    {
        return Err{SysErr::OutOfMemory};
    }

    hb_blob_t * hb_blob =
      hb_blob_create(font_data.data(), font_data.size(),
                     HB_MEMORY_MODE_READONLY, nullptr, nullptr);

    if (hb_blob == nullptr)
    {
        return Err{SysErr::DecodeFailed};
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
        return Err{SysErr::FaceNotFound};
    }

    hb_face_t * hb_face = hb_face_create(hb_blob, face);

    if (hb_face == nullptr)
    {
        return Err{SysErr::DecodeFailed};
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
        return Err{SysErr::DecodeFailed};
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
        return Err{SysErr::DecodeFailed};
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
        return Err{SysErr::DecodeFailed};
    }

    if (FT_Error err = FT_Set_Char_Size(ft_face, AU_UNIT, AU_UNIT, 72, 72);
        err != 0)
    {
        return Err{SysErr::DecodeFailed};
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
        postscript_name.append(cstr(ft_postscript_name)).unwrap();
    }

    if (ft_face->family_name != nullptr)
    {
        family_name.append(cstr(ft_face->family_name)).unwrap();
    }

    if (ft_face->style_name != nullptr)
    {
        style_name.append(cstr(ft_face->style_name)).unwrap();
    }

    u32 num_glyphs        = (u32) ft_face->num_glyphs;
    // glyph 0 is selected if the replacement codepoint glyph is not found
    u32 replacement_glyph = FT_Get_Char_Index(ft_face, 0xFFFD);
    u32 ellipsis_glyph    = FT_Get_Char_Index(ft_face, 0x2026);
    u32 space_glyph       = FT_Get_Char_Index(ft_face, ' ');

    // expressed on a AU_UNIT scale
    i32 ascent  = ft_face->size->metrics.ascender;
    i32 descent = -ft_face->size->metrics.descender;
    i32 advance = ft_face->size->metrics.max_advance;

    Vec<GlyphMetrics> glyphs{allocator_};

    if (!glyphs.resize(num_glyphs))
    {
        return Err{SysErr::OutOfMemory};
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
          .bearing{(i32) s->metrics.horiBearingX,
                   (i32) -s->metrics.horiBearingY                        },
          .advance = (i32) s->metrics.horiAdvance,
          .extent{(i32) s->metrics.width,        (i32) s->metrics.height}
        };
    }

    Vec<char> label{allocator_};

    if (!label.append(label_ref))
    {
        return Err{SysErr::OutOfMemory};
    }

    Result font = dyn<FontImpl>(
      inplace, allocator_, std::move(label), std::move(font_data), has_color,
      std::move(postscript_name), std::move(family_name), std::move(style_name),
      hb_blob, hb_face, hb_font, ft_lib, ft_face, face, std::move(glyphs),
      replacement_glyph, ellipsis_glyph, space_glyph,
      FontMetrics{.ascent = ascent, .descent = descent, .advance = advance});

    if (!font)
    {
        return Err{SysErr::OutOfMemory};
    }

    hb_blob = nullptr;
    hb_face = nullptr;
    hb_font = nullptr;
    ft_lib  = nullptr;
    ft_face = nullptr;

    return Ok{cast<ash::Font>(std::move(font.v()))};
}

Result<CpuFontAtlas, SysErr> FontSysImpl::rasterize_(Font font_,
                                                     u32  font_height)
{
    tracing::ScopeTrace  trace;
    FontImpl &           font             = (FontImpl &) *font_;
    static constexpr u32 MIN_ATLAS_EXTENT = 512;
    static_assert(MIN_ATLAS_EXTENT > 0, "Font atlas extent must be non-zero");
    static_assert(MIN_ATLAS_EXTENT >= 128,
                  "Font atlas extent must be at least 128px");
    static_assert(MIN_ATLAS_EXTENT % 64 == 0,
                  "Font atlas extent should be a multiple of 64");
    static_assert(MIN_ATLAS_EXTENT <= 1'024,
                  "Font atlas extent too large for GPU platform");

    CpuFontAtlas atlas{.glyphs{allocator_}, .channels{allocator_}};

    auto num_glyphs = size32(font.glyphs);

    if (!atlas.glyphs.resize(num_glyphs))
    {
        return Err{SysErr::OutOfMemory};
    }

    if (FT_Error err =
          FT_Set_Pixel_Sizes(font.ft_face, font_height, font_height);
        err != 0)
    {
        return Err{SysErr::DecodeFailed};
    }

    static constexpr u16 GLYPH_PADDING = 1;

    u32x2 max_glyph_extent;

    for (auto [i, g] : enumerate<u32>(atlas.glyphs))
    {
        if (FT_Error err = FT_Load_Glyph(font.ft_face, i, FT_LOAD_DEFAULT);
            err != 0)
        {
            continue;
        }

        g.area.extent = u32x2{font.ft_face->glyph->bitmap.width,
                              font.ft_face->glyph->bitmap.rows};

        max_glyph_extent = max_glyph_extent.max(g.area.extent);
    }

    ASH_CHECK(max_glyph_extent.x() <= MIN_ATLAS_EXTENT, "");
    ASH_CHECK(max_glyph_extent.y() <= MIN_ATLAS_EXTENT, "");

    auto atlas_extent     = u32x2::splat(MIN_ATLAS_EXTENT);
    auto inv_atlas_extent = 1 / atlas_extent.to<f32>();

    u32 num_layers = 0;
    {
        Vec<rect_pack::Rect> rects{allocator_};

        if (!rects.resize_uninit(num_glyphs))
        {
            return Err{SysErr::OutOfMemory};
        }

        for (auto [i, gl, ag, rect] :
             enumerate<u32>(font.glyphs, atlas.glyphs, rects))
        {
            // added padding to avoid texture spilling due to accumulated
            // floating-point uv interpolation errors
            u32x2 padded_extent{};

            if (ag.area.extent.x() != 0 && ag.area.extent.y() != 0)
            {
                padded_extent = ag.area.extent + GLYPH_PADDING * 2;
            }

            rect = rect_pack::Rect{.id         = i,
                                   .extent     = padded_extent.to<i32>(),
                                   .pos        = {},
                                   .was_packed = false};
        }

        Vec<rect_pack::Node> nodes{allocator_};
        auto                 num_nodes = atlas_extent.x();
        nodes.resize_uninit(num_nodes).unwrap();

        Span<rect_pack::Rect> unpacked = rects;

        while (!unpacked.is_empty())
        {
            // tries to pack all the glyph rects into the provided extent
            rect_pack::Context ctx;
            rect_pack::init(ctx, atlas_extent.to<i32>(), nodes.data(),
                            (i32) num_nodes);
            rect_pack::pack_rects(ctx, unpacked.data(), (i32) size32(unpacked));

            auto [just_packed, still_unpacked] = partition(
              unpacked, [](rect_pack::Rect const & r) { return r.was_packed; });

            ASH_CHECK(!just_packed.is_empty(), "");

            for (rect_pack::Rect const & r : just_packed)
            {
                atlas.glyphs[r.id].layer = num_layers;
            }

            unpacked = still_unpacked;

            num_layers++;
        }

        for (auto [i, r] : enumerate(rects))
        {
            auto & g = atlas.glyphs[r.id];

            if (g.area.extent.x() == 0 | g.area.extent.y() == 0)
            {
                g.area.offset = u32x2{};
            }
            else
            {
                // adjust back to original position from the padded position
                g.area.offset = (r.pos + GLYPH_PADDING).to<u32>();
            }

            auto center =
              g.area.offset.to<f32>() + 0.5F * g.area.extent.to<f32>();
            auto extent = g.area.extent.to<f32>();

            g.uv.center = norm_to_axis(center * inv_atlas_extent);
            g.uv.extent = extent * inv_atlas_extent;
        }
    }

    auto atlas_layer_size = atlas_extent.product<u64>() * 4;
    u64  atlas_size       = atlas_layer_size * num_layers;

    if (!atlas.channels.resize(atlas_size))
    {
        return Err{SysErr::OutOfMemory};
    }

    ImageLayerSpan<u8, 4> atlas_span{
      .channels = atlas.channels, .extent = atlas_extent, .layers = num_layers};

    for (auto [i, ag] : enumerate<u32>(atlas.glyphs))
    {
        if (FT_Error err =
              FT_Load_Glyph(font.ft_face, i,
                            FT_LOAD_DEFAULT | FT_LOAD_COLOR | FT_LOAD_RENDER);
            err != 0)
        {
            continue;
        }

        FT_GlyphSlot slot = font.ft_face->glyph;

        /// we don't want to handle negative pitches
        ASH_CHECK(slot->bitmap.pitch >= 0, "");

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

                copy_alpha_image_to_BGRA(src,
                                         atlas_span.layer(ag.layer).slice(
                                           ag.area.offset, ag.area.extent),
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

                copy_image(src, atlas_span.layer(ag.layer).slice(
                                  ag.area.offset, ag.area.extent));

                ag.has_color = true;
            }
            break;
            default:
                ASH_CHECK(false, "Unrecognized pixel mode {}",
                          slot->bitmap.pixel_mode);
        }
    }

    atlas.font_height = font_height;
    atlas.extent      = atlas_extent;
    atlas.num_layers  = num_layers;

    return Ok{std::move(atlas)};
}

Future<Result<GpuFontAtlas, SysErr>>
  FontSysImpl::upload_atlas_to_gpu_(Str                      label_span,
                                    Rc<CpuFontAtlas const *> atlas)
{
    ASH_CHECK(atlas->num_layers > 0, "");
    ASH_CHECK(atlas->extent.x() > 0, "");
    ASH_CHECK(atlas->extent.y() > 0, "");

    constexpr gpu::Format   format = gpu::Format::B8G8R8A8_UNORM;
    Vec<gpu::ImageViewInfo> view_infos{allocator_};

    StrVec label{allocator_};
    label.append(label_span).unwrap();

    for (u32 i : range(atlas->num_layers))
    {
        view_infos
          .push(gpu::ImageViewInfo{
            .label        = label,
            .view_type    = gpu::ImageViewType::Type2D,
            .view_format  = format,
            .mapping      = {},
            .aspects      = gpu::ImageAspects::Color,
            .mip_levels   = {0, 1},
            .array_layers = {i, 1}
        })
          .unwrap();
    }

    auto load_fut = image_sys_->load_from_memory(
      label,
      gpu::ImageInfo{.label  = label,
                     .type   = gpu::ImageType::Type2D,
                     .format = format,
                     .usage  = gpu::ImageUsage::Sampled |
                              gpu::ImageUsage::TransferDst |
                              gpu::ImageUsage::TransferSrc,
                     .aspects      = gpu::ImageAspects::Color,
                     .extent       = atlas->extent.append(1),
                     .mip_levels   = 1,
                     .array_layers = atlas->num_layers,
                     .sample_count = gpu::SampleCount::C1},
      view_infos, transmute(atlas.alias(), atlas->channels.view().as_const()));

    Vec<AtlasGlyph> gpu_glyphs{allocator_};
    gpu_glyphs.append(atlas->glyphs).unwrap();

    return scheduler_
      ->then(
        allocator_, WorkerThread::Any,
        [allocator = allocator_, font_height = atlas->font_height,
         glyphs =
           std::move(gpu_glyphs)](Result<ImageInfo, SysErr> & err) mutable {
            using R = Result<GpuFontAtlas, SysErr>;
            return err.match(
              [&](ImageInfo const & image) -> R {
                  GpuFontAtlas gpu_atlas{.textures{allocator},
                                         .image       = image.id,
                                         .font_height = font_height,
                                         .extent      = image.info.extent.xy(),
                                         .glyphs      = std::move(glyphs)};
                  gpu_atlas.textures.append(image.textures).unwrap();
                  gpu_atlas.image = image.id;
                  return Ok{std::move(gpu_atlas)};
              },
              [](SysErr err) -> R { return Err{err}; });
        },
        std::move(load_fut))
      .unwrap();
}

Future<Result<FontId, SysErr>> FontSysImpl::load_from_memory(Str     label_span,
                                                             RcBlob8 encoded,
                                                             u32 font_height,
                                                             u32 face)
{
    StrVec label{allocator_};
    label.append(label_span).unwrap();

    // [ ] using trace span id in logging

    auto decode_fut =
      scheduler_
        ->run(allocator_, WorkerThread::Any,
              [encoded = std::move(encoded), label = std::move(label), this,
               face]() mutable { return decode_(label, encoded, face); })
        .unwrap();

    auto raster_fut =
      scheduler_
        ->then(
          allocator_, WorkerThread::Any,
          [encoded = std::move(encoded), label = std::move(label), this,
           font_height](Result<Dyn<Font>, SysErr> & r) mutable {
              using R = Result<CpuFontAtlas, SysErr>;
              return r.match(
                [&](Dyn<Font> & f) -> R {
                    trace("Rasterizing font: {} @{}px"_str, label, font_height);
                    return rasterize_(f, font_height);
                },
                [](SysErr err) -> R { return Err{err}; });
          },
          decode_fut.alias())
        .unwrap();

    auto upload_fut =
      scheduler_
        ->then(
          allocator_, WorkerThread::Any,
          [this,
           decode_fut = decode_fut.alias()](Result<CpuFontAtlas, SysErr> & r) {
              using R = Result<Future<Result<GpuFontAtlas, SysErr>>, SysErr>;
              return r.match(
                [&](CpuFontAtlas & cpu_atlas) -> R {
                    auto rc_atlas =
                      rc<CpuFontAtlas>(allocator_, std::move(cpu_atlas))
                        .unwrap();
                    auto & font       = *decode_fut.get().v();
                    auto   label      = font.info().label.view();
                    auto   upload_fut = upload_atlas_to_gpu_(
                      label, transmute(rc_atlas.alias(),
                                         static_cast<CpuFontAtlas const *>(
                                         rc_atlas.get())));
                    return Ok{std::move(upload_fut)};
                },
                [](SysErr err) -> R { return Err{err}; });
          },
          std::move(raster_fut))
        .unwrap();

    auto flattened_upload_fut =
      scheduler_->flatten(allocator_, WorkerThread::Any, std::move(upload_fut))
        .unwrap();

    auto ret_fut = scheduler_
                     ->then(
                       allocator_, WorkerThread::Any,
                       [decode_fut = decode_fut.alias(),
                        this](Result<GpuFontAtlas, SysErr> & r) {
                           using R = Result<FontId, SysErr>;

                           return r.match(
                             [&](GpuFontAtlas & gpu_atlas) -> R {
                                 return Ok{add_font_(decode_fut.get().unwrap(),
                                                     std::move(gpu_atlas))};
                             },
                             [](SysErr err) -> R { return Err{err}; });
                       },
                       std::move(flattened_upload_fut))
                     .unwrap();

    return ret_fut;
}

Future<Result<FontId, SysErr>> FontSysImpl::load_from_path(Str label_span,
                                                           Str path,
                                                           u32 font_height,
                                                           u32 face)
{
    Future file_load_fut = file_sys_->load_file(allocator_, path);
    StrVec label{allocator_};
    label.append(label_span).unwrap();

    auto load_fut =
      scheduler_
        ->then(
          allocator_, WorkerThread::Any,
          [label = std::move(label), this, font_height,
           face](Result<Vec<u8>, SysErr> & r) {
              using R = Result<Future<Result<FontId, SysErr>>, SysErr>;
              return r.match(
                [&](Vec<u8> & data) -> R {
                    auto blob_span = data.view().as_const();
                    auto blob =
                      rc<Vec<u8>>(allocator_, std::move(data)).unwrap();
                    auto blob8 = transmute(std::move(blob), blob_span);
                    return Ok{load_from_memory(label, std::move(blob8),
                                               font_height, face)};
                },
                [&](SysErr err) -> R { return Err{err}; });
          },
          file_load_fut.alias())
        .unwrap();

    return scheduler_
      ->flatten(allocator_, WorkerThread::Any, std::move(load_fut))
      .unwrap();
}

FontInfo FontSysImpl::get(FontId id)
{
    ReadGuard guard{rw_lock_};
    ASH_CHECK(fonts_.is_valid_id(id), "");
    return fonts_[id].v0->info();
}

FontImpl & FontSysImpl::get_impl(FontId id)
{
    ReadGuard guard{rw_lock_};
    ASH_CHECK(fonts_.is_valid_id(id), "");
    return *(FontImpl *) fonts_[id].v0.get();
}

Option<FontInfo> FontSysImpl::get(Str label)
{
    ReadGuard guard{rw_lock_};
    for (auto & font : fonts_.dense.v0)
    {
        if (mem::eq(label, font->info().label))
        {
            return font->info();
        }
    }

    return none;
}

ImageId FontSysImpl::unload_(FontId id)
{
    Dyn<Font> & f        = fonts_[id].v0;
    FontImpl &  font     = (FontImpl &) *f;
    auto        image_id = font.gpu_atlas.v().image;
    font.gpu_atlas       = none;
    fonts_.erase(id);
    return image_id;
}

void FontSysImpl::unload(FontId id)
{
    ImageId image_id = ImageId::None;
    {
        WriteGuard guard{rw_lock_};
        image_id = unload_(id);
    }

    // deadlock-avoiding: unload image outside of the font system lock
    image_sys_->unload(image_id);
}

FontId FontSysImpl::add_font_(Dyn<Font> font, GpuFontAtlas gpu_atlas)
{
    FontImpl & font_impl = (FontImpl &) *font;
    font_impl.gpu_atlas  = std::move(gpu_atlas);

    {
        WriteGuard guard{rw_lock_};
        return fonts_.push(std::move(font)).unwrap();
    }
}

/// layout is output in AU_UNIT units. so it is independent of the actual
/// font-height and can be cached as necessary. text must have been sanitized
/// with invalid codepoints replaced before calling this.
/// @param script OpenType (ISO15924) Script
/// Tag. See: https://unicode.org/reports/tr24/#Relation_To_ISO15924
static inline Tuple<Span<hb_glyph_info_t const>,
                    Span<hb_glyph_position_t const>>
  shape_run(hb_font_t * font, hb_buffer_t * buffer, Str32 line,
            Slice codepoints, hb_script_t script, hb_direction_t direction,
            hb_language_t language, bool use_kerning, bool use_ligatures)
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
    hb_buffer_set_replacement_codepoint(
      buffer, HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT);
    hb_buffer_set_script(buffer, script);
    hb_buffer_set_direction(buffer, direction);
    // OpenType BCP-47 language tag specifying locale-sensitive shaping operations
    // as defined in the font
    hb_buffer_set_language(buffer, language);
    hb_buffer_add_codepoints(buffer, (u32 const *) line.data(),
                             (i32) line.size(), (u32) codepoints.offset,
                             (i32) codepoints.span);
    hb_shape(font, buffer, shaping_features, (u32) size(shaping_features));

    u32                         num_pos;
    hb_glyph_position_t const * glyph_pos =
      hb_buffer_get_glyph_positions(buffer, &num_pos);
    ASH_CHECK(!(glyph_pos == nullptr && num_pos > 0), "");

    u32                     num_info;
    hb_glyph_info_t const * glyph_info =
      hb_buffer_get_glyph_infos(buffer, &num_info);
    ASH_CHECK(!(glyph_info == nullptr && num_info > 0), "");

    ASH_CHECK(num_pos == num_info, "");

    return Tuple{
      Span{glyph_info, num_info},
      Span{glyph_pos,  num_pos }
    };
}

/// @brief Only needs to be called if it contains multiple scripts
/// outputs iso15924 or OpenType tags
static inline void paragraph_script_runs(SBAllocatorRef allocator, Str32 text,
                                         auto & run_indices, auto & scripts)
{
    if (text.is_empty())
    {
        return;
    }

    SBCodepointSequence codepoints{.stringEncoding = SBStringEncodingUTF32,
                                   .stringBuffer   = (void *) text.data(),
                                   .stringLength   = text.size()};

    SBScriptLocatorRef locator = SBScriptLocatorCreate(allocator);
    ASH_CHECK(locator != nullptr, "");
    SBScriptLocatorLoadCodepoints(locator, &codepoints);

    SBScriptAgent const * agent = SBScriptLocatorGetAgent(locator);
    ASH_CHECK(agent != nullptr, "");

    while (SBScriptLocatorMoveNext(locator) == SBTrue)
    {
        auto script = TextScript{agent->script};
        if (run_indices.is_empty())
        {
            run_indices.push(0ULL).unwrap();
            run_indices.push(agent->length).unwrap();
        }
        else
        {
            run_indices.push(run_indices.last() + agent->length).unwrap();
        }

        scripts.push(script).unwrap();
    }

    SBScriptLocatorRelease(allocator, locator);
}

/// @brief Only needs to be called if it is a bidirectional text
/// @returns the base embedding level
static inline u8 paragraph_levels(SBAllocatorRef allocator, Str32 text,
                                  SBAlgorithmRef algorithm, TextDirection base,
                                  auto & levels)
{
    // The embedding level is an integer value. LTR text segments have even
    // embedding levels (e.g., 0, 2, 4), and RTL text segments have odd embedding
    // levels (e.g., 1, 3, 5).
    if (text.is_empty())
    {
        return 0;
    }

    auto text_size = text.size();
    auto paragraph = SBAlgorithmCreateParagraph(
      allocator, algorithm, 0, text_size,
      (base == TextDirection::LeftToRight) ? SBLevelDefaultLTR :
                                             SBLevelDefaultRTL);
    ASH_CHECK(paragraph != nullptr, "");
    defer paragraph_{[&] { SBParagraphRelease(allocator, paragraph); }};

    ASH_CHECK(SBParagraphGetLength(paragraph) == text_size, "");
    SBLevel const   base_level = SBParagraphGetBaseLevel(paragraph);
    SBLevel const * p_levels   = SBParagraphGetLevelsPtr(paragraph);
    ASH_CHECK(p_levels != nullptr, "");
    levels.append(Span{p_levels, text_size}).unwrap();
    return base_level;
}

constexpr bool is_wrap_char(c32 c)
{
    return c == ' ' || c == '\t';
}

constexpr TextRunType classify_run_type(c32 c)
{
    switch (c)
    {
        case ' ':
            return TextRunType::WhiteSpace;
        case '\t':
            return TextRunType::Tab;
        default:
            return TextRunType::Char;
    }
}

struct RunProps
{
    TextRunType type       = TextRunType::WhiteSpace;
    u32         style      = 0;
    TextScript  script     = TextScript::Unknown;
    u8          base_level = 0;
    u8          level      = 0;
    u32         wrap_level = 0;
};

static inline void insert_run(TextLayout & l, FontStyle const & s,
                              Slice codepoints, usize base_cluster,
                              RunProps const &                props,
                              FontMetrics const &             font_metrics,
                              Span<hb_glyph_info_t const>     infos,
                              Span<hb_glyph_position_t const> positions)
{
    auto num_glyphs  = infos.size();
    auto first_glyph = l.glyphs.size();

    l.glyphs.extend_uninit(num_glyphs).unwrap();

    i32 advance = 0;

    for (auto i : range(num_glyphs))
    {
        hb_glyph_info_t const &     info  = infos[i];
        hb_glyph_position_t const & pos   = positions[i];
        auto                        shape = GlyphShape{
                                 .glyph   = info.codepoint,
                                 .cluster = base_cluster + info.cluster,
                                 .advance = pos.x_advance,
                                 .offset  = {pos.x_offset, -pos.y_offset}
        };

        l.glyphs[first_glyph + i] = shape;
        advance += pos.x_advance;
    }

    l.runs
      .push(TextRun{
        .codepoints  = codepoints,
        .style       = props.style,
        .font_height = s.height,
        .line_height = max(s.line_height, 1.0F),
        .glyphs{first_glyph, num_glyphs},
        .metrics{.ascent  = font_metrics.ascent,
                .descent = font_metrics.descent,
                .advance = advance},
        .base_level = props.base_level,
        .level      = props.level,
        .wrap_level = props.wrap_level,
        .type       = props.type
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
        for (auto i = 0uz; i < runs.size();)
        {
            while (i < runs.size() && runs[i].level < level)
            {
                i++;
            }

            auto first = i;

            while (i < runs.size() && runs[i].level >= level)
            {
                i++;
            }

            reverse(runs.slice(first, i - first));
        }

        level--;
    }
}

struct TextLayoutBufferImpl
{
    hb_buffer_t * hb_buffer_;    // harfbuzz shaping buffer

    explicit constexpr TextLayoutBufferImpl(hb_buffer_t * hb_buffer) :
      hb_buffer_{hb_buffer}

    {
    }

    constexpr TextLayoutBufferImpl(TextLayoutBufferImpl const &) = delete;
    constexpr TextLayoutBufferImpl &
      operator=(TextLayoutBufferImpl const &)               = delete;
    constexpr TextLayoutBufferImpl(TextLayoutBufferImpl &&) = delete;
    constexpr TextLayoutBufferImpl &
      operator=(TextLayoutBufferImpl &&) = delete;

    void clear()
    {
        hb_buffer_clear_contents(hb_buffer_);
    }

    ~TextLayoutBufferImpl()
    {
        hb_buffer_destroy(hb_buffer_);
    }
};

struct LayoutBufferHook
{
    LayoutBufferHook *   next = nullptr;
    LayoutBufferHook *   prev = nullptr;
    TextLayoutBufferImpl buffer;

    template <typename... Args>
    LayoutBufferHook(Args &&... args) : buffer{std::forward<Args>(args)...}
    {
        push(this);
    }

    LayoutBufferHook(LayoutBufferHook const &)             = delete;
    LayoutBufferHook & operator=(LayoutBufferHook const &) = delete;
    LayoutBufferHook(LayoutBufferHook &&)                  = delete;
    LayoutBufferHook & operator=(LayoutBufferHook &&)      = delete;

    ~LayoutBufferHook()
    {
        pop(this);
    }

    static void push(LayoutBufferHook *);
    static void pop(LayoutBufferHook *);
};

struct LayoutBuffersSink
{
    ISpinLock              lock;
    List<LayoutBufferHook> buffers{};
};

static LayoutBuffersSink layout_buffers_sink;

void LayoutBufferHook::push(LayoutBufferHook * hook)
{
    LockGuard guard{layout_buffers_sink.lock};
    layout_buffers_sink.buffers.push_back(hook);
}

void LayoutBufferHook::pop(LayoutBufferHook * hook)
{
    LockGuard guard{layout_buffers_sink.lock};
    layout_buffers_sink.buffers.pop_at(hook);
}

TextLayoutBufferImpl & FontSysImpl::get_thread_layout_buffer()
{
    static thread_local LayoutBufferHook thread_layout_buffer{[] {
        hb_buffer_t * hb_buffer = hb_buffer_create();
        ASH_CHECK(hb_buffer != nullptr &&
                    hb_buffer_allocation_successful(hb_buffer),
                  "");
        return hb_buffer;
    }()};

    return thread_layout_buffer.buffer;
}

void layout_paragraph(Paragraph & paragraph, f32 max_width,
                      TextBlock const & block, TextLayout & layout,
                      usize & caret_iter, f32x2 & extent)
{
    auto  lines_begin = layout.lines.size();
    usize irun        = paragraph.runs.begin();

    while (irun < paragraph.runs.end())
    {
        auto   first             = irun++;
        auto & first_run         = layout.runs[first];
        u8     base_level        = first_run.base_level;
        f32    font_height       = block.font_scale * first_run.font_height;
        auto   first_run_metrics = first_run.metrics.resolve(font_height);
        auto & style             = block.fonts[first_run.style];
        auto   advance =
          first_run_metrics.advance +
          (first_run.is_spacing() ? 0 :
                                    (block.font_scale * style.word_spacing));

        f32 width   = advance;
        f32 ascent  = first_run_metrics.ascent;
        f32 descent = first_run_metrics.descent;
        f32 line_height =
          max(font_height * first_run.line_height, first_run_metrics.height());

        while (irun < paragraph.runs.end())
        {
            auto & r = layout.runs[irun];
            auto   f = block.font_scale * r.font_height;
            auto   m = r.metrics.resolve(f);
            auto   l = max(f * r.line_height, m.height());
            auto & s = block.fonts[r.style];
            auto   a = m.advance +
                     (r.is_spacing() ? 0 : (block.font_scale * s.word_spacing));

            if (block.wrap && r.wrap_level != first_run.wrap_level &&
                (width + a) > max_width)
            {
                break;
            }

            width += a;
            ascent      = max(ascent, m.ascent);
            descent     = max(descent, m.descent);
            line_height = max(line_height, l);
            irun++;
        }

        auto & last_run   = layout.runs[irun - 1];
        auto   codepoints = Slice::offsets(first_run.codepoints.offset,
                                           last_run.codepoints.end());
        auto   runs       = Slice::offsets(first, irun);
        auto   num_carets = codepoints.span + 1;
        auto   carets     = Slice{caret_iter, num_carets};
        auto   line       = Line{
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

        reorder_line(layout.runs.view().slice(first, irun - first));

        extent.x() = max(extent.x(), width);
        extent.y() += line_height;
        caret_iter += num_carets;
    }

    auto lines_end  = layout.lines.size();
    paragraph.lines = Slice::offsets(lines_begin, lines_end);
}

/// https://stackoverflow.com/questions/62374506/how-do-i-align-glyphs-along-the-baseline-with-freetype
///
void FontSysImpl::layout_text(TextBlock const & block, f32 max_width,
                              TextLayout & layout, Allocator scratch)
{
    tracing::ScopeTrace trace;

    auto       text      = block.text;
    auto const text_size = block.text.size();
    ASH_CHECK(block.run_indices.size() == (block.fonts.size() + 1), "");
    ASH_CHECK(!block.run_indices.is_empty(),
              "No run styling provided for text");
    ASH_CHECK(block.run_indices.last() >= text_size,
              "Text runs need to span the entire text");

    SBAllocator sb_allocator_impl{
      .user_data = scratch.self,
      .allocate  = [](void * user_data, usize size, usize alignment,
                     void ** out_ptr) -> SBBoolean {
          auto allocator = (IAllocator *) user_data;
          auto layout    = Layout{.alignment = alignment, .size = size};
          return allocator->alloc(layout, *((u8 **) out_ptr)) ? SBTrue :
                                                                 SBFalse;
      },
      .deallocate =
        [](void * user_data, void * mem, usize size, usize alignment) {
            auto allocator = (IAllocator *) user_data;
            auto layout    = Layout{.alignment = alignment, .size = size};
            allocator->dealloc(layout, (u8 *) mem);
        }};
    SBAllocatorRef sb_allocator = &sb_allocator_impl;

    auto language = block.language.is_empty() ?
                      hb_language_get_default() :
                      hb_language_from_string(block.language.data(),
                                              (i32) block.language.size());

    TextLayoutBufferImpl & buffer = get_thread_layout_buffer();
    auto level_runs               = Vec<usize>::make(1'024, scratch).unwrap();
    auto levels                   = Vec<u8>::make(1'024, scratch).unwrap();
    auto script_runs              = Vec<usize>::make(1'024, scratch).unwrap();
    auto scripts = Vec<TextScript>::make(1'024, scratch).unwrap();

    layout.clear();

    // - the block never has empty paragraphs
    // - paragraphs never have empty lines; they may have empty codepoints or
    // break codepoints
    // - lines never have empty runs; they may have empty codepoints
    // - runs may have empty codepoints

    auto p = 0uz;

    auto style_iter = RunItemView{block.run_indices.view()}.begin();

    f32x2 extent{};
    usize caret_iter = 0;

    // do-while is used to ensure at least one paragraph is processed even if the
    // text is empty
    do
    {
        buffer.clear();
        level_runs.clear();
        levels.clear();
        script_runs.clear();
        scripts.clear();

        auto paragraph_begin = p;
        auto delims          = advance_paragraph(text.slice(paragraph_begin));
        auto paragraph_delims =
          Slice::slice(paragraph_begin + delims.offset, delims.span);
        auto paragraph_end        = paragraph_delims.offset;
        auto paragraph_size       = paragraph_end - paragraph_begin;
        auto paragraph_runs_begin = layout.runs.size();
        auto paragraph_text =
          text.slice(Slice::offsets(paragraph_begin, paragraph_end));
        auto sb_codepoints =
          SBCodepointSequence{.stringEncoding = SBStringEncodingUTF32,
                              .stringBuffer   = (void *) paragraph_text.data(),
                              .stringLength   = paragraph_text.size()};
        SBAlgorithmRef sb_algorithm =
          SBAlgorithmCreate(sb_allocator, &sb_codepoints);
        ASH_CHECK(sb_algorithm != nullptr, "");
        defer sb_algorithm_{
          [&] { SBAlgorithmRelease(sb_allocator, sb_algorithm); }};

        auto paragraph_level = paragraph_levels(
          sb_allocator, paragraph_text, sb_algorithm, block.direction, levels);
        paragraph_script_runs(sb_allocator, paragraph_text, script_runs,
                              scripts);

        auto run_iter = 0uz;
        auto scripts_iter =
          RunItemView{script_runs.view(), scripts.view()}.begin();
        auto levels_iter =
          RunItemView{level_runs.view(), levels.view()}.begin();
        auto wrap_level = u32{0};

        // do-while is used to ensure at least one run is processed even if the
        // paragraph is empty (empty paragraphs)
        do
        {
            auto run_begin = run_iter;
            style_iter.seek(paragraph_begin + run_begin);

            auto run_props =
              (run_begin < paragraph_size) ?
                RunProps{.type   = classify_run_type(paragraph_text[run_begin]),
                         .style  = static_cast<u32>(style_iter.run()),
                         .script = (*scripts_iter).v0,
                         .base_level = paragraph_level,
                         .level      = (*levels_iter).v0,
                         .wrap_level = wrap_level} :
                RunProps{.type       = TextRunType::Char,
                         .style      = 0,
                         .script     = TextScript::None,
                         .base_level = 0,
                         .level      = 0,
                         .wrap_level = 0};

            if (run_iter < paragraph_size)
            {
                ++run_iter;
            }

            if (!is_wrap_char(paragraph_text[run_begin]))
            {
                while (run_iter < paragraph_size &&
                       run_props.style == style_iter.run() &&
                       run_props.script == (*scripts_iter).v0 &&
                       run_props.level == (*levels_iter).v0 &&
                       run_props.level == levels[run_iter] &&
                       !is_wrap_char(paragraph_text[run_iter]))
                {
                    ++run_iter;
                    ++style_iter;
                    ++scripts_iter;
                    ++levels_iter;
                }
            }
            else
            {
                wrap_level++;
            }

            auto run_end = run_iter;

            auto & font_style = block.fonts[run_props.style];
            auto & font       = get_impl(font_style.font);

            auto run = Slice::offsets(run_begin, run_end);
            auto [infos, positions] =
              shape_run(font.hb_font, buffer.hb_buffer_, paragraph_text, run,
                        hb_script_from_iso15924_tag(SBScriptGetUnicodeTag(
                          SBScript{(u8) run_props.script})),
                        ((paragraph_level & 0x1) == 0) ? HB_DIRECTION_LTR :
                                                         HB_DIRECTION_RTL,
                        language, block.use_kerning, block.use_ligatures);

            auto block_run = Slice::offsets(paragraph_begin + run_begin,
                                            paragraph_begin + run_end);

            insert_run(layout, font_style, block_run, paragraph_begin,
                       run_props, font.metrics, infos, positions);

        } while (run_iter < paragraph_size);

        auto paragraph_runs_end = layout.runs.size();

        layout.paragraphs
          .push(Paragraph{
            .runs = Slice::offsets(paragraph_runs_begin, paragraph_runs_end),
            .codepoints = Slice::offsets(paragraph_begin, paragraph_end),
            .delimeters = paragraph_delims})
          .unwrap();

        layout_paragraph(layout.paragraphs.last(), max_width, block, layout,
                         caret_iter, extent);

        p = paragraph_delims.end();
    } while (p < text_size);

    layout.max_width      = max_width;
    layout.num_carets     = max(caret_iter, 1ULL);
    layout.num_codepoints = text_size;
    layout.extent         = extent;
    layout.laid_out       = true;
}

}    // namespace ash
