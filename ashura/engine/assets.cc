/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/assets.h"
#include "ashura/engine/font_impl.h"
#include "ashura/engine/rect_pack.h"
#include "ashura/std/allocator.h"
#include "ashura/std/error.h"
#include "ashura/std/range.h"

namespace ash
{

Future<Result<Vec<u8>, IoErr>> FileSystem::load_file(Span<char const> path,
                                                     AllocatorImpl    allocator)
{
  PathVec path_copy;
  path_copy.extend(path).unwrap("Maximum path size exceeded"_str);

  Future fut = future<Result<Vec<u8>, IoErr>>(allocator).unwrap();

  async::once(
    [allocator, path = path_copy, fut = fut.alias()]() {
      Vec<u8> data{allocator};
      read_file(path, data)
        .match([&](Void) { fut.yield(Ok{std::move(data)}).unwrap(); },
               [&](IoErr err) { fut.yield(Err{err}).unwrap(); });
    },
    Ready{}, TaskSchedule{.target = TaskTarget::Worker});

  return fut;
}

void FileSystem::init(Scheduler &)
{
}

void FileSystem::shutdown()
{
}

Image ImageSystem::create_image_(Span<char const>           label,
                                 gpu::ImageInfo const &     info,
                                 gpu::ImageViewInfo const & view_info_)
{
  gpu::Image image = sys->gpu.device->create_image(info).unwrap();

  gpu::ImageViewInfo view_info{view_info_};
  view_info.image = image;

  gpu::ImageView view = sys->gpu.device->create_image_view(view_info).unwrap();

  TextureId tex_id = sys->gpu.alloc_texture_id(view);

  ImageId id = ImageId{images_
                         .push(Image{.label           = label,
                                     .texture         = tex_id,
                                     .image_info      = info,
                                     .image_view_info = view_info,
                                     .image           = image,
                                     .image_view      = view})
                         .unwrap()};

  Image & img = images_[id].v0;
  img.id      = id;

  return img;
}

void ImageSystem::init()
{
}

void ImageSystem::shutdown()
{
}

Image ImageSystem::upload(gpu::ImageInfo const & info, Span<u8 const> channels)
{
  CHECK(info.type == gpu::ImageType::Type2D);
  CHECK(
    (info.usage & ~(gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferSrc |
                    gpu::ImageUsage::TransferDst)) == gpu::ImageUsage::None);
  CHECK(info.aspects == gpu::ImageAspects::Color);
  CHECK(info.extent.z == 1);
  CHECK(info.mip_levels == 1);
  CHECK(info.array_layers == 1);
  CHECK(info.sample_count == gpu::SampleCount::C1);
  CHECK(info.format == gpu::Format::R8G8B8A8_UNORM ||
        info.format == gpu::Format::R8G8B8_UNORM ||
        info.format == gpu::Format::B8G8R8A8_UNORM);

  u64 const bgra_size = pixel_size_bytes(info.extent.xy(), 4);

  Vec<u8> bgra{default_allocator};
  bgra.extend_uninit(bgra_size).unwrap();

  ImageSpan<u8, 4> bgra_span{
    .channels = bgra, .extent = info.extent.xy(), .stride = info.extent.x};

  switch (info.format)
  {
    case gpu::Format::R8G8B8A8_UNORM:
    {
      copy_RGBA_to_BGRA(ImageSpan<u8 const, 4>{.channels = channels,
                                               .extent   = info.extent.xy(),
                                               .stride   = info.extent.x},
                        bgra_span);
    }
    break;
    case gpu::Format::R8G8B8_UNORM:
    {
      copy_RGB_to_BGRA(ImageSpan<u8 const, 3>{.channels = channels,
                                              .extent   = info.extent.xy(),
                                              .stride   = info.extent.x},
                       bgra_span, U8_MAX);
    }
    break;
    case gpu::Format::B8G8R8A8_UNORM:
    {
      copy_image(ImageSpan<u8 const, 4>{.channels = channels,
                                        .extent   = info.extent.xy(),
                                        .stride   = info.extent.x},
                 bgra_span);
    }
    break;
    default:
      break;
  }

  Image image =
    create_image_(info.label, info,
                  gpu::ImageViewInfo{.label       = info.label,
                                     .image       = nullptr,
                                     .view_type   = gpu::ImageViewType::Type2D,
                                     .view_format = info.format,
                                     .mapping     = {},
                                     .aspects     = gpu::ImageAspects::Color,
                                     .first_mip_level   = 0,
                                     .num_mip_levels    = 1,
                                     .first_array_layer = 0,
                                     .num_array_layers  = 1});

  sys->gpu.upload_.queue(bgra, [image = image.image,
                                info](gpu::CommandEncoder & enc,
                                      gpu::Buffer buffer, Slice64 slice) {
    enc.copy_buffer_to_image(
      buffer, image,
      span({
        gpu::BufferImageCopy{.buffer_offset       = slice.offset,
                             .buffer_row_length   = info.extent.x,
                             .buffer_image_height = info.extent.y,
                             .image_layers{.aspects = gpu::ImageAspects::Color,
                                           .mip_level         = 0,
                                           .first_array_layer = 0,
                                           .num_array_layers  = 1},
                             .image_offset{0, 0, 0},
                             .image_extent{info.extent.x, info.extent.y, 1}}
    }));
  });

  return image;
}

Result<Image, ImageLoadErr>
  ImageSystem::load_from_memory(Span<char const> label, Vec2U extent,
                                gpu::Format format, Span<u8 const> buffer)
{
  return Ok{
    upload(gpu::ImageInfo{.label  = label,
                          .type   = gpu::ImageType::Type2D,
                          .format = format,
                          .usage  = gpu::ImageUsage::Sampled |
                                   gpu::ImageUsage::TransferDst |
                                   gpu::ImageUsage::TransferSrc,
                          .aspects = gpu::ImageAspects::Color,
                          .extent{extent.x, extent.y, 1},
                          .mip_levels   = 1,
                          .array_layers = 1,
                          .sample_count = gpu::SampleCount::C1},
           buffer)
  };
}

Future<Result<Image, ImageLoadErr>>
  ImageSystem::load_from_path(Span<char const> label, Span<char const> path,
                              AllocatorImpl allocator)
{
  Future fut      = future<Result<Image, ImageLoadErr>>(allocator).unwrap();
  Future load_fut = FileSystem::load_file(path);

  async::once(
    [fut = fut.alias(), load_fut = load_fut.alias(), label, allocator, this]() {
      load_fut.get().match(
        [&, this, label](Vec<u8> & buffer) {
          Vec<u8> channels{allocator};
          decode_image(buffer, channels)
            .match(
              [&, this](DecodedImageInfo const & info) {
                async::once(
                  [fut = fut.alias(), channels = std::move(channels), this,
                   info, label] {
                    fut
                      .yield(Ok{
                        upload(
                          gpu::ImageInfo{
                                         .label  = label,
                                         .type   = gpu::ImageType::Type2D,
                                         .format = info.format,
                                         .usage  = gpu::ImageUsage::Sampled |
                                     gpu::ImageUsage::TransferDst |
                                     gpu::ImageUsage::TransferSrc,
                                         .aspects = gpu::ImageAspects::Color,
                                         .extent{info.extent.x, info.extent.y, 1},
                                         .mip_levels   = 1,
                                         .array_layers = 1,
                                         .sample_count = gpu::SampleCount::C1},
                          channels)
                    })
                      .unwrap();
                  },
                  Ready{}, TaskSchedule{.target = TaskTarget::Main});
              },
              [&](ImageLoadErr err) { fut.yield(Err{err}).unwrap(); });
        },
        [&](IoErr err) {
          fut
            .yield(Err{err == IoErr::InvalidFileOrDir ?
                         ImageLoadErr::InvalidPath :
                         ImageLoadErr::IoErr})
            .unwrap();
        });
    },
    AwaitFutures{load_fut.alias()}, TaskSchedule{.target = TaskTarget::Worker});

  return fut;
}

Image ImageSystem::get(Span<char const> label)
{
  for (auto & image : images_.dense.v0)
  {
    if (mem::eq(label, image.label))
    {
      return image;
    }
  }

  CHECK_DESC(false, "Invalid Image label: ", label);
}

Image ImageSystem::get(ImageId id)
{
  return images_[id].v0;
}

void ImageSystem::unload(ImageId id)
{
  Image image = get(id);
  sys->gpu.release(image.image);
  sys->gpu.release(image.image_view);
  sys->gpu.release_texture_id(image.texture);
  images_.erase(id);
}

void FontSystem::init()
{
}

void FontSystem::shutdown()
{
}

Result<Dyn<Font *>, FontLoadErr>
  FontSystem::decode_(Span<char const> label, Span<u8 const> encoded, u32 face)
{
  Vec<char> font_data{default_allocator};
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
  if (FT_Init_FreeType(&ft_lib) != 0)
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

  if (FT_New_Memory_Face(ft_lib, (FT_Byte const *) font_data.data(),
                         (FT_Long) font_data.size(), 0, &ft_face) != 0)
  {
    return Err{FontLoadErr::DecodeFailed};
  }

  if (FT_Set_Char_Size(ft_face, AU_UNIT, AU_UNIT, 72, 72) != 0)
  {
    return Err{FontLoadErr::DecodeFailed};
  }

  defer ft_face_{[&] {
    if (ft_face != nullptr)
    {
      FT_Done_Face(ft_face);
    }
  }};

  char const * ft_postscript_name = FT_Get_Postscript_Name(ft_face);

  InplaceVec<char, FontImpl::MAX_NAME_SIZE> postscript_name;
  InplaceVec<char, FontImpl::MAX_NAME_SIZE> family_name;
  InplaceVec<char, FontImpl::MAX_NAME_SIZE> style_name;

  if (ft_postscript_name != nullptr)
  {
    postscript_name.extend(Span{ft_postscript_name, strlen(ft_postscript_name)})
      .unwrap();
  }

  if (ft_face->family_name != nullptr)
  {
    family_name.extend(Span{ft_face->family_name, strlen(ft_face->family_name)})
      .unwrap();
  }

  if (ft_face->style_name != nullptr)
  {
    style_name.extend(Span{ft_face->style_name, strlen(ft_face->style_name)})
      .unwrap();
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

  Vec<GlyphMetrics> glyphs{default_allocator};

  if (!glyphs.resize(num_glyphs))
  {
    return Err{FontLoadErr::OutOfMemory};
  }

  for (auto [i, metric] : enumerate<u32>(glyphs))
  {
    if (FT_Load_Glyph(ft_face, i, FT_LOAD_DEFAULT) == 0)
    {
      FT_GlyphSlot s = ft_face->glyph;

      // bin offsets are determined after binning and during rect packing
      metric = GlyphMetrics{
        .bearing{(i32) s->metrics.horiBearingX, (i32) -s->metrics.horiBearingY},
        .advance = (i32) s->metrics.horiAdvance,
        .extent{(i32) s->metrics.width,        (i32) s->metrics.height       }
      };
    }
  }

  Result font = dyn_inplace<FontImpl>(
    default_allocator, FontId::Default, label, std::move(font_data),
    std::move(postscript_name), std::move(family_name), std::move(style_name),
    hb_blob, hb_face, hb_font, ft_lib, ft_face, face, std::move(glyphs),
    replacement_glyph, ellipsis_glyph, space_glyph,
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

  return Ok{cast<ash::Font *>(std::move(font.value()))};
}

Result<> FontSystem::rasterize(Font & font_, u32 font_height)
{
  FontImpl &           font             = (FontImpl &) font_;
  static constexpr u32 MIN_ATLAS_EXTENT = 512;
  static_assert(MIN_ATLAS_EXTENT > 0, "Font atlas extent must be non-zero");
  static_assert(MIN_ATLAS_EXTENT > 128,
                "Font atlas extent must be at least 128px");
  static_assert(MIN_ATLAS_EXTENT % 64 == 0,
                "Font atlas extent should be a multiple of 64");
  static_assert(MIN_ATLAS_EXTENT <= gpu::MAX_IMAGE_EXTENT_2D,
                "Font atlas extent too large for GPU platform");
  CHECK(font_height <= 1'024);
  CHECK(font_height <= MIN_ATLAS_EXTENT / 8);

  Vec2U atlas_extent{MIN_ATLAS_EXTENT, MIN_ATLAS_EXTENT};

  font.cpu_atlas.expect_none("CPU font atlas has already been loaded");

  CpuFontAtlas atlas;

  u32 const num_glyphs = font.glyphs.size32();

  if (!atlas.glyphs.resize(num_glyphs))
  {
    return Err{};
  }

  if (FT_Set_Pixel_Sizes(font.ft_face, font_height, font_height) != 0)
  {
    return Err{};
  }

  for (auto [i, g] : enumerate<u32>(atlas.glyphs))
  {
    FT_Error ft_error = FT_Load_Glyph(font.ft_face, i, FT_LOAD_DEFAULT);
    if (ft_error != 0)
    {
      continue;
    }

    g.area.extent = Vec2U{font.ft_face->glyph->bitmap.width,
                          font.ft_face->glyph->bitmap.rows};
  }

  static constexpr u16 GLYPH_PADDING = 1;

  u32 num_layers = 0;
  {
    Vec<PackRect> rects{default_allocator};
    RectPacker    packer =
      RectPacker::make(as_vec2i(atlas_extent), default_allocator);

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

      rect = PackRect{
        .pos = {}, .extent = as_vec2i(padded_extent), .packed = false, .id = i};

      atlas_extent.x = max(atlas_extent.x, padded_extent.x);
      atlas_extent.y = max(atlas_extent.y, padded_extent.y);
    }

    CHECK(atlas_extent.x <= gpu::MAX_IMAGE_EXTENT_2D);
    CHECK(atlas_extent.y <= gpu::MAX_IMAGE_EXTENT_2D);

    Vec2 const atlas_scale = 1 / as_vec2(atlas_extent);

    u32 num_packed = 0;

    while (num_packed < num_glyphs)
    {
      // tries to pack all the glyph rects into the provided extent
      packer.reset(as_vec2i(atlas_extent));
      auto [packed, unpacked] = packer.pack(rects.view().slice(num_packed));
      CHECK(!packed.is_empty());
      for (PackRect & rect : rects.view().slice(num_packed))
      {
        atlas.glyphs[rect.id].layer = num_layers;
      }
      num_packed += packed.size32();
      num_layers++;
    }

    // sanity check. ideally all should have been packed
    CHECK(num_packed == num_glyphs);

    for (u32 i = 0; i < num_glyphs; i++)
    {
      PackRect const & r = rects[i];
      AtlasGlyph &     g = atlas.glyphs[r.id];

      if (g.area.extent.x == 0 | g.area.extent.y == 0)
      {
        // adjust back to original position from the padded position
        g.area.offset = as_vec2u(r.pos + GLYPH_PADDING);
      }
      else
      {
        g.area.offset = Vec2U{};
      }

      g.uv[0] = as_vec2(g.area.offset) * atlas_scale;
      g.uv[1] = as_vec2(g.area.end()) * atlas_scale;
    }
  }

  u64 const atlas_area       = (u64) atlas_extent.x * (u64) atlas_extent.y;
  u64 const atlas_layer_size = atlas_area;
  u64 const atlas_size       = atlas_layer_size * num_layers;

  if (!atlas.channels.resize(atlas_size))
  {
    return Err{};
  }

  ImageLayerSpan<u8, 1> atlas_span{
    .channels = atlas.channels, .extent = atlas_extent, .layers = num_layers};

  for (auto [i, ag] : enumerate<u32>(atlas.glyphs))
  {
    FT_GlyphSlot slot     = font.ft_face->glyph;
    FT_Error     ft_error = FT_Load_Glyph(
      font.ft_face, i, FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
    if (ft_error != 0)
    {
      continue;
    }

    CHECK(slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
    /// we don't want to handle negative pitches
    CHECK(slot->bitmap.pitch >= 0);

    ImageSpan<u8 const, 1> src{
      .channels{slot->bitmap.buffer,
                slot->bitmap.rows * (u32) slot->bitmap.pitch},
      .extent = {slot->bitmap.width,  slot->bitmap.rows      },
      .stride = (u32) slot->bitmap.pitch
    };

    copy_image(src, atlas_span.get_layer(ag.layer).slice(ag.area.offset,
                                                         ag.area.extent));
  }

  atlas.font_height = font_height;
  atlas.extent      = atlas_extent;
  atlas.num_layers  = num_layers;

  font.cpu_atlas = std::move(atlas);

  return Ok{};
}

FontId FontSystem::upload_(Dyn<Font *> font_)
{
  FontImpl & font = (FontImpl &) *font_.get();
  CHECK(font.cpu_atlas.is_some());
  CHECK(font.gpu_atlas.is_none());

  CpuFontAtlas & atlas = font.cpu_atlas.value();

  CHECK(atlas.num_layers > 0);
  CHECK(atlas.extent.x > 0);
  CHECK(atlas.extent.y > 0);

  GpuFontAtlas gpu_atlas{.textures{default_allocator},
                         .images{default_allocator},
                         .font_height = atlas.font_height,
                         .extent      = atlas.extent};

  gpu_atlas.glyphs.extend(atlas.glyphs).unwrap();

  Vec<u8> bgra_pixels{default_allocator};
  bgra_pixels.resize(pixel_size_bytes(atlas.extent, 4)).unwrap();

  ImageSpan<u8, 4> bgra{
    .channels = bgra_pixels, .extent = atlas.extent, .stride = atlas.extent.x};

  for (u32 i = 0; i < atlas.num_layers; i++)
  {
    copy_alpha_image_to_BGRA(atlas.span().get_layer(i).as_const(), bgra, U8_MAX,
                             U8_MAX, U8_MAX);
    Image image =
      sys->image
        .load_from_memory(font.label, gpu_atlas.extent,
                          gpu::Format::B8G8R8A8_UNORM, bgra.channels)
        .unwrap();
    gpu_atlas.textures.push(image.texture).unwrap();
    gpu_atlas.images.push(image.id).unwrap();
  }

  font.gpu_atlas = std::move(gpu_atlas);

  // unload CPU atlas
  font.cpu_atlas = none;

  FontId id = FontId{fonts_.push(std::move(font_)).unwrap()};

  FontImpl & f = (FontImpl &) *fonts_[id].v0;

  f.id = id;

  return id;
}

Future<Result<FontId, FontLoadErr>>
  FontSystem::load_from_memory(Span<char const> label, Vec<u8> encoded,
                               u32 font_height, u32 face)
{
  Future fut = future<Result<FontId, FontLoadErr>>(default_allocator).unwrap();
  async::once(
    [fut = fut.alias(), encoded = std::move(encoded), label, this, face,
     font_height]() mutable {
      decode_(label, encoded, face)
        .match(
          [&, this](Dyn<Font *> & font) {
            rasterize(*font, font_height)
              .match(
                [&, this](Void) {
                  async::once(
                    [font = std::move(font), this,
                     fut  = std::move(fut)]() mutable {
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
  FontSystem::load_from_path(Span<char const> label, Span<char const> path,
                             u32 font_height, u32 face)
{
  Future load_fut = FileSystem::load_file(path, default_allocator);

  Future fut = future<Result<FontId, FontLoadErr>>(default_allocator).unwrap();

  async::once(
    [load_fut = load_fut.alias(), fut = std::move(fut), this, label,
     font_height, face]() mutable {
      load_fut.get().match(
        [&](Vec<u8> & encoded) {
          Future mem_load_fut =
            load_from_memory(label, std::move(encoded), font_height, face);

          async::once(
            [fut = fut.alias(), mem_load_fut = mem_load_fut.alias()]() {
              fut.yield(mem_load_fut.get()).unwrap();
            },

            AwaitFutures{load_fut.alias()},
            TaskSchedule{.target = TaskTarget::Worker});
        },
        [&](IoErr err) { fut.yield(Err{err}).unwrap(); });
    },
    AwaitFutures{load_fut.alias()});

  return fut;
}

Font & FontSystem::get(FontId id)
{
  return *fonts_[id].v0;
}

Font & FontSystem::get(Span<char const> label)
{
  for (auto & font : fonts_.dense.v0)
  {
    if (mem::eq(label, font->info().label))
    {
      return *font;
    }
  }
  CHECK_DESC(false, "Invalid Font label: ", label);
}

void FontSystem::unload(FontId id)
{
  Dyn<Font *> & f    = fonts_[id].v0;
  FontImpl &    font = (FontImpl &) *f;

  for (ImageId image : font.gpu_atlas.value().images)
  {
    sys->image.unload(image);
  }

  fonts_.erase(id);
}

void ShaderSystem::init()
{
}

void ShaderSystem::shutdown()
{
}

Result<Shader, ShaderLoadErr>
  ShaderSystem::load_spirv_from_memory(Span<char const> label,
                                       Span<u32 const>  spirv)
{
  gpu::Shader object =
    sys->gpu.device
      ->create_shader(gpu::ShaderInfo{.label = label, .spirv_code = spirv})
      .unwrap();

  ShaderId id =
    ShaderId{shaders_.push(Shader{.label = label, .shader = object}).unwrap()};

  Shader & shader = shaders_[id].v0;
  shader.id       = id;

  return Ok{shader};
}

Future<Result<Shader, ShaderLoadErr>>
  ShaderSystem::load_spirv_from_path(Span<char const> label,
                                     Span<char const> path)
{
  Future load_fut = FileSystem::load_file(path);
  Future fut =
    future<Result<Shader, ShaderLoadErr>>(default_allocator).unwrap();

  async::once(
    [fut = fut.alias(), load_fut = load_fut.alias(), label, this]() mutable {
      load_fut.get().match(
        [&, label, this](Vec<u8> & spirv) {
          async::once(
            [fut = std::move(fut), spirv = std::move(spirv), label, this]() {
              static_assert(MIN_VEC_ALIGNMENT >= alignof(u32));
              static_assert(std::endian::native == std::endian::little);
              fut
                .yield(load_spirv_from_memory(label,
                                              spirv.view().reinterpret<u32>()))
                .unwrap();
            },
            Ready{}, TaskSchedule{.target = TaskTarget::Main});
        },
        [&](IoErr err) {
          fut
            .yield(Err{err == IoErr::InvalidFileOrDir ?
                         ShaderLoadErr::InvalidPath :
                         ShaderLoadErr::IOErr})
            .unwrap();
        });
    },
    AwaitFutures{load_fut.alias()}, TaskSchedule{.target = TaskTarget::Main});

  return fut;
}

Shader ShaderSystem::get(ShaderId id)
{
  return shaders_[id].v0;
}

Shader ShaderSystem::get(Span<char const> label)
{
  for (auto [shader] : shaders_)
  {
    if (mem::eq(label, shader.label))
    {
      return shader;
    }
  }

  CHECK_DESC(false, "Invalid Shader label: ", label);
}

void ShaderSystem::unload(ShaderId id)
{
  Shader & shader = shaders_[id].v0;
  sys->gpu.release(shader.shader);
  shaders_.erase(id);
}

}    // namespace ash
