#pragma once

#include <filesystem>
#include <utility>

#include "fmt/format.h"
#include "include/core/SkImage.h"
#include "stx/result.h"
#include "vlk/ui/asset_manager.h"
#include "vlk/primitives.h"

namespace vlk {
namespace ui {

enum class ImageFormat : uint8_t { RGB, RGBA, Gray };

constexpr std::string_view format(ImageFormat image_format) {
  switch (image_format) {
    case ImageFormat::Gray:
      return "Gray";
    case ImageFormat::RGB:
      return "RGB";
    case ImageFormat::RGBA:
      return "RGBA";
      default:
      return "";
  }
}

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
        [&](sk_sp<SkImage> const& image) {
          VLK_ENSURE(image != nullptr);
          Asset::update_size(image->imageInfo().computeMinByteSize());
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
  std::string tag;
};

struct MemoryImageSourceData {
  ImageInfo info;
  std::vector<uint8_t> buffer;
  std::string tag;
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
  virtual std::unique_ptr<Asset> load(AssetLoadArgs const& args) const override;

  static std::shared_ptr<AssetLoader const> get_default();
};

struct MemoryImageLoader : public AssetLoader {
  virtual std::unique_ptr<Asset> load(AssetLoadArgs const& args) const override;

  static std::shared_ptr<AssetLoader const> get_default();
};

}  // namespace impl

// TODO(lamarrr): maximum target size optional, or target scale factor defaults
// to 1.0f
struct FileImageSource {
  FileImageSource(std::filesystem::path path,
                  stx::Option<ImageFormat> target_format = stx::None) {
    auto format_str = target_format.clone()
                          .map([](ImageFormat value) { return format(value); })
                          .unwrap_or("internal format");

    data_ = std::shared_ptr<impl::FileImageSourceData const>{
        new impl::FileImageSourceData{
            path, target_format,
            fmt::format("Bultin.FileImage(path: {}, format: {})", path.c_str(),
                        std::move(format_str))}};
  }

  auto data() const { return data_; }

  auto const& data_ref() const { return data_; }

  AssetTag get_tag() const { return AssetTag::from_shared(data_, data_->tag); }

  bool operator==(FileImageSource const& other) const {
    return data_->tag == other.data_->tag;
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
        impl::MemoryImageSourceData{
            image_info, std::move(image_buffer),
            fmt::format("Bultin.MemoryImage(uid: {})", make_uid())});
  }

  ImageInfo info() const { return data_->info; }

  auto data() const { return data_; }

  auto const& data_ref() const { return data_; }

  AssetTag get_tag() const { return AssetTag::from_shared(data_, data_->tag); }

  bool operator==(MemoryImageSource const& other) const {
    return data_->tag == other.data_->tag;
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
      image_source.get_tag(),
      std::make_unique<impl::FileImageLoadArgs>(
          impl::FileImageLoadArgs{image_source.data_ref()}),
      impl::FileImageLoader::get_default());
}

inline auto add_asset(AssetManager& asset_manager,
                      MemoryImageSource const& image_source) {
  return asset_manager.add(
      image_source.get_tag(),
      std::make_unique<impl::MemoryImageLoadArgs>(
          impl::MemoryImageLoadArgs{image_source.data_ref()}),
      impl::MemoryImageLoader::get_default());
}

inline stx::Result<std::shared_ptr<ImageAsset const>, AssetError> get_asset(
    AssetManager& asset_manager, FileImageSource const& image_source) {
  TRY_OK(asset, asset_manager.get(image_source.get_tag()));
  auto image_asset = std::dynamic_pointer_cast<ImageAsset const>(asset);
  VLK_ENSURE(image_asset != nullptr);
  return stx::Ok(std::move(image_asset));
}

inline stx::Result<std::shared_ptr<ImageAsset const>, AssetError> get_asset(
    AssetManager& asset_manager, MemoryImageSource const& image_source) {
  TRY_OK(asset, asset_manager.get(image_source.get_tag()));
  auto image_asset = std::dynamic_pointer_cast<ImageAsset const>(asset);
  VLK_ENSURE(image_asset != nullptr);
  return stx::Ok(std::move(image_asset));
}

// the contents of a bundle need to managed individually

}  // namespace ui
}  // namespace vlk
