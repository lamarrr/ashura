#pragma once

#include <filesystem>
#include <utility>

#include "include/core/SkImage.h"
#include "stx/result.h"
#include "vlk/ui/asset_manager.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/render_context.h"

namespace vlk {
namespace ui {

enum class ImageFormat : uint8_t { RGB, RGBA, Gray };

namespace impl {

constexpr uint8_t unaligned_channel_size(ImageFormat format) {
  switch (format) {
    case ImageFormat::RGB:
      return 3;
    case ImageFormat::RGBA:
      return 4;
    case ImageFormat::Gray:
      return 1;
    default:
      return 0;
  }
}

}  // namespace impl

enum class ImageLoadError : uint8_t { InvalidPath, LoadFailed };

constexpr std::string_view format(ImageLoadError error) {
  switch (error) {
    case ImageLoadError::InvalidPath:
      return "InvalidPath";
    case ImageLoadError::LoadFailed:
      return "LoadFailed";
    default:
      return "";
  }
}

struct ImageInfo {
  Extent extent;
  ImageFormat format = ImageFormat::RGB;
};

struct ImageAsset : public Asset {
  explicit ImageAsset(stx::Result<sk_sp<SkImage>, ImageLoadError>&& load_result)
      : load_result_{std::move(load_result)} {
    load_result_.as_cref().match(
        [&](sk_sp<SkImage> const& gpu_texture) {
          VLK_ENSURE(gpu_texture != nullptr);
          Asset::update_size(gpu_texture->imageInfo().computeMinByteSize());
        },
        [&](ImageLoadError) { Asset::update_size(0); });
  }

  auto const& get_ref() const { return load_result_; }

 private:
  stx::Result<sk_sp<SkImage>, ImageLoadError> load_result_;
};

namespace impl {
struct FileImageSourceData {
  std::filesystem::path path;
  stx::Option<ImageFormat> target_format = stx::None;
  std::string identifier;
};

struct MemoryImageSourceData {
  ImageInfo info;
  std::vector<uint8_t> buffer;
  std::string identifier;
};

struct FileImageLoadArgs : public AssetLoadArgs {
  explicit FileImageLoadArgs(
      std::shared_ptr<FileImageSourceData const> source_data) {
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
  explicit MemoryImageLoadArgs(
      std::shared_ptr<MemoryImageSourceData const> source_data) {
    VLK_ENSURE(source_data != nullptr);
    data_ = std::move(source_data);
  }

  std::shared_ptr<MemoryImageSourceData const> source_data() const {
    return data_;
  }

 private:
  std::shared_ptr<MemoryImageSourceData const> data_;
};

struct FileImageLoader : public AssetLoader {
  virtual std::unique_ptr<Asset> load(RenderContext const& context,
                                      AssetLoadArgs const& args) const override;

  static std::shared_ptr<AssetLoader const> get_default();
};

struct MemoryImageLoader : public AssetLoader {
  virtual std::unique_ptr<Asset> load(RenderContext const& context,
                                      AssetLoadArgs const& args) const override;

  static std::shared_ptr<AssetLoader const> get_default();
};

}  // namespace impl

// TODO(lamarrr): target size optional
struct FileImageSource {
  FileImageSource(std::filesystem::path path,
                  stx::Option<ImageFormat> target_format = stx::None) {
    std::string identifier = std::string("FileImage{path: ") +
                             std::string(path) + std::string(", format: ") +
                             target_format.clone()
                                 .map(enum_ut<ImageFormat>)
                                 .map([](auto v) { return std::to_string(v); })
                                 .unwrap_or("internal format") +
                             "}";

    data_ = std::make_shared<impl::FileImageSourceData const>(
        impl::FileImageSourceData{std::move(path), target_format,
                                  std::move(identifier)});
  }

  auto data() const { return data_; }

  auto const& data_ref() const { return data_; }

  bool operator==(FileImageSource const& other) const {
    return data_ == other.data_;
  }

  bool operator!=(FileImageSource const& other) const {
    return !(*this == other);
  }

 private:
  std::shared_ptr<impl::FileImageSourceData const> data_;
};

struct MemoryImageSource {
  MemoryImageSource(ImageInfo const& image_info,
                    std::vector<uint8_t>&& image_buffer) {
    VLK_ENSURE(image_info.extent.width * image_info.extent.height *
                   impl::unaligned_channel_size(image_info.format) ==
               image_buffer.size());
    VLK_ENSURE(image_info.extent.visible());

    data_ = std::make_shared<impl::MemoryImageSourceData const>(
        impl::MemoryImageSourceData{image_info, std::move(image_buffer),
                                    std::string("MemoryImage{uid: ") +
                                        std::to_string(make_uid()) + "}"});
  }

  ImageInfo info() const { return data_->info; }

  auto data() const { return data_; }

  auto const& data_ref() const { return data_; }

  bool operator==(MemoryImageSource const& other) const {
    return data_ == other.data_;
  }

  bool operator!=(MemoryImageSource const& other) const {
    return !(*this == other);
  }

 private:
  static uint64_t make_uid();

  // this makes copying this struct relatively cheap as we don't have to copy
  // the buffer or metadata
  std::shared_ptr<impl::MemoryImageSourceData const> data_;
};

inline auto add_asset(AssetManager& asset_manager,
                      FileImageSource const& image_source) {
  return asset_manager.add(
      image_source.data_ref()->identifier,
      std::make_unique<impl::FileImageLoadArgs>(
          impl::FileImageLoadArgs{image_source.data_ref()}),
      impl::FileImageLoader::get_default());
}

inline auto add_asset(AssetManager& asset_manager,
                      MemoryImageSource const& image_source) {
  return asset_manager.add(
      image_source.data_ref()->identifier,
      std::make_unique<impl::MemoryImageLoadArgs>(
          impl::MemoryImageLoadArgs{image_source.data_ref()}),
      impl::MemoryImageLoader::get_default());
}

inline stx::Result<std::shared_ptr<ImageAsset const>, AssetError> get_asset(
    AssetManager& asset_manager, FileImageSource const& image_source) {
  TRY_OK(asset, asset_manager.get(image_source.data_ref()->identifier));
  auto image_asset = std::dynamic_pointer_cast<ImageAsset const>(asset);
  VLK_ENSURE(image_asset != nullptr);
  return stx::Ok(std::move(image_asset));
}

inline stx::Result<std::shared_ptr<ImageAsset const>, AssetError> get_asset(
    AssetManager& asset_manager, MemoryImageSource const& image_source) {
  TRY_OK(asset, asset_manager.get(image_source.data_ref()->identifier));
  auto image_asset = std::dynamic_pointer_cast<ImageAsset const>(asset);
  VLK_ENSURE(image_asset != nullptr);
  return stx::Ok(std::move(image_asset));
}

// the contents of a bundle need to managed individually

}  // namespace ui
}  // namespace vlk
