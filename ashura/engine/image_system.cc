/// SPDX-License-Identifier: MIT
#include "ashura/engine/image_system.h"
#include "ashura/engine/file_system.h"
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/image_decoder.h"
#include "ashura/engine/systems.h"
#include "ashura/std/image.h"

namespace ash
{

ImageInfo IImageSys::create_image_(Vec<char> label, gpu::ImageInfo const & info,
                                   Span<gpu::ImageViewInfo const> view_infos)
{
  gpu::Image gpu_image = sys.gpu->device()->create_image(info).unwrap();

  Image image{.label = std::move(label), .info = info, .image = gpu_image};

  for (gpu::ImageViewInfo view_info : view_infos)
  {
    view_info.image = gpu_image;
    gpu::ImageView view =
      sys.gpu->device()->create_image_view(view_info).unwrap();
    TextureId tex_id = sys.gpu->alloc_texture_id(view);
    image.view_infos.push(view_info).unwrap();
    image.views.push(view).unwrap();
    image.textures.push(tex_id).unwrap();
  }

  ImageId id = ImageId{images_.push(std::move(image)).unwrap()};

  Image & img = images_[(usize) id].v0;
  img.id      = id;

  return img.to_view();
}

void IImageSys::shutdown()
{
  while (!images_.is_empty())
  {
    unload(ImageId{images_.to_id(0)});
  }
}

ImageInfo IImageSys::upload_(Vec<char> label, gpu::ImageInfo const & info,
                             Span<gpu::ImageViewInfo const> view_infos,
                             Span<u8 const>                 channels)
{
  CHECK(info.type == gpu::ImageType::Type2D, "");
  CHECK(
    (info.usage & ~(gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferSrc |
                    gpu::ImageUsage::TransferDst)) == gpu::ImageUsage::None,
    "");
  CHECK(info.aspects == gpu::ImageAspects::Color, "");
  CHECK(info.extent.z() == 1, "");
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

  auto buffer_id = sys.gpu->plan()->push_gpu(bgra);

  sys.gpu->plan()->add_pass([buffer_id, img = image.image,
                             info](GpuFrame frame, gpu::CommandEncoder enc) {
    auto buffer = frame->get(buffer_id);
    enc->copy_buffer_to_image(
      buffer.buffer.buffer, img,
      span({
        gpu::BufferImageCopy{
                             .buffer_offset       = buffer.slice.offset,
                             .buffer_row_length   = info.extent.x(),
                             .buffer_image_height = info.extent.y(),
                             .image_layers{
            .aspects      = gpu::ImageAspects::Color,
            .mip_level    = 0,
            .array_layers = {0, info.array_layers},

          }, .image_area{.offset{0, 0, 0}, .extent = info.extent}}
    }));
  });

  return image;
}

Result<ImageInfo, ImageLoadErr>
  IImageSys::load_from_memory(Vec<char> label, gpu::ImageInfo const & info,
                              Span<gpu::ImageViewInfo const> view_infos,
                              Span<u8 const>                 channels)
{
  return Ok{upload_(std::move(label), info, view_infos, channels)};
}

Future<Result<ImageInfo, ImageLoadErr>>
  IImageSys::load_from_path(Vec<char> label, Str path)
{
  Future fut = future<Result<ImageInfo, ImageLoadErr>>(allocator_).unwrap();
  Future load_fut = sys.file->load_file(allocator_, path);

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
                        upload_(
                          std::move(label),
                          gpu::ImageInfo{.label  = label_view,
                                         .type   = gpu::ImageType::Type2D,
                                         .format = info.format,
                                         .usage  = gpu::ImageUsage::Sampled |
                                                  gpu::ImageUsage::TransferDst |
                                                  gpu::ImageUsage::TransferSrc,
                                         .aspects    = gpu::ImageAspects::Color,
                                         .extent     = info.extent.append(1),
                                         .mip_levels = 1,
                                         .array_layers = 1,
                                         .sample_count = gpu::SampleCount::C1},
                          span({gpu::ImageViewInfo{
                            .label       = label_view,
                            .image       = nullptr,
                            .view_type   = gpu::ImageViewType::Type2D,
                            .view_format = info.format,
                            .mapping     = {},
                            .aspects     = gpu::ImageAspects::Color,
                            .mip_levels{0, 1},
                            .array_layers{0, 1}}}
                          ),
                          channels)
                    })
                      .unwrap();
                  },
                  Ready{}, ThreadId::Main);
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
    AwaitFutures{load_fut.alias()}, ThreadId::AnyWorker);

  return fut;
}

Option<ImageInfo> IImageSys::get(Str label)
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

ImageInfo IImageSys::get(ImageId id)
{
  CHECK(images_.is_valid_id((usize) id), "");
  return images_[(usize) id].v0.to_view();
}

void IImageSys::unload(ImageId id)
{
  ImageInfo image = get(id);
  for (TextureId id : image.textures)
  {
    sys.gpu->release_texture_id(id);
  }
  for (gpu::ImageView view : image.views)
  {
    sys.gpu->plan()->add_preframe_task(
      [view, dev = sys.gpu->device()] { dev->uninit(view); });
  }
  sys.gpu->plan()->add_preframe_task(
    [image = image.image, dev = sys.gpu->device()] { dev->uninit(image); });
  images_.erase((usize) id);
}

}    // namespace ash
