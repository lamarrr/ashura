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

namespace vlk {
namespace ui {

enum class TextSlant : uint8_t {
  Upright,
  Italic,
  Oblique,
};

enum class TextDecoration : uint8_t {
  NoDecoration = 0x0,
  Underline = 0x1,
  Overline = 0x2,
  StrikeThrough = 0x4,
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

enum class DecorationStyle : uint8_t { Solid, Double, Dotted, Dashed, Wavy };

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

struct TextProperties;

namespace impl {
SkPaint const& ref_background_paint(TextProperties const& props) noexcept;

SkPaint const& ref_foreground_paint(TextProperties const& props) noexcept;
}  // namespace impl

struct TextProperties {
  TextProperties color(Color text_color) const {
    TextProperties out{*this};
    out.foreground_paint_.setColor(text_color.to_argb());
    return out;
  }

  Color color() const noexcept {
    return Color::from_argb(foreground_paint_.getColor());
  }

  TextProperties background_color(Color color) const {
    TextProperties out{*this};
    out.background_paint_.setColor(color.to_argb());
    return out;
  }

  Color background_color() const noexcept {
    return Color::from_argb(background_paint_.getColor());
  }

  TextProperties font_size(float size) const {
    TextProperties out{*this};
    out.font_size_ = size;
    return out;
  }

  float font_size() const noexcept { return font_size_; }

  TextProperties letter_spacing(float spacing) const {
    TextProperties out{*this};
    out.letter_spacing_ = spacing;
    return out;
  }

  float letter_spacing() const noexcept { return letter_spacing_; }

  TextProperties word_spacing(float spacing) const {
    TextProperties out{*this};
    out.word_spacing_ = spacing;
    return out;
  }

  float word_spacing() const noexcept { return word_spacing_; }

  TextProperties locale(std::string_view const& name) const {
    TextProperties out{*this};
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

  TextProperties slant(TextSlant slant) const {
    TextProperties out{*this};
    out.slant_ = slant;
    return out;
  }

  TextProperties italic() const { return slant(TextSlant::Italic); }

  TextSlant slant() const noexcept { return slant_; }

  TextProperties underlined() const {
    TextProperties out{*this};
    out.decoration_ = static_cast<TextDecoration>(
        static_cast<uint8_t>(decoration_) |
        static_cast<uint8_t>(TextDecoration::Underline));
    return out;
  }

  bool is_underlined() const noexcept {
    return static_cast<bool>(static_cast<uint8_t>(decoration_) &
                             static_cast<uint8_t>(TextDecoration::Underline));
  }

  TextProperties overlined() const {
    TextProperties out{*this};
    out.decoration_ = static_cast<TextDecoration>(
        static_cast<uint8_t>(decoration_) |
        static_cast<uint8_t>(TextDecoration::Overline));
    return out;
  }

  bool is_overlined() const noexcept {
    return static_cast<bool>(static_cast<uint8_t>(decoration_) &
                             static_cast<uint8_t>(TextDecoration::Overline));
  }

  TextProperties strikethrough() {
    TextProperties out{*this};
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

  TextProperties clear_decorations() const {
    TextProperties out{*this};
    out.decoration_ = TextDecoration::NoDecoration;
    return out;
  }

  TextDecoration get_decorations() const noexcept { return decoration_; }

  TextProperties font_weight(FontWeight weight) const {
    TextProperties out{*this};
    out.font_weight_ = weight;
    return out;
  }

  FontWeight font_weight() const { return font_weight_; }

  TextProperties decoration_style(DecorationStyle style) const {
    TextProperties out{*this};
    out.decoration_style_ = style;
    return out;
  }

  DecorationStyle decoration_style() const noexcept {
    return decoration_style_;
  }

  TextProperties font_family(std::string_view const& font) const {
    TextProperties out{*this};
    out.font_family_ = font;
    return out;
  }

  std::string font_family() const { return font_family_; }

  TextProperties direction(TextDirection direction) const {
    TextProperties out{*this};
    out.direction_ = direction;
    return out;
  }

  TextDirection direction() const noexcept { return direction_; }

  TextProperties align(TextAlign align) {
    TextProperties out{*this};
    out.align_ = align;
    return out;
  }

  TextAlign align() const noexcept { return align_; }

  TextProperties line_limit(size_t limit) {
    TextProperties out{*this};
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
  TextDecoration decoration_ = TextDecoration::NoDecoration;
  FontWeight font_weight_ = FontWeight::Normal;
  TextSlant slant_ = TextSlant::Upright;
  DecorationStyle decoration_style_ = DecorationStyle::Solid;
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

  friend SkPaint const& impl::ref_background_paint(
      TextProperties const& props) noexcept;
  friend SkPaint const& impl::ref_foreground_paint(
      TextProperties const& props) noexcept;
};

struct Text : public Widget {
  Text(std::string_view const& str,
       TextProperties const& properties = TextProperties{},
       stx::Span<TextShadow const> const& shadows = {});

  // virtual stx::Span<Widget* const> get_children() const noexcept override {
  //  return {};
  // }

  /*virtual Rect compute_area(
      Extent const& allotted_extent,
      [[maybe_unused]] stx::Span<Rect> const& children_area) override;
*/

  virtual void draw(Canvas& canvas) override;

 private:
  std::string text_data_;
  std::unique_ptr<skia::textlayout::Paragraph> paragraph_;
};

}  // namespace ui
}  // namespace vlk
