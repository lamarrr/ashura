/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/image_decoder.h"
#include "ashura/engine/rect_pack.h"
#include "ashura/engine/shader.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/async.h"
#include "ashura/std/fs.h"
#include "ashura/std/map.h"
#include "ashura/std/vec.h"

namespace ash
{

enum WorkerThreadId : u32
{
  Render = 0,
  Audio  = 1,
  Video  = 2
};

// D16_UNORM support and other formatts
// bgr srgb is supported for sampling
//
// srgb color attachment
//
//
// src size
// target size
//
// max size - if set, will not go beyond
// - option<> none if textur not available
// - 2d/3d
// - mipping / coherent (shader requirements)
//
// O(1) navigation
//
//
// budget, updates
//
// have images of size 4096x4096, pack the images into it as they come
//
// [ ] needs separate render thread that only does render work. needed so loading resources and stalling on the main thread will not stall the render thread
// [ ] needs
//
// [ ] images: static read-only. we can pack them, streaming mode. on load, add pre-frame tasks.most images would need to be downsampled first,
// we'll need to categorize or get hints about image usage
// [ ] animations
// [ ] audio
// [ ] ...
//
//

enum class ImageId : u64
{
};

enum class FontId : u64
{
};

enum class AudioId : u64
{
};

enum class VideoId : u64
{
};

enum class ShaderId : u64
{
};

struct Image
{
  ImageId        id{};
  TextureId      texture{};
  gpu::Rect      view_area{};
  Vec2           uv[2]{};
  gpu::ImageInfo info{};
};

struct FileSystem
{
  static Future<Result<Vec<u8>, IoErr>>
    load_file(Span<char const> path,
              AllocatorImpl    allocator = default_allocator)
  {
    InplaceVec<char, MAX_PATH_SIZE> path_copy;
    path_copy.resize(path.size()).expect("Maximum path size exceeded");
    mem::copy(path, path_copy.view());

    Future fut = future<Result<Vec<u8>, IoErr>>(allocator).unwrap();

    async::once(
      [allocator, path = path_copy, fut = fut.alias()]() {
        Vec<u8> data{allocator};
        read_file(path, data)
          .match([&](Void) { fut.yield(Ok{std::move(data)}).unwrap(); },
                 [&](IoErr err) { fut.yield(Err{err}).unwrap(); });
      },
      Ready{}, TaskSchedule{.target = TaskTarget::Worker, .thread = none});

    return fut;
  }
};

// Future<Result<Image, ResourceLoadError>>;
//
// // submit to end/begin of render frame
//
//
//
// schedule task to load the image from disk.
//
// take the image from memory, resize it if needed. upload to gpu, return handle
// use handle to check if it is ready.
//
// - use gpu-visible staging buffer, then forward that
// - can only be used on the main thread
//
// [ ] mt-safe
//
struct ImageSystem
{
  static constexpr u32 ATLAS_PADDING = 1;

  struct Atlas
  {
    gpu::ImageInfo image_info = {};

    gpu::ImageViewInfo image_view_info = {};

    gpu::Image image = nullptr;

    gpu::ImageView image_view = nullptr;

    bool is_multi = false;

    RectPacker packer{};

    Vec<Image> images{};
  };

  gpu::Format format_     = gpu::Format::B8G8R8A8_UNORM;
  /// @brief The size used for compacting the images
  u32         atlas_size_ = 2'048;
  Vec<Atlas>  atlases_;
  InplaceVec<gpu::Buffer, gpu::MAX_FRAME_BUFFERING>           staging_buffers_;
  InplaceVec<gpu::CommandEncoder *, gpu::MAX_FRAME_BUFFERING> encoders_;
  u32                                                         ring_index_ = 0;
  gpu::Device *                                               device_ = nullptr;

  // void init(gpu::Device * dev, u32 buffering)
  // {
  //   staging_buffers_.resize(buffering).unwrap();
  // }

  void validate_image_info(gpu::ImageInfo const & info)
  {
    CHECK(info.type == gpu::ImageType::Type2D);
    CHECK(
      (info.usage & ~(gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferSrc |
                      gpu::ImageUsage::TransferDst)) == gpu::ImageUsage::None);
    CHECK(info.aspects == gpu::ImageAspects::Color ||
          info.aspects == gpu::ImageAspects::Depth ||
          info.aspects ==
            (gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil));
    CHECK(info.extent.z == 1);
    CHECK(info.mip_levels == 1);
    CHECK(info.array_layers == 1);
    CHECK(info.sample_count == gpu::SampleCount::C1);
    CHECK(info.format == format_);
  }

  Tuple<u32, RectU> allocate_rect(gpu::Extent extent)
  {
    if (extent.x <= atlas_size_ && extent.y <= atlas_size_)
    {
      for (auto [i, atlas] : enumerate<u32>(atlases_))
      {
        if (atlas.is_multi)
        {
          PackRect pack_rect[] = {
            {.extent = as_vec2i(extent + ATLAS_PADDING * 2)}};
          auto [packed, _] = atlas.packer.pack(pack_rect);
          if (packed.is_empty())
          {
            continue;
          }

          return {
            i, RectU{as_vec2u(pack_rect[0].pos + ATLAS_PADDING), extent}
          };
        }
      }

      gpu::Image image =
        device_
          ->create_image(gpu::ImageInfo{
            .label  = "Image Atlas",
            .type   = gpu::ImageType::Type2D,
            .format = format_,
            .usage  = gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferDst |
                     gpu::ImageUsage::TransferSrc,
            .aspects = gpu::ImageAspects::Color,
            .extent{atlas_size_, atlas_size_},
            .mip_levels   = 1,
            .array_layers = 1,
            .sample_count = gpu::SampleCount::C1
      })
          .unwrap();

      gpu::ImageView view = device_
                              ->create_image_view(gpu::ImageViewInfo{
                                .label       = "Image Atlas View",
                                .image       = image,
                                .view_type   = gpu::ImageViewType::Type2D,
                                .view_format = format_,
                                .mapping{},
                                .aspects           = gpu::ImageAspects::Color,
                                .first_mip_level   = 0,
                                .num_mip_levels    = 1,
                                .first_array_layer = 0,
                                .num_array_layers  = 1})
                              .unwrap();

      // create new multi-atlas
      // create image
      // create view
      // allocate texture id? immediate? next frame?
      // update texture descriptor set to view
      // copy buffer data to current frame's staging buffer
      // submit pre-frame job to update the texture region
      //
      //
    }
    else
    {
      // create new single atlas with exact size & guaranteed padding
    }
  }

  static void load_from_memory(gpu::ImageInfo const & info, Vec<u8> buffer)
  {
    validate_image_info(info);

    //
    // keep track of images available and slots for them
    //
    //
    //

    // upload data to current frame's staging buffer
    // submit to gpu system
    //
    //
    //
    //
  }

  static Future<Result<Image, ImageLoadErr>>
    load_from_path(Span<char const> label, Span<char const> path,
                   AllocatorImpl allocator = default_allocator)
  {
    Future fut      = future<Result<Image, ImageLoadErr>>(allocator).unwrap();
    Future load_fut = FileSystem::load_file(path);

    async::once(
      [fut = fut.alias(), load_fut = load_fut.alias(), label, allocator]() {
        load_fut.get().match(
          [&](Vec<u8> & buffer) {
            Vec<u8> channels{allocator};
            decode_image(buffer, channels)
              .match(
                [&](DecodedImageInfo const & info) {
                  Vec<u8> bgra_channels{allocator};
                  // [ ] abstract this!!
                  bgra_channels
                    .resize_uninit((u64) info.extent.x * (u64) info.extent.y *
                                   4ULL)
                    .unwrap();

                  ImageSpan<u8, 4> bgra_span{.channels = bgra_channels,
                                             .extent   = info.extent,
                                             .stride   = info.extent.x};

                  switch (info.format)
                  {
                    case gpu::Format::R8G8B8A8_UNORM:
                      copy_RGBA_to_BGRA(
                        ImageSpan<u8 const, 4>{.channels = channels,
                                               .extent   = info.extent,
                                               .stride   = info.extent.x},
                        bgra_span);
                      break;
                    case gpu::Format::R8G8B8_UNORM:
                      copy_RGB_to_BGRA(
                        ImageSpan<u8 const, 3>{.channels = channels,
                                               .extent   = info.extent,
                                               .stride   = info.extent.x},
                        bgra_span, U8_MAX);
                      break;

                    default:
                      CHECK_UNREACHABLE();
                  }

                  //
                  // allocate slot on the atlases for the image
                  //
                  // schedule for upload to GPU
                  //
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
      AwaitFutures{load_fut.alias()},
      TaskSchedule{.target = TaskTarget::Worker});

    return fut;
  }

  static void get(ImageId id);

  static void get(Span<char const> label);

  static void unload(ImageId);
};

// [ ] SPIRV compilation
// [ ] mt
// gpu::Shader
struct ShaderSystem
{
  Result<> compile(ShaderCompileInfo const & info, Vec<u32> out);

  Result<gpu::Shader> load_from_memory(Vec<u32> spirv);

  void get(ShaderId id);

  void get(Span<char const> label);

  void unload(ShaderId);
};

// [ ] usual font store and atlasing then uploading to GPU
// [ ] mt
// Dyn<Font>
struct FontSystem
{
  void load_from_memory();
  void load_from_path();
  void get(FontId id);
  void get(Span<char const> label);
  void unload(FontId);
};

// [ ] load audio, convert into PCM16 using FFMPEG, load into SDL
// [ ] use single audio thread
// [ ] mt
struct AudioSystem
{
  void load_from_memory();
  void load_from_path();
  void get(AudioId id);
  void get(Span<char const> label);
  void unload(AudioId);
};

// [ ] load video frame using FFMPEG, upload to Vulkan
// [ ] use single video thread
// [ ] mt
struct VideoSystem
{
  void load_from_memory();
  void load_from_path();
  void get(VideoId id);
  void get(Span<char const> label);
  void unload(VideoId);

  // add pre-frame task to send the new update to the GPU. use for VIDEO frames.
  void create_image();
};

struct AssetSystem
{
  ImageSystem  image;
  FontSystem   font;
  AudioSystem  audio;
  VideoSystem  video;
  ShaderSystem shader;
};

}    // namespace ash
