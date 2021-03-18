#pragma once

#include <cstdint>
#include "stx/option.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

struct [[nodiscard]] Ticks {
  Ticks(uint64_t value = 0) : value_{value} {}

  [[nodiscard]] inline constexpr bool operator==(Ticks const &other)
      const noexcept {
    return value_ == other.value_;
  }

  [[nodiscard]] inline constexpr bool operator!=(Ticks const &other)
      const noexcept {
    return !(*this == other);
  }

  inline constexpr void operator++() noexcept { value_++; }

  [[nodiscard]] inline constexpr bool operator<(Ticks const &other)
      const noexcept {
    return value_ < other.value_;
  }

  [[nodiscard]] inline constexpr bool operator>(Ticks const &other)
      const noexcept {
    return value_ > other.value_;
  }

  [[nodiscard]] inline constexpr bool operator<=(Ticks const &other)
      const noexcept {
    return value_ <= other.value_;
  }

  [[nodiscard]] inline constexpr bool operator>=(Ticks const &other)
      const noexcept {
    return value_ >= other.value_;
  }

  [[nodiscard]] uint64_t value() const noexcept { return value_; }

 private:
  uint64_t value_ = 0;
};

// type marker
template <typename T>
using Normalized = T;  // normalized range [0.0f, 1.0f], i.e. for depth buffer
                       // where we are not exposing the depth bit

struct [[nodiscard]] Offset {
  uint32_t x;
  uint32_t y;
};

[[nodiscard]] inline constexpr Offset operator+(Offset const &a,
                                                Offset const &b) noexcept {
  return Offset{a.x + b.x, a.y + b.y};
}

[[nodiscard]] inline constexpr bool operator==(Offset const &a,
                                               Offset const &b) noexcept {
  return a.x == b.x && a.y == b.y;
}

[[nodiscard]] inline constexpr bool operator!=(Offset const &a,
                                               Offset const &b) noexcept {
  return !(a == b);
}

struct [[nodiscard]] RelativeOffset {
  Normalized<float> x;
  Normalized<float> y;
};

struct [[nodiscard]] Extent {
  uint32_t width;
  uint32_t height;
};

[[nodiscard]] inline constexpr bool operator==(Extent const &a,
                                               Extent const &b) noexcept {
  return a.width == b.width && a.height == b.height;
}

[[nodiscard]] inline constexpr bool operator!=(Extent const &a,
                                               Extent const &b) noexcept {
  return !(a == b);
}

struct [[nodiscard]] RelativeExtent {
  Normalized<float> width;
  Normalized<float> height;
};

struct [[nodiscard]] Rect {
  Offset offset;
  Extent extent;

  constexpr bool overlaps(Rect const &other) noexcept {
    uint64_t x1_min = offset.x;
    uint64_t x1_max = x1_min + extent.width;
    uint64_t y1_min = offset.y;
    uint64_t y1_max = y1_min + extent.height;
    uint64_t x2_min = other.offset.x;
    uint64_t x2_max = x2_min + other.extent.width;
    uint64_t y2_min = other.offset.y;
    uint64_t y2_max = y2_min + other.extent.height;

    return (x1_max > x2_min && x2_max > x1_min) &&
           (y1_max > y2_min && y2_max > y1_min);
  }
};

[[nodiscard]] inline constexpr bool operator==(Rect const &a,
                                               Rect const &b) noexcept {
  return a.offset == b.offset && a.extent == b.extent;
}

[[nodiscard]] inline constexpr bool operator!=(Rect const &a,
                                               Rect const &b) noexcept {
  return !(a == b);
}

struct [[nodiscard]] RelativeRect {
  RelativeOffset offset;
  RelativeExtent extent;
};

// TODO(lamarrr): fix id casing here
struct [[nodiscard]] Color {
  static constexpr uint32_t kRedMask = 0xFF000000U;
  static constexpr uint32_t kGreenMask = kRedMask >> 8;
  static constexpr uint32_t kBlueMask = kGreenMask >> 8;
  static constexpr uint32_t kAlphaMask = kBlueMask >> 8;

  uint32_t rgba;

  static constexpr Color from_rgba(uint8_t r, uint8_t g, uint8_t b,
                                   uint8_t a) noexcept {
    return Color{static_cast<uint32_t>(r) << 24 |
                 static_cast<uint32_t>(g) << 16 |
                 static_cast<uint32_t>(b) << 8 | a};
  }

  static constexpr Color from_argb(uint32_t argb) noexcept {
    return Color{(argb << 8) | (argb >> 24)};
  }

  static constexpr Color from_rgb(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return Color::from_rgba(r, g, b, 0xFF);
  }

  [[nodiscard]] constexpr uint32_t to_argb() const noexcept {
    return (rgba >> 8) | (rgba << 24);
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
};

[[nodiscard]] inline constexpr bool operator==(Color const &a,
                                               Color const &b) noexcept {
  return a.rgba == b.rgba;
}

[[nodiscard]] inline constexpr bool operator!=(Color const &a,
                                               Color const &b) noexcept {
  return !(a == b);
}

inline constexpr Color operator|(Color const &a, Color const &b) noexcept {
  return Color{a.rgba | b.rgba};
}

inline constexpr Color operator&(Color const &a, Color const &b) noexcept {
  return Color{a.rgba & b.rgba};
}

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

struct [[nodiscard]] Edge {
  uint32_t top = 0, right = 0, bottom = 0, left = 0;

  static constexpr Edge uniform(uint32_t value) noexcept {
    return Edge{value, value, value, value};
  }

  static constexpr Edge xy(uint32_t x, uint32_t y) noexcept {
    return Edge{y, x, y, x};
  }

  static constexpr Edge trbl(uint32_t t, uint32_t r, uint32_t b,
                             uint32_t l) noexcept {
    return Edge{t, r, b, l};
  }
};

[[nodiscard]] inline constexpr bool operator==(Edge const &a,
                                               Edge const &b) noexcept {
  return a.top == b.top && a.right == b.right && a.bottom == b.bottom &&
         a.left == b.left;
}

[[nodiscard]] inline constexpr bool operator!=(Edge const &a,
                                               Edge const &b) noexcept {
  return !(a == b);
}

struct [[nodiscard]] Corner {
  uint32_t top_left = 0, top_right = 0, bottom_right = 0, bottom_left = 0;

  static constexpr Corner uniform(uint32_t value) noexcept {
    return Corner{value, value, value, value};
  }

  static constexpr Corner across(uint32_t tl_br, uint32_t tr_bl) noexcept {
    return Corner{tl_br, tr_bl, tl_br, tr_bl};
  }

  static constexpr Corner spec(uint32_t tl, uint32_t tr, uint32_t br,
                               uint32_t bl) noexcept {
    return Corner{tl, tr, br, bl};
  }
};

[[nodiscard]] inline constexpr bool operator==(Corner const &a,
                                               Corner const &b) noexcept {
  return a.top_left == b.top_left && a.top_right == b.top_right &&
         a.bottom_right == b.bottom_right && a.bottom_left == b.bottom_left;
}

[[nodiscard]] inline constexpr bool operator!=(Corner const &a,
                                               Corner const &b) noexcept {
  return !(a == b);
}

enum class [[nodiscard]] Stretch : uint8_t{
    None = 0b00,
    X = 0b01,
    Y = 0b10,
    All = X | Y,
};

inline constexpr Stretch operator|(Stretch a, Stretch b) noexcept {
  return vlk::enum_or(a, b);
}

inline constexpr Stretch operator&(Stretch a, Stretch b) noexcept {
  return vlk::enum_and(a, b);
}

// TODO(lamarrr): consider using constrain with aspect ratio?

struct [[nodiscard]] Sizing {
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
  constexpr Sizing(Sizing && other) = default;
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
#if VLK_ENABLE_DEBUG_CHECKS
    if (type_ == Type::Relative) return stx::Some(RelativeRect{relative_data_});
    return stx::None;
#else
    return stx::Some(RelativeRect{relative_data_});
#endif
  }

  stx::Option<Rect> get_absolute() const noexcept {
#if VLK_ENABLE_DEBUG_CHECKS
    if (type_ == Type::Absolute) return stx::Some(Rect{rect_data_});
    return stx::None;
#else
    return stx::Some(Rect{rect_data_});
#endif
  }

 private:
  Type type_;
  union {
    RelativeRect relative_data_;
    Rect rect_data_;
  };
};

}  // namespace ui
}  // namespace vlk
