#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

#include "include/core/SkPaint.h"
#include "stx/span.h"
#include "vlk/utils/limits.h"

namespace skia {
namespace textlayout {

struct Paragraph;

}
}  // namespace skia

// TODO(lamarrr): accessibility text scale factor, can be changed at runtime? or
// constant
// TODO(lamarrr): IMPORTANT!!! Font loading from path at least

namespace vlk {
namespace ui {

enum class TextSlant : uint8_t {
  Upright,
  Italic,
  Oblique,
};

enum class TextDecoration : uint8_t {
  None = 0,
  Underline = 1,
  Overline = 2,
  StrikeThrough = 4,
};

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

// TODO(LAMARRR): update the use of this
enum class TextDecorationStyle : uint8_t {
  None = 0,
  Solid = 1,
  Double = 2,
  Dotted = 4,
  Dashed = 8,
  Wavy = 16
};

struct TextShadow {
  Color color = colors::Black;
  Offset offset = Offset{0, 0};
  double blur_radius = 0.0;
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

struct TextProps {
  TextProps color(Color text_color) const {
    TextProps out{*this};
    out.foreground_paint_.setColor(text_color.to_argb());
    return out;
  }

  Color color() const noexcept {
    return Color::from_argb(foreground_paint_.getColor());
  }

  TextProps background_color(Color color) const {
    TextProps out{*this};
    out.background_paint_.setColor(color.to_argb());
    return out;
  }

  Color background_color() const noexcept {
    return Color::from_argb(background_paint_.getColor());
  }

  TextProps font_size(float size) const {
    TextProps out{*this};
    out.font_size_ = size;
    return out;
  }

  float font_size() const noexcept { return font_size_; }

  TextProps letter_spacing(float spacing) const {
    TextProps out{*this};
    out.letter_spacing_ = spacing;
    return out;
  }

  float letter_spacing() const noexcept { return letter_spacing_; }

  TextProps word_spacing(float spacing) const {
    TextProps out{*this};
    out.word_spacing_ = spacing;
    return out;
  }

  float word_spacing() const noexcept { return word_spacing_; }

  TextProps locale(std::string_view const& name) const {
    TextProps out{*this};
    size_t max_size = name.size() < static_cast<size_t>(std::size(locale_) - 1)
                          ? name.size()
                          : static_cast<size_t>(std::size(locale_));
    for (char& c : out.locale_) c = 0x00;
    for (size_t i = 0; i < max_size; i++) {
      out.locale_[i] = name[i];
    }
    return out;
  }

  std::string locale() const { return locale_; }

  TextProps slant(TextSlant slant) const {
    TextProps out{*this};
    out.slant_ = slant;
    return out;
  }

  TextProps italic() const { return slant(TextSlant::Italic); }

  TextSlant slant() const noexcept { return slant_; }

  TextProps underlined() const {
    TextProps out{*this};
    out.decoration_ = static_cast<TextDecoration>(
        static_cast<uint8_t>(decoration_) |
        static_cast<uint8_t>(TextDecoration::Underline));
    return out;
  }

  bool is_underlined() const noexcept {
    return static_cast<bool>(static_cast<uint8_t>(decoration_) &
                             static_cast<uint8_t>(TextDecoration::Underline));
  }

  TextProps overlined() const {
    TextProps out{*this};
    out.decoration_ = static_cast<TextDecoration>(
        static_cast<uint8_t>(decoration_) |
        static_cast<uint8_t>(TextDecoration::Overline));
    return out;
  }

  bool is_overlined() const noexcept {
    return static_cast<bool>(static_cast<uint8_t>(decoration_) &
                             static_cast<uint8_t>(TextDecoration::Overline));
  }

  TextProps strikethrough() {
    TextProps out{*this};
    out.decoration_ = static_cast<TextDecoration>(
        static_cast<uint8_t>(decoration_) |
        static_cast<uint8_t>(TextDecoration::StrikeThrough));
    return out;
  }

  bool has_strikethrough() const noexcept {
    return static_cast<bool>(
        static_cast<uint8_t>(decoration_) &
        static_cast<uint8_t>(TextDecoration::StrikeThrough));
  }

  TextProps clear_decorations() const {
    TextProps out{*this};
    out.decoration_ = TextDecoration::NoDecoration;
    return out;
  }

  TextDecoration get_decorations() const noexcept { return decoration_; }

  TextProps font_weight(FontWeight weight) const {
    TextProps out{*this};
    out.font_weight_ = weight;
    return out;
  }

  FontWeight font_weight() const { return font_weight_; }

  TextProps decoration_style(TextDecorationStyle style) const {
    TextProps out{*this};
    out.decoration_style_ = style;
    return out;
  }

  TextDecorationStyle decoration_style() const noexcept {
    return decoration_style_;
  }

  TextProps font_family(std::string_view const& font) const {
    TextProps out{*this};
    out.font_family_ = font;
    return out;
  }

  std::string font_family() const { return font_family_; }

  TextProps direction(TextDirection direction) const {
    TextProps out{*this};
    out.direction_ = direction;
    return out;
  }

  TextDirection direction() const noexcept { return direction_; }

  TextProps align(TextAlign align) {
    TextProps out{*this};
    out.align_ = align;
    return out;
  }

  TextAlign align() const noexcept { return align_; }

  TextProps line_limit(size_t limit) {
    TextProps out{*this};
    out.line_limit_ = limit;
    return out;
  }

  size_t line_limit() const noexcept { return line_limit_; }

 private:
  SkPaint foreground_paint_ = default_foreground_paint_();
  SkPaint background_paint_ = default_background_paint_();
  // float height = 1.0;
  float font_size_ = 14.0, letter_spacing_ = 0.0, word_spacing_ = 0.0;
  char locale_[85] = {};
  TextDecoration decoration_ = TextDecoration::None;
  FontWeight font_weight_ = FontWeight::Normal;
  TextSlant slant_ = TextSlant::Upright;
  TextDecorationStyle decoration_style_ = TextDecorationStyle::Solid;
  std::string font_family_ = "sans-serif";
  TextDirection direction_ = TextDirection::Ltr;
  TextAlign align_ = TextAlign::Left;
  size_t line_limit_ = usize_max;

  static SkPaint default_foreground_paint_() {
    SkPaint paint;
    paint.setColor(colors::Black.to_argb());
    paint.setAntiAlias(true);
    return paint;
  }

  static SkPaint default_background_paint_() {
    SkPaint paint;
    paint.setColor(colors::Transparent.to_argb());
    paint.setAntiAlias(true);
    return paint;
  }

  friend SkPaint const& ref_background_paint(TextProps const& props) noexcept;
  friend SkPaint const& ref_foreground_paint(TextProps const& props) noexcept;
};

struct Text : public Widget {
  Text(std::string_view const& str, TextProps const& properties = TextProps{},
       stx::Span<TextShadow const> const& shadows = {});

  virtual void draw(Canvas& canvas) override;

 private:
  std::string text_data_;
  std::unique_ptr<skia::textlayout::Paragraph> paragraph_;
};

}  // namespace ui
}  // namespace vlk
