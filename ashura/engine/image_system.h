/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/errors.h"
#include "ashura/engine/gpu_system.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

enum class ImageId : u64
{
  None = U64_MAX
};

struct ImageInfo
{
  ImageId id = ImageId::None;

  Str label{};

  Span<TextureIndex const> textures{};

  gpu::ImageInfo info{};

  Span<gpu::ImageViewInfo const> view_infos{};

  gpu::Image image = nullptr;

  Span<gpu::ImageView const> views{};
};

struct Image
{
  ImageId id = ImageId::None;

  Vec<char> label{};

  Vec<TextureIndex> textures{};

  gpu::ImageInfo info{};

  Vec<gpu::ImageViewInfo> view_infos{};

  gpu::Image image = nullptr;

  Vec<gpu::ImageView> views{};

  constexpr ImageInfo to_view() const
  {
    return {.id         = id,
            .label      = label,
            .textures   = textures,
            .info       = info,
            .view_infos = view_infos,
            .image      = image,
            .views      = views};
  }
};

struct IImageSys
{
  Allocator        allocator_;
  SparseVec<Image> images_{};

  explicit IImageSys(Allocator allocator) :
    allocator_{allocator},
    images_{allocator}
  {
  }

  IImageSys(IImageSys const &)             = delete;
  IImageSys(IImageSys &&)                  = default;
  IImageSys & operator=(IImageSys const &) = delete;
  IImageSys & operator=(IImageSys &&)      = default;
  ~IImageSys()                             = default;

  void shutdown();

  ImageInfo create_image_(Vec<char> label, gpu::ImageInfo const & info,
                          Span<gpu::ImageViewInfo const> view_infos);

  ImageInfo upload_(Vec<char> label, gpu::ImageInfo const & info,
                    Span<gpu::ImageViewInfo const> view_infos,
                    Span<u8 const>                 channels);

  Result<ImageInfo, ImageLoadErr>
    load_from_memory(Vec<char> label, gpu::ImageInfo const & info,
                     Span<gpu::ImageViewInfo const> view_infos,
                     Span<u8 const>                 channels);

  Future<Result<ImageInfo, ImageLoadErr>> load_from_path(Vec<char> label,
                                                         Str       path);

  Option<ImageInfo> get(Str label);

  ImageInfo get(ImageId id);

  void unload(ImageId id);
};

}    // namespace ash
