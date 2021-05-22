#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

#include "stx/span.h"
#include "vlk/utils/limits.h"

#include "modules/skparagraph/include/Paragraph.h"

// future-TODO(lamarrr): accessibility text scale factor, can be changed at
// runtime? or constant

// PR0
// TODO(lamarrr): IMPORTANT!!! Font loading from path at least

namespace vlk {
namespace ui {

// from file, from byte data, from name
// we need to be able to share font data acrross widgets and thus need a font
// manager
struct Typeface;
struct Font;
struct FontManager;

enum class TextDecoration : uint8_t {
  None = 0,
  Underline = 1,
  Overline = 2,
  StrikeThrough = 4,
};

constexpr TextDecoration operator|(TextDecoration a, TextDecoration b) {
  return vlk::enum_or(a, b);
}

constexpr TextDecoration operator&(TextDecoration a, TextDecoration b) {
  return vlk::enum_and(a, b);
}

enum class FontSlant : uint8_t {
  Upright = 0,
  Italic = 1,
  Oblique = 2,
};

constexpr FontSlant operator|(FontSlant a, FontSlant b) {
  return vlk::enum_or(a, b);
}

constexpr FontSlant operator&(FontSlant a, FontSlant b) {
  return vlk::enum_and(a, b);
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
  FontWidth width = FontWidth::Normal;
  FontSlant slant = FontSlant::Upright;
};

struct TextProps {
  TextProps color(Color text_color) const {
    TextProps out{*this};
    out.foreground_color_ = text_color;
    return out;
  }

  Color color() const { return foreground_color_; }

  TextProps background_color(Color color) const {
    TextProps out{*this};
    out.background_color_ = color;
    return out;
  }

  Color background_color() const { return background_color_; }

  TextProps font_size(float size) const {
    TextProps out{*this};
    out.font_size_ = size;
    return out;
  }

  float font_size() const { return font_size_; }

  TextProps letter_spacing(float spacing) const {
    TextProps out{*this};
    out.letter_spacing_ = spacing;
    return out;
  }

  float letter_spacing() const { return letter_spacing_; }

  TextProps word_spacing(float spacing) const {
    TextProps out{*this};
    out.word_spacing_ = spacing;
    return out;
  }

  float word_spacing() const { return word_spacing_; }

  TextProps locale(std::string_view const& new_locale) const {
    TextProps out{*this};
    out.locale_ = new_locale;
    return out;
  }

  std::string locale() const { return locale_; }

  TextProps font_style(FontStyle style) const {
    TextProps out{*this};
    out.font_style_ = style;
    return out;
  }

  FontStyle font_style() const { return font_style_; }

  TextProps slant(FontSlant slant) const {
    TextProps out{*this};
    out.font_style_.slant = slant;
    return out;
  }

  FontSlant slant() const { return font_style_.slant; }

  TextProps italic() const { return slant(FontSlant::Italic); }
  TextProps upright() const { return slant(FontSlant::Upright); }
  TextProps oblique() const { return slant(FontSlant::Oblique); }

  TextProps font_weight(FontWeight weight) const {
    TextProps out{*this};
    out.font_style_.weight = weight;
    return out;
  }

  FontWeight font_weight() const { return font_style_.weight; }

  TextProps font_width(FontWidth width) const {
    TextProps out{*this};
    out.font_style_.width = width;
    return out;
  }

  FontWidth font_width() const { return font_style_.width; }

  TextProps underlined() const {
    TextProps out{*this};
    out.decoration_ = decoration_ | TextDecoration::Underline;
    return out;
  }

  TextProps overlined() const {
    TextProps out{*this};
    out.decoration_ = decoration_ | TextDecoration::Overline;
    return out;
  }

  TextProps strikethrough() const {
    TextProps out{*this};
    out.decoration_ = decoration_ | TextDecoration::StrikeThrough;
    return out;
  }

  TextProps no_decoration() const {
    TextProps out{*this};
    out.decoration_ = TextDecoration::None;
    return out;
  }

  TextDecoration decoration() const { return decoration_; }

  TextProps decoration_color(Color color) const {
    TextProps out{*this};
    out.decoration_color_ = color;
    return out;
  }

  Color decoration_color() const { return decoration_color_; }

  TextProps decoration_style(TextDecorationStyle style) const {
    TextProps out{*this};
    out.decoration_style_ = style;
    return out;
  }

  TextDecorationStyle decoration_style() const { return decoration_style_; }

  // TODO(lamarrr): fix this
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

  TextDirection direction() const { return direction_; }

  TextProps align(TextAlign align) const {
    TextProps out{*this};
    out.align_ = align;
    return out;
  }

  TextAlign align() const { return align_; }

  TextProps line_limit(uint32_t limit) const {
    TextProps out{*this};
    out.line_limit_ = limit;
    return out;
  }

  uint32_t line_limit() const { return line_limit_; }

  void antialias(bool value) { antialiased_ = value; }

  bool antialias() const { return antialiased_; }

 private:
  Color foreground_color_ = colors::Black;
  Color background_color_ = colors::Transparent;
  // float height = 1.0;
  // float leading = -1
  float font_size_ = 14.0f;
  float letter_spacing_ = 0.0f;
  float word_spacing_ = 0.0f;
  std::string locale_ = {};
  TextDecoration decoration_ = TextDecoration::None;
  Color decoration_color_ = colors::Black;
  FontStyle font_style_ = FontStyle{};
  TextDecorationStyle decoration_style_ = TextDecorationStyle::Solid;
  std::string font_family_ = "sans-serif";
  TextDirection direction_ = TextDirection::Ltr;
  TextAlign align_ = TextAlign::Left;
  uint32_t line_limit_ = u32_max;
  bool antialiased_ = true;
};

// TODO(lamarrr): A single paragraph. doesn't support multiple stylings. use
// builder->addTextStyle and builder->addText
struct Text : public Widget {
  Text(std::string_view const& utf8_str,
       TextProps const& properties = TextProps{}) {
    update_text(utf8_str);
    update_props(properties);
    Widget::update_needs_trimming(true);
    tick(std::chrono::nanoseconds(0));
  }

  ~Text() override final = default;

  std::string get_text() const { return utf8_text_; }

  TextProps get_props() const { return properties_; }

  void update_text(std::string_view const& utf8_str) {
    utf8_text_ = utf8_str;
    paragraph_dirty_ = true;
  }

  void update_props(TextProps const& properties) {
    properties_ = properties;
    paragraph_dirty_ = true;
  }

  // TODO(lamarrr): clip text if it height exceeds the maximum extent?
  virtual void draw(Canvas&, AssetManager&) override final;

  virtual void tick([
      [maybe_unused]] std::chrono::nanoseconds const& interval) override final {
    if (paragraph_dirty_) {
      rebuild_paragraph_();
      paragraph_dirty_ = false;

      mark_render_dirty();
      mark_layout_dirty();
    }
  }

  virtual Extent trim(Extent const&) override;

 private:
  std::string utf8_text_;
  TextProps properties_;

  std::unique_ptr<skia::textlayout::Paragraph> paragraph_{nullptr};
  bool paragraph_dirty_ = true;

  void rebuild_paragraph_();
};

}  // namespace ui
}  // namespace vlk
