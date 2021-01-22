#pragma once

#include <filesystem>

#include "stx/result.h"
#include "stx/span.h"

namespace vlk {
namespace desc {

struct Image2D {
  enum class Format : uint8_t {
    Internal = 0,
    Grey = 1,
    GreyAlpha = 2,
    RGB = 3,
    RGBA = 4
  };

  std::filesystem::path path;
  Format target_format;
  bool flip_vertically;
};

}  // namespace desc

// data namespace is low-level and less-forgiving of mistakes and intended for
// optimized used cases
namespace data {

enum class Error : uint8_t { InvalidPath, Internal };

// stored in h x w x c memory order.
struct Image2D {
  enum class Format : uint8_t { Grey = 1, GreyAlpha = 2, RGB = 3, RGBA = 4 };

  static stx::Result<Image2D, Error> load(desc::Image2D const& desc);

  Image2D() : pixel_data_{}, width_{}, height_{}, format_{Format::RGBA} {}
  Image2D(Image2D const&) = delete;
  Image2D& operator=(Image2D const&) = delete;
  Image2D(Image2D&& other) noexcept;
  Image2D& operator=(Image2D&& other) noexcept;
  ~Image2D() noexcept;

  uint32_t width() const noexcept { return width_; }
  uint32_t height() const noexcept { return height_; }
  Format format() const noexcept { return format_; }
  uint32_t channels() const noexcept { return static_cast<uint32_t>(format()); }

  size_t size() const noexcept {
    return static_cast<size_t>(width()) * static_cast<size_t>(height()) *
           static_cast<size_t>(format());
  }

  stx::Span<uint8_t const> bytes() const noexcept {
    return stx::Span<uint8_t const>(pixel_data_, pixel_data_ + size());
  }

  bool is_valid() const noexcept { return size() != 0; }

 private:
  uint8_t* pixel_data_;
  uint32_t width_, height_;
  Format format_;
};

}  // namespace data
}  // namespace vlk
