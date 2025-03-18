/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/image_decoder.h"
#include "ashura/engine/shader.h"
#include "ashura/engine/window.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/async.h"
#include "ashura/std/fs.h"
#include "ashura/std/vec.h"

namespace ash
{

struct ImageInfo
{
  ImageId id = ImageId::Invalid;

  Str label{};

  Span<TextureId const> textures{};

  gpu::ImageInfo info{};

  Span<gpu::ImageViewInfo const> view_infos{};

  gpu::Image image = nullptr;

  Span<gpu::ImageView const> views{};
};

struct Image
{
  ImageId id = ImageId::Invalid;

  Vec<char> label{};

  Vec<TextureId> textures{};

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

struct ShaderInfo
{
  ShaderId id{};

  Str label{};

  gpu::Shader shader = nullptr;
};

struct Shader
{
  ShaderId id{};

  Vec<char> label{};

  gpu::Shader shader = nullptr;

  constexpr ShaderInfo view() const
  {
    return ShaderInfo{.id = id, .label = label, .shader = shader};
  }
};

struct FileSystem
{
  AllocatorRef allocator_;

  explicit FileSystem(AllocatorRef allocator) : allocator_{allocator}
  {
  }

  FileSystem(FileSystem const &)             = delete;
  FileSystem(FileSystem &&)                  = default;
  FileSystem & operator=(FileSystem const &) = delete;
  FileSystem & operator=(FileSystem &&)      = default;
  ~FileSystem()                              = default;

  void shutdown();

  Future<Result<Vec<u8>, IoErr>> load_file(Str path);
};

struct ImageSystem
{
  gpu::Format           format_ = gpu::Format::B8G8R8A8_UNORM;
  AllocatorRef          allocator_;
  SparseVec<Vec<Image>> images_{};

  explicit ImageSystem(AllocatorRef allocator) :
    allocator_{allocator},
    images_{allocator}
  {
  }

  ImageSystem(ImageSystem const &)             = delete;
  ImageSystem(ImageSystem &&)                  = default;
  ImageSystem & operator=(ImageSystem const &) = delete;
  ImageSystem & operator=(ImageSystem &&)      = default;
  ~ImageSystem()                               = default;

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

  ImageInfo get(Str label);

  ImageInfo get(ImageId id);

  void unload(ImageId id);
};

struct FontSystem
{
  static Dyn<FontSystem *> create(AllocatorRef allocator);

  virtual ~FontSystem() = default;

  virtual void shutdown() = 0;

  /// @brief rasterize the font at the specified font height. Note: raster is
  /// stored as alpha values.
  /// @note rasterizing mutates the font's internal data, not thread-safe
  /// @param font_height the font height at which the texture should be
  /// rasterized at (px)
  /// @param allocator scratch allocator to use for storing intermediates
  virtual Result<> rasterize(Font & font, u32 font_height) = 0;

  virtual void layout_text(TextBlock const & block, f32 max_width,
                           TextLayout & layout) = 0;

  virtual Future<Result<FontId, FontLoadErr>>
    load_from_memory(Vec<char> label, Vec<u8> encoded, u32 font_height,
                     u32 face = 0) = 0;

  virtual Future<Result<FontId, FontLoadErr>> load_from_path(Vec<char> label,
                                                             Str       path,
                                                             u32 font_height,
                                                             u32 face = 0) = 0;

  virtual FontInfo get(FontId id) = 0;

  virtual FontInfo get(Str label) = 0;

  virtual void unload(FontId id) = 0;
};

struct ShaderSystem
{
  AllocatorRef           allocator_;
  SparseVec<Vec<Shader>> shaders_;

  ShaderSystem(AllocatorRef allocator) :
    allocator_{allocator},
    shaders_{allocator}
  {
  }

  ShaderSystem(ShaderSystem const &)             = delete;
  ShaderSystem(ShaderSystem &&)                  = default;
  ShaderSystem & operator=(ShaderSystem const &) = delete;
  ShaderSystem & operator=(ShaderSystem &&)      = default;
  ~ShaderSystem()                                = default;

  void shutdown();

  Result<ShaderInfo, ShaderLoadErr> load_from_memory(Vec<char>       label,
                                                     Span<u32 const> spirv);

  Future<Result<ShaderInfo, ShaderLoadErr>> load_from_path(Vec<char> label,
                                                           Str       path);

  ShaderInfo get(ShaderId id);

  ShaderInfo get(Str label);

  void unload(ShaderId);
};

struct Systems
{
  FileSystem &   file;
  GpuSystem &    gpu;
  ImageSystem &  image;
  FontSystem &   font;
  ShaderSystem & shader;
  WindowSystem & window;
};

extern Systems * sys;

}    // namespace ash
