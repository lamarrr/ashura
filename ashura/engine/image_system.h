/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/errors.h"
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/systems.h"
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

    Vec<char> label;

    Vec<TextureIndex> textures;

    gpu::ImageInfo info{};

    Vec<gpu::ImageViewInfo> view_infos;

    gpu::Image image = nullptr;

    Vec<gpu::ImageView> views;

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
    Allocator                 allocator_;
    SparseVec<ImageId, Image> images_;
    IRWSpinLock               rw_lock_;
    GpuSys                    gpu_sys_;
    FileSys                   file_sys_;
    Scheduler                 scheduler_;

    explicit IImageSys(Allocator allocator, GpuSys gpu_sys, FileSys file_sys,
                       Scheduler scheduler) :
      allocator_{allocator},
      images_{allocator},
      rw_lock_{},
      gpu_sys_{gpu_sys},
      file_sys_{file_sys},
      scheduler_{scheduler}
    {
    }

    IImageSys(IImageSys const &)             = delete;
    IImageSys(IImageSys &&)                  = default;
    IImageSys & operator=(IImageSys const &) = delete;
    IImageSys & operator=(IImageSys &&)      = default;
    ~IImageSys()                             = default;

    void shutdown();

    // Not MT-safe. Main-thread only.
    Result<ImageInfo, SysErr>
      create_image_(Str label, gpu::ImageInfo const & info,
                    Span<gpu::ImageViewInfo const> view_infos);

    // Not MT-safe. Main-thread only.
    Result<ImageInfo, SysErr> upload_(Str label, gpu::ImageInfo const & info,
                                      Span<gpu::ImageViewInfo const> view_infos,
                                      Span<u8 const>                 channels);

    Future<Result<ImageInfo, SysErr>>
      load_from_memory(Str label, gpu::ImageInfo const & info,
                       Span<gpu::ImageViewInfo const> view_infos,
                       RcBlob8                        channels);

    Future<Result<ImageInfo, SysErr>> load_from_path(Str label, Str path);

    Option<ImageInfo> get(Str label);

    ImageInfo get(ImageId id);

    void unload_(ImageId id);

    /// @warning Resources are not reference counted. Holding on to their info
    /// structs after unloading them will be catastrophic
    void unload(ImageId id);
};

}    // namespace ash
