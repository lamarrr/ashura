#pragma once

#include <cinttypes>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

#include "stx/mem.h"
#include "stx/option.h"
#include "vlk/asset_tag.h"
#include "vlk/primitives.h"

namespace vlk {

enum class ImageFormat : uint8_t { RGB, RGBA, Gray };

struct ImageInfo {
  Extent extent;
  ImageFormat format = ImageFormat::RGB;
};

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

struct FileImageSourceData {
  std::filesystem::path path;
  stx::Option<ImageFormat> target_format = stx::None;
  std::string tag;
};

struct MemoryImageSourceData {
  ImageInfo info;
  std::vector<uint8_t> bytes;
  std::string tag;
};

// TODO(lamarrr): maximum target size optional, or target scale factor defaults
// to 1.0f
struct FileImageSource {
  FileImageSource(std::filesystem::path path,
                  stx::Option<ImageFormat> target_format = stx::None);

  FileImageSource(FileImageSource const& other) : data{other.data.share()} {}
  FileImageSource(FileImageSource&& other) = default;
  FileImageSource& operator=(FileImageSource const& other) {
    data = other.data.share();
    return *this;
  }
  FileImageSource& operator=(FileImageSource&& other) = default;
  ~FileImageSource() = default;

  auto get_tag() const {
    return AssetTag{
        stx::transmute(std::string_view{data.handle->tag}, data.share())};
  }

  bool operator==(FileImageSource const& other) const {
    return data.handle->tag == other.data.handle->tag;
  }

  bool operator!=(FileImageSource const& other) const {
    return !(*this == other);
  }

  stx::Rc<FileImageSourceData const*> data;
};

struct MemoryImageSource {
  MemoryImageSource(ImageInfo image_info, std::vector<uint8_t>&& image_buffer);

  MemoryImageSource(MemoryImageSource const& other)
      : data{other.data.share()} {}
  MemoryImageSource(MemoryImageSource&& other) = default;
  MemoryImageSource& operator=(MemoryImageSource const& other) {
    data = other.data.share();
    return *this;
  }
  MemoryImageSource& operator=(MemoryImageSource&& other) = default;
  ~MemoryImageSource() = default;

  ImageInfo get_info() const { return data.handle->info; }

  auto get_tag() const {
    return AssetTag{
        stx::transmute(std::string_view{data.handle->tag}, data.share())};
  }

  bool operator==(MemoryImageSource const& other) const {
    return data.handle->tag == other.data.handle->tag;
  }

  bool operator!=(MemoryImageSource const& other) const {
    return !(*this == other);
  }

  static uint64_t make_uid();

  stx::Rc<MemoryImageSourceData const*> data;
};

using ImageSource = std::variant<MemoryImageSource, FileImageSource>;

}  // namespace vlk
