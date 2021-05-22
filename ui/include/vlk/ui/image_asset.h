#pragma once

#include <filesystem>
#include <variant>

#include "stx/span.h"
#include "vlk/ui/asset_manager.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/raster_context.h"

namespace vlk {
namespace ui {

enum class ImageFormat : uint8_t { RGB, RGBA, Grey };

enum class ImageLoadError : uint8_t { InvalidPath, LoadFailed };

namespace impl {

constexpr uint8_t unaligned_channel_size(ImageFormat format) {
  switch (format) {
    case ImageFormat::RGB:
      return 3;
    case ImageFormat::RGBA:
      return 4;
    case ImageFormat::Grey:
      return 1;
  }
}

}  // namespace impl

struct ImageInfo {
  Extent extent;
  ImageFormat format = ImageFormat::RGB;

  constexpr uint32_t channels() const {
    return impl::unaligned_channel_size(format);
  }

  constexpr uint32_t min_size() const {
    return extent.height * extent.width * channels();
  }
};

struct ImageAsset : public Asset {
  ImageAsset(stx::Result<sk_sp<SkSurface>, ImageLoadError>&& load_result)
      : load_result_{std::move(load_result)} {
    load_result_.as_cref().match(
        [&](sk_sp<SkSurface> const& gpu_texture) {
          VLK_ENSURE(gpu_texture != nullptr);
          Asset::update_size(gpu_texture->imageInfo().computeMinByteSize());
        },
        [&](ImageLoadError) { Asset::update_size(0); });
  }

  auto const& get_ref() const { return load_result_; }

 private:
  stx::Result<sk_sp<SkSurface>, ImageLoadError> load_result_;
};

namespace impl {

static constexpr SkColorType to_skia(ImageFormat format) {
  switch (format) {
    case ImageFormat::RGB:
      return SkColorType::kRGB_888x_SkColorType;
    case ImageFormat::RGBA:
      return SkColorType::kRGBA_8888_SkColorType;
    case ImageFormat::Grey:
      return SkColorType::kGray_8_SkColorType;
  }
}

static inline SkImageInfo to_skia(ImageInfo const& info) {
  return SkImageInfo::Make(info.extent.width, info.extent.height,
                           to_skia(info.format),
                           SkAlphaType::kUnpremul_SkAlphaType);
}

constexpr uint8_t aligned_channel_size(ImageFormat format) {
  switch (format) {
    case ImageFormat::RGB:
      return 4;
    case ImageFormat::RGBA:
      return 4;
    case ImageFormat::Grey:
      return 1;
  }
}

struct StbiImageBuffer {
  explicit StbiImageBuffer() {}

  StbiImageBuffer(u_char* stbi_buffer, ImageInfo image_info)
      : stbi_image_buffer_{stbi_buffer}, info_{image_info} {
    VLK_ENSURE(stbi_buffer != nullptr);
  }

  stx::Span<uint8_t const> span() const {
    return stx::Span<u_char const>(stbi_image_buffer_, info_.min_size())
        .as_u8();
  }

  static auto load_from_file(std::filesystem::path const& path,
                             ImageFormat target_format)
      -> stx::Result<StbiImageBuffer, ImageLoadError>;

  StbiImageBuffer(StbiImageBuffer const&) = delete;
  StbiImageBuffer& operator=(StbiImageBuffer const&) = delete;

  StbiImageBuffer(StbiImageBuffer&& other) {
    stbi_image_buffer_ = other.stbi_image_buffer_;
    other.stbi_image_buffer_ = nullptr;
    info_ = other.info_;
    other.info_ = ImageInfo{};
  }

  StbiImageBuffer& operator=(StbiImageBuffer&& other) {
    std::swap(stbi_image_buffer_, other.stbi_image_buffer_);
    std::swap(info_, other.info_);
    return *this;
  }

  ~StbiImageBuffer();

  bool is_valid() const {
    return info_.min_size() != 0 && stbi_image_buffer_ != nullptr;
  }

  ImageInfo info() const { return info_; }

 private:
  u_char* stbi_image_buffer_ = nullptr;
  ImageInfo info_;
};

inline std::unique_ptr<uint32_t[]> make_aligned_RGBx_buffer(
    uint8_t const* buffer, uint32_t width, uint32_t height) {
  std::unique_ptr<uint32_t[]> aligned_memory(new uint32_t[height * width]);

  for (uint64_t i = 0, j = 0; i < height * width; i++, j += 3) {
    // j is not correct yet?
    uint32_t const r = buffer[j];
    uint32_t const g = buffer[j + 1];
    uint32_t const b = buffer[j + 2];
    aligned_memory[i] = (r << 24) | (g << 16) | (b << 8) | 0xFF;
  }
  return aligned_memory;
}

inline std::unique_ptr<uint32_t[]> make_aligned_RGBA_buffer(
    uint8_t const* buffer, uint32_t width, uint32_t height) {
  std::unique_ptr<uint32_t[]> aligned_memory(new uint32_t[height * width]);

  for (uint64_t i = 0, j = 0; i < height * width; i++, j += 4) {
    uint32_t const r = buffer[j];
    uint32_t const g = buffer[j + 1];
    uint32_t const b = buffer[j + 2];
    uint32_t const a = buffer[j + 3];
    aligned_memory[i] = (r << 24) | (g << 16) | (b << 8) | a;
  }
  return aligned_memory;
}

// bufffer memory is aligned for optimal use
// in the skia API (8-bit alignment for single channel images i.e. Grey. and
// 32-bit alignment for multi-channel images i.e. RGB and RGBA).
// a new buffer establishes alignment requirement for the image if necessary
inline sk_sp<SkSurface> dispatch_image_to_gpu(RasterContext const& context,
                                              ImageInfo const& info,
                                              uint8_t const* unaligned_pixels) {
  sk_sp texture = context.create_target_texture(
      info.extent, to_skia(info.format), kUnpremul_SkAlphaType);

  switch (info.format) {
    case ImageFormat::Grey: {
      texture->writePixels(
          SkPixmap(to_skia(info), unaligned_pixels, info.extent.width * 1), 0,
          0);
    } break;
    case ImageFormat::RGB: {
      std::unique_ptr<uint32_t[]> aligned_buffer = make_aligned_RGBx_buffer(
          unaligned_pixels, info.extent.width, info.extent.height);
      texture->writePixels(
          SkPixmap(to_skia(info), aligned_buffer.get(), info.extent.width * 4),
          0, 0);
    } break;
    case ImageFormat::RGBA: {
      std::unique_ptr<uint32_t[]> aligned_buffer = make_aligned_RGBA_buffer(
          unaligned_pixels, info.extent.width, info.extent.height);
      texture->writePixels(
          SkPixmap(to_skia(info), aligned_buffer.get(), info.extent.width * 4),
          0, 0);
    } break;
    default:
      VLK_PANIC("Unsupported Image Format");
  }

  return texture;
}

// remove pixmap and replace with this function instead
inline sk_sp<SkSurface> dispatch_image_to_gpu(
    RasterContext const& context, StbiImageBuffer const& unaligned_buffer) {
  return dispatch_image_to_gpu(context, unaligned_buffer.info(),
                               unaligned_buffer.span().data());
}

struct FileImageSourceData {
  std::filesystem::path path;
  ImageFormat target_format = ImageFormat::RGB;
  std::string identifier;
};

struct MemoryImageSourceData {
  ImageInfo info;
  std::vector<uint8_t> buffer;
  std::string identifier;
};

struct FileImageLoadArgs : public AssetLoadArgs {
  FileImageLoadArgs(std::shared_ptr<FileImageSourceData const>&& source_data) {
    VLK_ENSURE(source_data != nullptr);
    data_ = std::move(source_data);
  }

  std::shared_ptr<FileImageSourceData const> source_data() const {
    return data_;
  }

 private:
  std::shared_ptr<FileImageSourceData const> data_;
};

struct MemoryImageLoadArgs : public AssetLoadArgs {
  MemoryImageLoadArgs(
      std::shared_ptr<MemoryImageSourceData const>&& source_data) {
    VLK_ENSURE(source_data != nullptr);
    data_ = std::move(source_data);
  }

  std::shared_ptr<MemoryImageSourceData const> source_data() const {
    return data_;
  }

 private:
  std::shared_ptr<MemoryImageSourceData const> data_;
};

// TODO(lamarrr): for MemoryImageLoadArgs we need a shared_ptr of the actual
// memory? or do we need to move. note that the image is moved to the gpu

struct FileImageLoader : public AssetLoader {
  virtual std::unique_ptr<Asset> load(
      RasterContext const& context, AssetLoadArgs const& args) const override {
    auto const& load_args = upcast<FileImageLoadArgs const&>(args);

    std::shared_ptr source_data = load_args.source_data();

    stx::Result load_result = impl::StbiImageBuffer::load_from_file(
        source_data->path, source_data->target_format);

    if (load_result.is_err())
      return std::make_unique<ImageAsset>(
          ImageAsset{stx::Err(std::move(load_result).unwrap_err())});

    sk_sp surface = dispatch_image_to_gpu(context, load_result.value());

    return std::make_unique<ImageAsset>(
        ImageAsset{stx::Ok(std::move(surface))});
  }

  static std::shared_ptr<AssetLoader const> get_default();
};

struct MemoryImageLoader : public AssetLoader {
  virtual std::unique_ptr<Asset> load(
      RasterContext const& context, AssetLoadArgs const& args) const override {
    auto const& load_args = upcast<MemoryImageLoadArgs const&>(args);

    std::shared_ptr source_data = load_args.source_data();

    sk_sp surface = dispatch_image_to_gpu(context, source_data->info,
                                          source_data->buffer.data());

    return std::make_unique<ImageAsset>(
        ImageAsset{stx::Ok(std::move(surface))});
  }

  static std::shared_ptr<AssetLoader const> get_default();
};

};  // namespace impl

// NOTE: we need these types to be cheap to copy as much as possible
// TODO(lamarrr): have global aliases

struct FileImageSource {
  FileImageSource(std::filesystem::path path, ImageFormat target_format) {
    std::string identifier = std::string("FileImage:") + std::string(path);
    data_ = std::make_shared<impl::FileImageSourceData const>(
        impl::FileImageSourceData{std::move(path), target_format,
                                  std::move(identifier)});
  }

  std::filesystem::path path() const { return data_->path; }

  ImageFormat target_format() const { return data_->target_format; }

  std::string_view identifier_ref() const { return data_->identifier; }

  std::string identifier() const { return data_->identifier; }

  std::shared_ptr<impl::FileImageSourceData const> data() const {
    return data_;
  }

 private:
  std::shared_ptr<impl::FileImageSourceData const> data_;
};

struct MemoryImageSource {
  MemoryImageSource(ImageInfo const& image_info,
                    std::vector<uint8_t>&& image_buffer) {
    VLK_ENSURE(image_info.min_size() == image_buffer.size());
    VLK_ENSURE(image_info.extent.visible());

    data_ = std::make_shared<impl::MemoryImageSourceData const>(
        impl::MemoryImageSourceData{
            image_info, std::move(image_buffer),
            std::string("MemoryImage:") + std::to_string(make_uid())});
  }

  ImageInfo info() const { return data_->info; }

  stx::Span<uint8_t const> buffer_ref() const { return data_->buffer; }

  std::string_view identifier_ref() const { return data_->identifier; }

  std::string identifier() const { return data_->identifier; }

  std::shared_ptr<impl::MemoryImageSourceData const> data() const {
    return data_;
  }

 private:
  static uint64_t make_uid();

  // this makes copying this struct relatively cheap as we don't have to copy
  // the buffer or metadata
  std::shared_ptr<impl::MemoryImageSourceData const> data_;
};

// TODO(lamarrr): requires_persistence?
inline auto add_asset(AssetManager& asset_manager,
                      FileImageSource const& image_source) {
  return asset_manager.add(image_source.identifier_ref(),
                           std::make_unique<impl::FileImageLoadArgs>(
                               impl::FileImageLoadArgs{image_source.data()}),
                           impl::FileImageLoader::get_default());
}

inline auto add_asset(AssetManager& asset_manager,
                      MemoryImageSource const& image_source) {
  return asset_manager.add(image_source.identifier_ref(),
                           std::make_unique<impl::MemoryImageLoadArgs>(
                               impl::MemoryImageLoadArgs{image_source.data()}),
                           impl::MemoryImageLoader::get_default());
}

inline stx::Result<std::shared_ptr<ImageAsset const>, AssetError> get_asset(
    AssetManager& asset_manager, FileImageSource const& image_source) {
  std::shared_ptr data = image_source.data();
  TRY_OK(asset, asset_manager.get(data->identifier));
  auto image_asset = std::dynamic_pointer_cast<ImageAsset const>(asset);
  VLK_ENSURE(image_asset != nullptr);

  return stx::Ok(std::move(image_asset));
}

inline stx::Result<std::shared_ptr<ImageAsset const>, AssetError> get_asset(
    AssetManager& asset_manager, MemoryImageSource const& image_source) {
  std::shared_ptr data = image_source.data();
  TRY_OK(asset, asset_manager.get(data->identifier));
  auto image_asset = std::dynamic_pointer_cast<ImageAsset const>(asset);
  VLK_ENSURE(image_asset != nullptr);
  return stx::Ok(std::move(image_asset));
}

// the contents of a bundle need to managed individually

}  // namespace ui
}  // namespace vlk