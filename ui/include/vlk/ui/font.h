#pragma once

#include <cinttypes>
#include <string>
#include <string_view>

#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

enum class TextDecoration : uint8_t {
  None = 0,
  Underline = 1,
  Overline = 2,
  StrikeThrough = 4,
};

STX_DEFINE_ENUM_BIT_OPS(TextDecoration)

enum class FontSlant : uint8_t {
  Upright = 0,
  Italic = 1,
  Oblique = 2,
};

STX_DEFINE_ENUM_BIT_OPS(FontSlant)

constexpr std::string_view format(FontSlant slant) {
  switch (slant) {
    case FontSlant::Upright:
      return "Upright";

    case FontSlant::Italic:
      return "Italic";

    case FontSlant::Oblique:
      return "Oblique";

    default:
      return "";
  }
}

enum class FontWeight : uint16_t {
  Invisible = 0,
  Thin = 100,
  ExtraLight = 200,
  Light = 300,
  Normal = 400,
  Medium = 500,
  SemiBold = 600,
  Bold = 700,
  ExtraBold = 800,
  Black = 900,
  ExtraBlack = 1000,
};

constexpr std::string_view format(FontWeight weight) {
  switch (weight) {
    case FontWeight::Invisible:
      return "Invisible";

    case FontWeight::Thin:
      return "Thin";

    case FontWeight::ExtraLight:
      return "ExtraLight";

    case FontWeight::Light:
      return "Light";

    case FontWeight::Normal:
      return "Normal";

    case FontWeight::Medium:
      return "Medium";

    case FontWeight::SemiBold:
      return "SemiBold";

    case FontWeight::Bold:
      return "Bold";

    case FontWeight::ExtraBold:
      return "ExtraBold";

    case FontWeight::Black:
      return "Black";

    case FontWeight::ExtraBlack:
      return "ExtraBlack";

    default:
      return "";
  }
}

enum class FontWidth : uint8_t {
  UltraCondensed = 1,
  ExtraCondensed = 2,
  Condensed = 3,
  SemiCondensed = 4,
  Normal = 5,
  SemiExpanded = 6,
  Expanded = 7,
  ExtraExpanded = 8,
  UltraExpanded = 9,
};

constexpr std::string_view format(FontWidth width) {
  switch (width) {
    case FontWidth::UltraCondensed:
      return "UltraCondensed";

    case FontWidth::ExtraCondensed:
      return "ExtraCondensed";

    case FontWidth::Condensed:
      return "Condensed";

    case FontWidth::SemiCondensed:
      return "SemiCondensed";

    case FontWidth::Normal:
      return "Normal";

    case FontWidth::SemiExpanded:
      return "SemiExpanded";

    case FontWidth::Expanded:
      return "Expanded";

    case FontWidth::ExtraExpanded:
      return "ExtraExpanded";

    case FontWidth::UltraExpanded:
      return "UltraExpanded";

    default:
      return "";
  }
}

enum class TextDecorationStyle : uint8_t {
  Solid = 0,
  Double,
  Dotted,
  Dashed,
  Wavy
};

enum class TextDirection : uint8_t {
  Rtl,
  Ltr,
};

enum class TextAlign : uint8_t {
  Left,
  Right,
  Center,
  Justify,
  Start,
  End,
};

struct FontStyle {
  FontWeight weight = FontWeight::Normal;
  FontSlant slant = FontSlant::Upright;
  FontWidth width = FontWidth::Normal;

  constexpr bool operator==(FontStyle const& other) const {
    return weight == other.weight && slant == other.slant &&
           width == other.width;
  }

  constexpr bool operator!=(FontStyle const& other) const {
    return !(*this == other);
  }
};

std::string format(FontStyle style);

}  // namespace ui
}  // namespace vlk
