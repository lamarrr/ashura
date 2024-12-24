/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/image_decoder.h"
#include "ashura/engine/shader.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/async.h"
#include "ashura/std/fs.h"
#include "ashura/std/vec.h"

namespace ash
{

struct Image
{
  ImageId id{};

  Span<char const> label{};

  TextureId texture{};

  gpu::ImageInfo image_info = {};

  gpu::ImageViewInfo image_view_info = {};

  gpu::Image image = nullptr;

  gpu::ImageView image_view = nullptr;
};

struct Shader
{
  ShaderId id{};

  Span<char const> label{};

  gpu::Shader shader = nullptr;
};

struct FileSystem
{
  FileSystem()                               = default;
  FileSystem(FileSystem const &)             = delete;
  FileSystem(FileSystem &&)                  = delete;
  FileSystem & operator=(FileSystem const &) = delete;
  FileSystem & operator=(FileSystem &&)      = delete;
  ~FileSystem()                              = default;


  void init(Scheduler &);

  void shutdown();

  static Future<Result<Vec<u8>, IoErr>>
    load_file(Span<char const> path,
              AllocatorImpl    allocator = default_allocator);

};

struct ImageSystem
{
  static constexpr gpu::Format format_ = gpu::Format::B8G8R8A8_UNORM;

  SparseVec<Vec<Image>> images_{};

  ImageSystem()                                = default;
  ImageSystem(ImageSystem const &)             = delete;
  ImageSystem(ImageSystem &&)                  = delete;
  ImageSystem & operator=(ImageSystem const &) = delete;
  ImageSystem & operator=(ImageSystem &&)      = delete;
  ~ImageSystem()                               = default;

  void init();
  
  void shutdown();

  Image create_image_(Span<char const> label, gpu::ImageInfo const & info,
                      gpu::ImageViewInfo const & view_info);

  Image upload(gpu::ImageInfo const & info, Span<u8 const> channels);

  Result<Image, ImageLoadErr> load_from_memory(Span<char const> label,
                                               Vec2U extent, gpu::Format format,
                                               Span<u8 const> buffer);

  Future<Result<Image, ImageLoadErr>>
    load_from_path(Span<char const> label, Span<char const> path,
                   AllocatorImpl allocator = default_allocator);

  Image get(Span<char const> label);

  Image get(ImageId id);

  void unload(ImageId id);
};

struct FontSystem
{
  SparseVec<Vec<Dyn<Font *>>> fonts_;

  FontSystem()                               = default;
  FontSystem(FontSystem const &)             = delete;
  FontSystem(FontSystem &&)                  = delete;
  FontSystem & operator=(FontSystem const &) = delete;
  FontSystem & operator=(FontSystem &&)      = delete;
  ~FontSystem()                              = default;

  void init();

  void shutdown();

  Result<Dyn<Font *>, FontLoadErr>
    decode_(Span<char const> label, Span<u8 const> encoded, u32 face = 0);

  /// @brief rasterize the font at the specified font height. Note: raster is
  /// stored as alpha values.
  /// @note rasterizing mutates the font's internal data, not thread-safe
  /// @param font_height the font height at which the texture should be
  /// rasterized at (px)
  /// @param allocator scratch allocator to use for storing intermediates
  Result<> rasterize(Font & font, u32 font_height);

  FontId upload_(Dyn<Font *> font);

  Future<Result<FontId, FontLoadErr>> load_from_memory(Span<char const> label,
                                                       Vec<u8>          encoded,
                                                       u32 font_height,
                                                       u32 face = 0);

  Future<Result<FontId, FontLoadErr>> load_from_path(Span<char const> label,
                                                     Span<char const> path,
                                                     u32 font_height,
                                                     u32 face = 0);

  Font & get(FontId id);

  Font & get(Span<char const> label);

  void unload(FontId id);
};

struct ShaderSystem
{
  SparseVec<Vec<Shader>> shaders_;

  ShaderSystem()                                 = default;
  ShaderSystem(ShaderSystem const &)             = delete;
  ShaderSystem(ShaderSystem &&)                  = delete;
  ShaderSystem & operator=(ShaderSystem const &) = delete;
  ShaderSystem & operator=(ShaderSystem &&)      = delete;
  ~ShaderSystem()                                = default;

  void init();
  
  void shutdown();

  Result<Shader, ShaderLoadErr> load_spirv_from_memory(Span<char const> label,
                                                       Span<u32 const>  spirv);

  Future<Result<Shader, ShaderLoadErr>>
    load_spirv_from_path(Span<char const> label, Span<char const> path);

  Shader get(ShaderId id);

  Shader get(Span<char const> label);

  void unload(ShaderId);
};

struct Systems
{
  FileSystem   file;
  GpuSystem    gpu;
  ImageSystem  image;
  FontSystem   font;
  ShaderSystem shader;

  void init()
  {
    file.init(*scheduler);
    // [ ] gpu.init()
    image.init();
    font.init();
    shader.init();
  }

  void shutdown()
  {
    shader.shutdown();
    font.shutdown();
    image.shutdown();
    // [ ] gpu.shutdown();
    file.shutdown();
  }
};

extern Systems * sys;

}    // namespace ash
