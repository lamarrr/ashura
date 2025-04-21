/// SPDX-License-Identifier: MIT
#include "ashura/engine/systems.h"
#include "ashura/std/allocator.h"
#include "ashura/std/error.h"

namespace ash
{

Future<Result<Vec<u8>, IoErr>> FileSystem::load_file(Str path)
{
  Vec<char> path_copy{allocator_};
  path_copy.extend(path).unwrap();

  Future fut = future<Result<Vec<u8>, IoErr>>(allocator_).unwrap();

  scheduler->once(
    [this, path = std::move(path_copy), fut = fut.alias()]() {
      Vec<u8> data{allocator_};
      read_file(path, data)
        .match([&](Void) { fut.yield(Ok{std::move(data)}).unwrap(); },
               [&](IoErr err) { fut.yield(Err{err}).unwrap(); });
    },
    Ready{}, TaskSchedule{.target = TaskTarget::Worker});

  return fut;
}

void FileSystem::shutdown()
{
}

ImageInfo ImageSystem::create_image_(Vec<char>                      label,
                                     gpu::ImageInfo const &         info,
                                     Span<gpu::ImageViewInfo const> view_infos)
{
  gpu::Image gpu_image = sys->gpu.device_->create_image(info).unwrap();

  Image image{.label = std::move(label), .info = info, .image = gpu_image};

  for (gpu::ImageViewInfo view_info : view_infos)
  {
    view_info.image = gpu_image;
    gpu::ImageView view =
      sys->gpu.device_->create_image_view(view_info).unwrap();
    TextureId tex_id = sys->gpu.alloc_texture_id(view);
    image.view_infos.push(view_info).unwrap();
    image.views.push(view).unwrap();
    image.textures.push(tex_id).unwrap();
  }

  ImageId id = ImageId{images_.push(std::move(image)).unwrap()};

  Image & img = images_[(usize) id].v0;
  img.id      = id;

  return img.to_view();
}

void ImageSystem::shutdown()
{
  while (!images_.is_empty())
  {
    unload(ImageId{images_.to_id(0)});
  }
}

ImageInfo ImageSystem::upload_(Vec<char> label, gpu::ImageInfo const & info,
                               Span<gpu::ImageViewInfo const> view_infos,
                               Span<u8 const>                 channels)
{
  CHECK(info.type == gpu::ImageType::Type2D, "");
  CHECK(
    (info.usage & ~(gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferSrc |
                    gpu::ImageUsage::TransferDst)) == gpu::ImageUsage::None,
    "");
  CHECK(info.aspects == gpu::ImageAspects::Color, "");
  CHECK(info.extent.z == 1, "");
  CHECK(info.mip_levels == 1, "");
  CHECK(info.array_layers > 0, "");
  CHECK(view_infos.size() > 0, "");
  CHECK(info.sample_count == gpu::SampleCount::C1, "");
  CHECK(info.format == gpu::Format::R8G8B8A8_UNORM ||
          info.format == gpu::Format::R8G8B8_UNORM ||
          info.format == gpu::Format::B8G8R8A8_UNORM,
        "");

  gpu::Format resolved_format = gpu::Format::B8G8R8A8_UNORM;

  u64 const bgra_size =
    pixel_size_bytes(info.extent.xy(), 4) * info.array_layers;

  Vec<u8> bgra_tmp{allocator_};

  Span<u8 const> bgra;

  switch (info.format)
  {
    case gpu::Format::R8G8B8A8_UNORM:
    {
      bgra_tmp.extend_uninit(bgra_size).unwrap();
      ImageLayerSpan<u8, 4> dst{.channels = bgra_tmp,
                                .extent   = info.extent.xy(),
                                .layers   = info.array_layers};

      ImageLayerSpan<u8 const, 4> src{.channels = channels,
                                      .extent   = info.extent.xy(),
                                      .layers   = info.array_layers};

      for (u32 i = 0; i < info.array_layers; i++)
      {
        copy_RGBA_to_BGRA(src.layer(i), dst.layer(i));
      }

      bgra = bgra_tmp;
    }
    break;
    case gpu::Format::R8G8B8_UNORM:
    {
      bgra_tmp.extend_uninit(bgra_size).unwrap();
      ImageLayerSpan<u8, 4> dst{.channels = bgra_tmp,
                                .extent   = info.extent.xy(),
                                .layers   = info.array_layers};

      ImageLayerSpan<u8 const, 3> src{.channels = channels,
                                      .extent   = info.extent.xy(),
                                      .layers   = info.array_layers};

      for (u32 i = 0; i < info.array_layers; i++)
      {
        copy_RGB_to_BGRA(src.layer(i), dst.layer(i), U8_MAX);
      }

      bgra = bgra_tmp;
    }
    break;
    case gpu::Format::B8G8R8A8_UNORM:
    {
      bgra = channels;
    }
    break;
    default:
      break;
  }

  gpu::ImageInfo resolved_info = info;
  resolved_info.format         = resolved_format;

  Vec<gpu::ImageViewInfo> resolved_view_infos =
    vec(allocator_, view_infos).unwrap();

  for (gpu::ImageViewInfo & info : resolved_view_infos)
  {
    info.view_format = resolved_format;
  }

  ImageInfo image =
    create_image_(std::move(label), resolved_info, resolved_view_infos);

  sys->gpu.upload(bgra, [image = image.image, info](gpu::CommandEncoder & enc,
                                                    gpu::Buffer buffer,
                                                    Slice64     slice) {
    enc.copy_buffer_to_image(
      buffer, image,
      span({
        gpu::BufferImageCopy{
                             .buffer_offset       = slice.offset,
                             .buffer_row_length   = info.extent.x,
                             .buffer_image_height = info.extent.y,
                             .image_layers{.aspects           = gpu::ImageAspects::Color,
                        .mip_level         = 0,
                        .first_array_layer = 0,
                        .num_array_layers  = info.array_layers},
                             .image_area{.offset{0, 0, 0}, .extent = info.extent}}
    }));
  });

  return image;
}

Result<ImageInfo, ImageLoadErr>
  ImageSystem::load_from_memory(Vec<char> label, gpu::ImageInfo const & info,
                                Span<gpu::ImageViewInfo const> view_infos,
                                Span<u8 const>                 channels)
{
  return Ok{upload_(std::move(label), info, view_infos, channels)};
}

Future<Result<ImageInfo, ImageLoadErr>>
  ImageSystem::load_from_path(Vec<char> label, Str path)
{
  Future fut = future<Result<ImageInfo, ImageLoadErr>>(allocator_).unwrap();
  Future load_fut = sys->file.load_file(path);

  scheduler->once(
    [fut = fut.alias(), load_fut = load_fut.alias(), label = std::move(label),
     this]() mutable {
      load_fut.get().match(
        [&, this, label = std::move(label)](Vec<u8> & buffer) mutable {
          trace("Decoding image {} ", label);
          Vec<u8> channels{allocator_};
          decode_image(buffer, channels)
            .match(
              [&, this](DecodedImageInfo const & info) {
                trace("Succesfully decoded image {}", label);
                scheduler->once(
                  [fut = fut.alias(), channels = std::move(channels), this,
                   info, label                 = std::move(label)]() mutable {
                    Span label_view = label.view();
                    fut
                      .yield(Ok{
                        upload_(std::move(label),
                                gpu::ImageInfo{
                                               .label  = label_view,
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
                                span({gpu::ImageViewInfo{
                                  .label           = label_view,
                                  .image           = nullptr,
                                  .view_type       = gpu::ImageViewType::Type2D,
                                  .view_format     = info.format,
                                  .mapping         = {},
                                  .aspects         = gpu::ImageAspects::Color,
                                  .first_mip_level = 0,
                                  .num_mip_levels  = 1,
                                  .first_array_layer = 0,
                                  .num_array_layers  = 1}}
                                ),
                                channels)
                    })
                      .unwrap();
                  },
                  Ready{}, TaskSchedule{.target = TaskTarget::Main});
              },
              [&](ImageLoadErr err) {
                trace("Failed to decode image {}", label);
                fut.yield(Err{err}).unwrap();
              });
        },
        [&](IoErr err) {
          trace("Failed to load image {}", label);
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

Option<ImageInfo> ImageSystem::get(Str label)
{
  for (auto & image : images_.dense.v0)
  {
    if (mem::eq(label, image.label.view()))
    {
      return image.to_view();
    }
  }

  return none;
}

ImageInfo ImageSystem::get(ImageId id)
{
  CHECK(images_.is_valid_id((usize) id), "");
  return images_[(usize) id].v0.to_view();
}

void ImageSystem::unload(ImageId id)
{
  ImageInfo image = get(id);
  for (TextureId id : image.textures)
  {
    sys->gpu.release_texture_id(id);
  }
  for (gpu::ImageView view : image.views)
  {
    sys->gpu.release(view);
  }
  sys->gpu.release(image.image);
  images_.erase((usize) id);
}

void ShaderSystem::shutdown()
{
  while (!shaders_.is_empty())
  {
    unload(ShaderId{shaders_.to_id(0)});
  }
}

Result<ShaderInfo, ShaderLoadErr>
  ShaderSystem::load_from_memory(Vec<char> label, Span<u32 const> spirv)
{
  gpu::Shader object =
    sys->gpu.device_
      ->create_shader(gpu::ShaderInfo{.label = label, .spirv_code = spirv})
      .unwrap();

  ShaderId id =
    ShaderId{shaders_.push(Shader{.label = std::move(label), .shader = object})
               .unwrap()};

  Shader & shader = shaders_[(usize) id].v0;
  shader.id       = id;

  return Ok{shader.view()};
}

Future<Result<ShaderInfo, ShaderLoadErr>>
  ShaderSystem::load_from_path(Vec<char> label, Str path)
{
  Future load_fut = sys->file.load_file(path);
  Future fut = future<Result<ShaderInfo, ShaderLoadErr>>(allocator_).unwrap();

  scheduler->once(
    [fut = fut.alias(), load_fut = load_fut.alias(), label = std::move(label),
     this]() mutable {
      load_fut.get().match(
        [&, label = std::move(label), this](Vec<u8> & spirv) mutable {
          scheduler->once(
            [fut = std::move(fut), spirv = std::move(spirv),
             label = std::move(label), this]() mutable {
              static_assert(spirv.alignment() >= alignof(u32));
              static_assert(std::endian::native == std::endian::little);
              fut
                .yield(load_from_memory(std::move(label),
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

ShaderInfo ShaderSystem::get(ShaderId id)
{
  CHECK(shaders_.is_valid_id((usize) id), "");
  return shaders_[(usize) id].v0.view();
}

Option<ShaderInfo> ShaderSystem::get(Str label)
{
  for (auto [shader] : shaders_)
  {
    if (mem::eq(label, shader.label.view()))
    {
      return shader.view();
    }
  }

  return none;
}

void ShaderSystem::unload(ShaderId id)
{
  Shader & shader = shaders_[(usize) id].v0;
  sys->gpu.release(shader.shader);
  shaders_.erase((usize) id);
}

Systems * sys = nullptr;

}    // namespace ash
