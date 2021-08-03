#pragma once

#include "vlk/asset.h"

/*
#include <filesystem>
#include <utility>

#include "fmt/format.h"
#include "include/core/SkImage.h"
#include "stx/result.h"
#include "vlk/primitives.h"
#include "vlk/ui/asset_manager.h"
*/

#include "include/core/SkRefCnt.h"

struct SkImage;

namespace vlk {

struct ImageAsset : public Asset {
  explicit ImageAsset(sk_sp<SkImage> raw_image);

  auto get_raw() const { return raw_; }

  ~ImageAsset() final {}

 private:
  sk_sp<SkImage> raw_;
};

// namespace ui {

/*
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

*/

/*
namespace impl {
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
*/
// }  // namespace ui
}  // namespace vlk
