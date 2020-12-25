#pragma once
#include <filesystem>
#include <fstream>
#include <utility>
#include <vector>

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// clang-format on

#include "stx/result.h"

namespace vlk {

namespace fs = std::filesystem;

// namespace graph?
namespace desc {

struct Image {
  enum class Format : uint8_t {
    Internal = 0,
    Grey = 1,
    GreyAlpha = 2,
    RGB = 3,
    RGBA = 4
  };

  fs::path path;
  Format target_format;
  bool flip_vertically;
};

}  // namespace desc

// namespace emgine?
namespace data {
// data namespace is low-level and less-forgiving of mistakes and intended for
// optimized used cases

enum class Error : uint8_t { InvalidPath };
// stored in h x w x c memory order.
struct Image {
  enum class Format : uint8_t { Grey = 1, GreyAlpha = 2, RGB = 3, RGBA = 4 };

  static stx::Result<Image, Error> load(desc::Image const& desc) {
    if (!fs::exists(desc.path)) return stx::Err(Error::InvalidPath);

    std::ifstream stream(desc.path, std::ios_base::ate | std::ios_base::binary);

    auto size = stream.tellg();

    std::vector<char> buffer;
    buffer.resize(size);

    stream.seekg(0);
    stream.read(buffer.data(), buffer.size());

    int width, height, inbuilt_channels;

    int target_channels = 0;

    auto const format = desc.target_format;

    switch (format) {
      case desc::Image::Format::Internal:
        target_channels = 0;
        break;
      case desc::Image::Format::Grey:
        target_channels = 1;
        break;
      case desc::Image::Format::GreyAlpha:
        target_channels = 2;
        break;
      case desc::Image::Format::RGB:
        target_channels = 3;
        break;
      case desc::Image::Format::RGBA:
        target_channels = 4;
        break;
      default:
        VLK_ENSURE(false, "Unrecognized Image Format");
    }

    stbi_set_flip_vertically_on_load(desc.flip_vertically);

    auto* image_uc = stbi_load_from_memory(
        reinterpret_cast<stbi_uc const*>(buffer.data()), buffer.size(), &width,
        &height, &inbuilt_channels, target_channels);

    VLK_ENSURE(image_uc != nullptr, "Error occured while trying to load image");

    int resulting_channels = 0;
    if (target_channels == 0) {
      resulting_channels = inbuilt_channels;
    } else {
      resulting_channels = target_channels;
    }

    Image image{};
    image.pixel_data_ = image_uc;
    image.format_ = Format(resulting_channels);
    image.width_ = width;
    image.height_ = height;

    return stx::Ok(std::move(image));
  }

  Image(Image const&) = delete;
  Image& operator=(Image const&) = delete;
  Image(Image&& other) noexcept
      : pixel_data_{other.pixel_data_},
        width_{other.width_},
        height_{other.height_},
        format_{other.format_} {
    other.pixel_data_ = nullptr;
    other.width_ = 0;
    other.height_ = 0;
  }

  Image& operator=(Image&& other) noexcept {
    auto deleter = std::move(*this);

    format_ = other.format_;
    width_ = other.width_;
    height_ = other.height_;
    pixel_data_ = other.pixel_data_;

    other.width_ = 0;
    other.height_ = 0;
    other.pixel_data_ = nullptr;

    return *this;
  }

  ~Image() {
    // the image is allocated by stb library using malloc
    if (pixel_data_ != nullptr) free(pixel_data_);
  }

  auto width() const noexcept { return width_; }
  auto height() const noexcept { return height_; }
  auto size() const noexcept {
    return width_ * height_ * static_cast<size_t>(format_);
  }
  auto format() const noexcept { return format_; }

  stx::Span<uint8_t const> bytes() const noexcept {
    return stx::Span<uint8_t const>(pixel_data_, pixel_data_ + size());
  }

 private:
  Image() {}
  uint8_t* pixel_data_;
  size_t width_, height_;
  Format format_;
};

}  // namespace data
}  // namespace vlk
