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

  auto get_tag() const {
    return AssetTag{
        stx::transmute(std::string_view{data.get()->tag}, data.share())};
  }

  bool operator==(FileImageSource const& other) const {
    return data.get()->tag == other.data.get()->tag;
  }

  bool operator!=(FileImageSource const& other) const {
    return !(*this == other);
  }

  stx ::Rc<FileImageSourceData const*> data;
};

struct MemoryImageSource {
  MemoryImageSource(ImageInfo image_info, std::vector<uint8_t>&& image_buffer);

  ImageInfo get_info() const { return data.get()->info; }

  auto get_tag() const {
    return AssetTag{
        stx::transmute(std::string_view{data.get()->tag}, data.share())};
  }

  bool operator==(MemoryImageSource const& other) const {
    return data.get()->tag == other.data.get()->tag;
  }

  bool operator!=(MemoryImageSource const& other) const {
    return !(*this == other);
  }

  static uint64_t make_uid();

  stx ::Rc<MemoryImageSourceData const*> data;
};

using ImageSource = std::variant<MemoryImageSource, FileImageSource>;

}  // namespace vlk
