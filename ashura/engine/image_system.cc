/// SPDX-License-Identifier: MIT
#include "ashura/engine/image_system.h"
#include "ashura/engine/file_system.h"
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/image_decoder.h"
#include "ashura/engine/systems.h"
#include "ashura/std/image.h"
#include "ashura/std/trace.h"

namespace ash
{

Result<ImageInfo, SysErr>
  IImageSys::create_image_(Str label_span, gpu::ImageInfo const & info,
                           Span<gpu::ImageViewInfo const> view_infos)
{
    gpu::Image gpu_image = gpu_sys_->device()->create_image(info).unwrap();
    StrVec     label{allocator_};
    if (!label.append(label_span))
    {
        return Err{SysErr::OutOfMemory};
    }

    Image image{.label = std::move(label),
                .textures{allocator_},
                .info = info,
                .view_infos{allocator_},
                .image = gpu_image,
                .views{allocator_}};

    for (gpu::ImageViewInfo view_info : view_infos)
    {
        view_info.image = gpu_image;
        gpu::ImageView view =
          gpu_sys_->device()->create_image_view(view_info).unwrap();
        TextureIndex tex_index = gpu_sys_->alloc_texture_index(view);
        image.view_infos.push(view_info).unwrap();
        image.views.push(view).unwrap();
        image.textures.push(tex_index).unwrap();
    }

    {
        WriteGuard guard{rw_lock_};
        ImageId    id = images_.push(std::move(image)).unwrap();

        Image & img = images_[id].v0;
        img.id      = id;
        return Ok{img.to_view()};
    }
}

void IImageSys::shutdown()
{
    WriteGuard guard{rw_lock_};
    while (!images_.is_empty())
    {
        unload_(images_.to_id(0));
    }
}

Result<ImageInfo, SysErr>
  IImageSys::upload_(Str label_span, gpu::ImageInfo const & info,
                     Span<gpu::ImageViewInfo const> view_infos,
                     Span<u8 const>                 channels)
{
    tracing::ScopeTrace trace;

    ASH_CHECK(info.type == gpu::ImageType::Type2D, "");
    ASH_CHECK(
      (info.usage & ~(gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferSrc |
                      gpu::ImageUsage::TransferDst)) == gpu::ImageUsage::None,
      "");
    ASH_CHECK(info.aspects == gpu::ImageAspects::Color, "");
    ASH_CHECK(info.extent.z() == 1, "");
    ASH_CHECK(info.mip_levels == 1, "");
    ASH_CHECK(info.array_layers > 0, "");
    ASH_CHECK(view_infos.size() > 0, "");
    ASH_CHECK(info.sample_count == gpu::SampleCount::C1, "");
    ASH_CHECK(info.format == gpu::Format::R8G8B8A8_UNORM ||
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
            if (!bgra_tmp.extend_uninit(bgra_size))
            {
                return Err{SysErr::OutOfMemory};
            }

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
            if (!bgra_tmp.extend_uninit(bgra_size))
            {
                return Err{SysErr::OutOfMemory};
            }

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

    Vec<gpu::ImageViewInfo> resolved_view_infos{allocator_};
    if (!resolved_view_infos.append(view_infos))
    {
        return Err{SysErr::OutOfMemory};
    }

    for (gpu::ImageViewInfo & info : resolved_view_infos)
    {
        info.view_format = resolved_format;
    }

    auto image_ = create_image_(label_span, resolved_info, resolved_view_infos);

    if (image_.is_err())
    {
        return Err{image_.err()};
    }

    ImageInfo image = image_.unwrap();

    auto buffer_id = gpu_sys_->current_plan()->push_gpu(bgra);

    gpu_sys_->current_plan()->add_pass(
      [buffer_id, img = image.image, info](GpuFrame            frame,
                                           gpu::CommandEncoder enc) {
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

    return Ok{image};
}

Future<Result<ImageInfo, SysErr>>
  IImageSys::load_from_memory(Str label_span, gpu::ImageInfo const & info,
                              Span<gpu::ImageViewInfo const> view_infos,
                              RcBlob8                        channels)
{
    StrVec label{allocator_};
    label.append(label_span).unwrap();

    Vec<gpu::ImageViewInfo> view_infos_vec{allocator_};
    view_infos_vec.append(view_infos).unwrap();

    auto info_copy = gpu::ImageInfo{.label        = label,
                                    .type         = info.type,
                                    .format       = info.format,
                                    .usage        = info.usage,
                                    .aspects      = info.aspects,
                                    .extent       = info.extent,
                                    .mip_levels   = info.mip_levels,
                                    .array_layers = info.array_layers,
                                    .sample_count = info.sample_count};

    return scheduler_
      ->run(allocator_, MainThread::Main,
            [label = std::move(label), info = info_copy,
             view_infos = std::move(view_infos_vec),
             channels   = std::move(channels), this]() mutable {
                return upload_(label, info, view_infos.view(), channels);
            })
      .unwrap();
}

Future<Result<ImageInfo, SysErr>> IImageSys::load_from_path(Str label_span,
                                                            Str path)
{
    Future load_fut = file_sys_->load_file(allocator_, path);
    StrVec label{allocator_};
    label.append(label_span).unwrap();

    auto decode_fut =
      scheduler_
        ->then(
          allocator_, WorkerThread::Any,
          [allocator = allocator_](Result<Vec<u8>, SysErr> & r) {
              using R = Result<Tuple<Vec<u8>, DecodedImageInfo>, SysErr>;
              return r.match(
                [&](Span<u8 const> buffer) -> R {
                    Vec<u8> channels{allocator};
                    return decode_image(buffer, channels)
                      .map([&](DecodedImageInfo const & info) {
                          return Tuple{std::move(channels), info};
                      });
                },
                [&](SysErr err) -> R { return Err{err}; });
          },
          std::move(load_fut))
        .unwrap();

    return scheduler_
      ->then(
        allocator_, MainThread::Main,
        [label = std::move(label),
         this](Result<Tuple<Vec<u8>, DecodedImageInfo>, SysErr> & r) {
            using R = Result<ImageInfo, SysErr>;
            return r.match(
              [&](Tuple<Vec<u8>, DecodedImageInfo> & t) -> R {
                  gpu::ImageInfo image_info{
                    .label  = label,
                    .type   = gpu::ImageType::Type2D,
                    .format = t.v1.format,
                    .usage  = gpu::ImageUsage::Sampled |
                             gpu::ImageUsage::TransferDst |
                             gpu::ImageUsage::TransferSrc,
                    .aspects      = gpu::ImageAspects::Color,
                    .extent       = t.v1.extent.append(1),
                    .mip_levels   = 1,
                    .array_layers = 1,
                    .sample_count = gpu::SampleCount::C1};

                  gpu::ImageViewInfo view_infos[] = {
                    {.label       = label,
                     .image       = nullptr,
                     .view_type   = gpu::ImageViewType::Type2D,
                     .view_format = t.v1.format,
                     .mapping     = {},
                     .aspects     = gpu::ImageAspects::Color,
                     .mip_levels{0, 1},
                     .array_layers{0, 1}}
                  };

                  return upload_(label, image_info, view_infos, t.v0);
              },
              [](SysErr err) -> R { return Err{err}; });
        },
        std::move(decode_fut))
      .unwrap();
}

Option<ImageInfo> IImageSys::get(Str label)
{
    ReadGuard guard{rw_lock_};
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
    ReadGuard guard{rw_lock_};
    ASH_CHECK(images_.is_valid_id(id), "");
    return images_[id].v0.to_view();
}

void IImageSys::unload_(ImageId id)
{
    ASH_CHECK(images_.is_valid_id(id), "");
    ImageInfo image = images_[id].v0.to_view();
    for (TextureIndex idx : image.textures)
    {
        gpu_sys_->release_texture_index(idx);
    }
    for (gpu::ImageView view : image.views)
    {
        gpu_sys_->current_plan()->add_preframe_task(
          [view, dev = gpu_sys_->device()](GpuFrame) { dev->uninit(view); });
    }
    gpu_sys_->current_plan()->add_preframe_task(
      [image = image.image, dev = gpu_sys_->device()](GpuFrame) {
          dev->uninit(image);
      });

    images_.erase(id);
}

void IImageSys::unload(ImageId id)
{
    WriteGuard guard{rw_lock_};
    unload_(id);
}

}    // namespace ash
