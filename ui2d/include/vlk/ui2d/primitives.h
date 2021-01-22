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
struct Sizing {
  enum class [[nodiscard]] Type : uint8_t{
      // the part of the target used is a portion of the image specified in
      // pixels
      Relative,
      // the part of the target used is a portion of the image specified within
      // the range of 0.0 to 1.0f and scaled to the target's dimensions
      Absolute};

  explicit constexpr Sizing(Rect const &rect) noexcept
      : type_{Type::Absolute}, rect_data_{rect} {}
  explicit constexpr Sizing(RelativeRect const &rect) noexcept
      : type_{Type::Relative}, relative_data_{rect} {}

  constexpr Sizing() noexcept
      : type_{Type::Relative},
        relative_data_{RelativeRect{{0.0f, 0.0f}, {1.0f, 1.0f}}} {}

  constexpr Sizing(Sizing const &other) = default;
  constexpr Sizing(Sizing &&other) = default;
  constexpr Sizing &operator=(Sizing const &other) = default;
  constexpr Sizing &operator=(Sizing &&other) = default;
  ~Sizing() noexcept = default;

  static constexpr Sizing relative(RelativeRect const &relative) noexcept {
    return Sizing{relative};
  }

  static constexpr Sizing relative(float offset_x, float offset_y, float width,
                                   float height) noexcept {
    return Sizing{RelativeRect{RelativeOffset{offset_x, offset_y},
                               RelativeExtent{width, height}}};
  }

  static constexpr Sizing relative(float width, float height) noexcept {
    return relative(0.0f, 0.0f, width, height);
  }

  static constexpr Sizing relative() noexcept { return relative(1.0f, 1.0f); }

  static constexpr Sizing absolute(Rect const &rect) noexcept {
    return Sizing{rect};
  }

  static constexpr Sizing absolute(uint32_t offset_x, uint32_t offset_y,
                                   uint32_t width, uint32_t height) noexcept {
    return absolute(Rect{Offset{offset_x, offset_y}, Extent{width, height}});
  }

  static constexpr Sizing absolute(uint32_t width, uint32_t height) noexcept {
    return absolute(0, 0, width, height);
  }

  constexpr Type type() const noexcept { return type_; }

  stx::Option<RelativeRect> get_relative() const noexcept {
    if (type_ == Type::Relative) return stx::Some(RelativeRect{relative_data_});
    return stx::None;
  }

  stx::Option<Rect> get_absolute() const noexcept {
    if (type_ == Type::Absolute) return stx::Some(Rect{rect_data_});
    return stx::None;
  }

 private:
  Type type_;
  union {
    RelativeRect relative_data_;
    Rect rect_data_;
  };
};

}  // namespace ui2d
}  // namespace vlk
