#pragma once
#include "ashura/animation.h"
#include "ashura/primitives.h"

namespace ash
{
constexpr nanoseconds THEME_ANIMATION_DURATION = milliseconds{200};

struct ThemeData
{
  color            background;          // color of the background
  color            primary;             // most frequent color in the theme. usually dark.
  color            on_primary;          // color for items on primary colored areas
  color            secondary;           // used to ascent areas around primary colors. i.e. highlighting or pinpointing certain areas. usually lighter version of the primary color
  color            on_secondary;        // colors for items on secondary colored areas
  color            surface;             // color used for containers, lists, etc.
  color            on_surface;          // color to use for items on top of surface colored areas
  color            error;               // color to use for error boxes
  color            on_error;            // color to use for error texts
  std::string_view header_font;
  std::string_view header_fallback_font;
  f32              header_font_size = 0;
  std::string_view body_font;
  std::string_view body_fallback_font;
  f32              body_font_size = 0;
};

template <>
struct Tween<ThemeData>
{
  ThemeData begin;
  ThemeData end;

  constexpr ThemeData lerp(f32 percentage);
};

struct Theme
{
  ThemeData dark{
      .header_font          = "RobotoMono",
      .header_fallback_font = "RobotoMono",
      .header_font_size     = 30,
      .body_font            = "Roboto",
      .body_fallback_font   = "Roboto",
      .body_font_size       = 18};

  ThemeData light{
      .header_font          = "RobotoMono",
      .header_fallback_font = "RobotoMono",
      .header_font_size     = 30,
      .body_font            = "Roboto",
      .body_fallback_font   = "Roboto",
      .body_font_size       = 18};
};

}        // namespace ash