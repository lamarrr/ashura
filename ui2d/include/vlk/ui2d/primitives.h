#pragma once

#include <cstdint>

namespace vlk {
namespace ui2d {

// type marker
template <typename T>
using Normalized = T;  // normalized range [0.0f, 1.0f], i.e. for depth buffer
                       // where we are not exposing the depth bit

struct Offset {
  uint32_t x;
  uint32_t y;

  constexpr Offset operator+(Offset const &other) const noexcept {
    return Offset{x + other.x, y + other.y};
  }
};

struct RelativeOffset {
  Normalized<float> x;
  Normalized<float> y;
};

struct Extent {
  uint32_t width;
  uint32_t height;
};

struct RelativeExtent {
  Normalized<float> width;
  Normalized<float> height;
};

struct Rect {
  Offset offset;
  Extent extent;
};

struct RelativeRect {
  RelativeOffset offset;
  RelativeExtent extent;
};

struct Color {
  static constexpr uint32_t kRedMask = 0xFF000000U;
  static constexpr uint32_t kGreenMask = kRedMask >> 8;
  static constexpr uint32_t kBlueMask = kGreenMask >> 8;
  static constexpr uint32_t kAlphaMask = kBlueMask >> 8;

  uint32_t rgba;

  static constexpr Color Rgba(uint8_t r, uint8_t g, uint8_t b,
                              uint8_t a) noexcept {
    return Color{static_cast<uint32_t>(r) << 24 |
                 static_cast<uint32_t>(g) << 16 |
                 static_cast<uint32_t>(b) << 8 | a};
  }

  static constexpr Color Rgb(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return Color::Rgba(r, g, b, kAlphaMask);
  }

  constexpr Color with_red(uint8_t r) const noexcept {
    return Color{(rgba & ~kRedMask) | r};
  }

  constexpr Color with_green(uint8_t g) const noexcept {
    return Color{(rgba & ~kGreenMask) | g};
  }

  constexpr Color with_blue(uint8_t b) const noexcept {
    return Color{(rgba & ~kBlueMask) | b};
  }

  constexpr Color with_alpha(uint8_t a) const noexcept {
    return Color{(rgba & ~kAlphaMask) | a};
  }

  constexpr bool operator==(Color const &other) const noexcept {
    return rgba & other.rgba;
  }

  constexpr bool operator!=(Color const &other) const noexcept {
    return !(*this == other);
  }

  constexpr uint32_t argb() const noexcept {
    return (rgba >> 8) | (rgba << 24);
  }
};

namespace colors {
constexpr auto Transparent = Color::Rgba(0x00, 0x00, 0x00, 0x00);
constexpr auto White = Color::Rgb(0xFF, 0xFF, 0xFF);
constexpr auto Black = Color::Rgb(0x00, 0x00, 0x00);
constexpr auto Red = Color::Rgb(0xFF, 0x00, 0x00);
constexpr auto Blue = Color::Rgb(0x00, 0x00, 0xFF);
constexpr auto Green = Color::Rgb(0x00, 0xFF, 0x00);
constexpr auto Cyan = Color::Rgb(0x00, 0xFF, 0xFF);
constexpr auto Magenta = Color::Rgb(0xFF, 0x00, 0xFF);
}  // namespace colors

struct TopRightBottomLeft {
  uint32_t top = 0, right = 0, bottom = 0, left = 0;

  static constexpr TopRightBottomLeft uniform(uint32_t value) noexcept {
    return TopRightBottomLeft{value, value, value, value};
  }

  static constexpr TopRightBottomLeft xy(uint32_t x, uint32_t y) noexcept {
    return TopRightBottomLeft{y, x, y, x};
  }

  static constexpr TopRightBottomLeft trbl(uint32_t t, uint32_t r, uint32_t b,
                                           uint32_t l) noexcept {
    return TopRightBottomLeft{t, r, b, l};
  }
};

}  // namespace ui2d
}  // namespace vlk
