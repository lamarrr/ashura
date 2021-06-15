#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "modules/skparagraph/include/Paragraph.h"
#include "stx/option.h"
#include "stx/span.h"
#include "vlk/ui/font_source.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/utils/limits.h"

namespace vlk {
namespace ui {

// text shadow
// h-shadow	Required. The position of the horizontal shadow. Negative values
// are allowed v-shadow	Required. The position of the vertical shadow. Negative
// values are allowed
// blur-radius	Optional. The blur radius. Default value is 0
// color	Optional. The color of the shadow. Look at CSS Color Values for
// a complete list of possible color values
//
//
//

// TODO(lamarrr): all widgets must have a constrain method
// especially since our layout system is context agnostic
struct TextProps {
  TextProps color(Color text_color) const {
    TextProps out{*this};
    out.color_ = stx::Some(std::move(text_color));
    return out;
  }

  TextProps color(stx::NoneType) const {
    TextProps out{*this};
    out.color_ = stx::None;
    return out;
  }

  auto color() const { return color_; }

  TextProps background_color(Color color) const {
    TextProps out{*this};
    out.background_color_ = stx::Some(std::move(color));
    return out;
  }

  TextProps background_color(stx::NoneType) const {
    TextProps out{*this};
    out.background_color_ = stx::None;
    return out;
  }

  auto background_color() const { return background_color_; }

  TextProps font_size(float size) const {
    TextProps out{*this};
    out.font_size_ = stx::Some(std::move(size));
    return out;
  }

  TextProps font_size(stx::NoneType) const {
    TextProps out{*this};
    out.font_size_ = stx::None;
    return out;
  }

  auto font_size() const { return font_size_; }

  TextProps letter_spacing(float spacing) const {
    TextProps out{*this};
    out.letter_spacing_ = stx::Some(std::move(spacing));
    return out;
  }

  TextProps letter_spacing(stx::NoneType) const {
    TextProps out{*this};
    out.letter_spacing_ = stx::None;
    return out;
  }

  auto letter_spacing() const { return letter_spacing_; }

  TextProps word_spacing(float spacing) const {
    TextProps out{*this};
    out.word_spacing_ = stx::Some(std::move(spacing));
    return out;
  }

  TextProps word_spacing(stx::NoneType) const {
    TextProps out{*this};
    out.word_spacing_ = stx::None;
    return out;
  }

  auto word_spacing() const { return word_spacing_; }

  TextProps locale(std::string new_locale) const {
    TextProps out{*this};
    out.locale_ = stx::Some(std::move(new_locale));
    return out;
  }

  TextProps locale(stx::NoneType) const {
    TextProps out{*this};
    out.locale_ = stx::None;
    return out;
  }

  TextProps default_locale() const {
    TextProps out{*this};
    out.locale_ = stx::None;
    return out;
  }

  auto locale() const { return locale_; }

  //! uses the specified system font if available, else uses the default
  //! system font
  TextProps font(SystemFont system_font) const {
    TextProps out{*this};
    out.font_ = stx::Some(FontSource{std::move(system_font)});
    return out;
  }

  //! loads the specified fonts from the specified faces of the fonts if not
  //! already loaded. if the required font face specified by `.style` fails
  //! to load, the defult system font is used.
  TextProps font(FileFont file_font) const {
    TextProps out{*this};
    out.font_ = stx::Some(FontSource{std::move(file_font)});
    return out;
  }

  //! decodes the specified fonts from the provided bytes if not
  //! already loaded. if the required font face specified by `.style` fails
  //! to load, the defult system font is used.
  TextProps font(MemoryFont memory_font) const {
    TextProps out{*this};
    out.font_ = stx::Some(FontSource{std::move(memory_font)});
    return out;
  }

  //! loads the typeface from the specified path. if the required typeface fails
  //! to load, the defult system font is used.
  TextProps font(FileTypefaceSource file_source) const {
    TextProps out{*this};
    out.font_ = stx::Some(FontSource{std::move(file_source)});
    return out;
  }

  //! decodes the typeface from the provided bytes. if the required typeface
  //! fails to decode, the defult system font is used.
  TextProps font(MemoryTypefaceSource memory_source) const {
    TextProps out{*this};
    out.font_ = stx::Some(FontSource{std::move(memory_source)});
    return out;
  }

  TextProps font(stx::NoneType) const {
    TextProps out{*this};
    out.font_ = stx::None;
    return out;
  }

  auto font() const { return font_; }

  auto const& font_ref() const { return font_; }

  TextProps underlined() const {
    TextProps out{*this};
    out.decoration_ =
        stx::Some(decoration_.clone().unwrap_or(TextDecoration::None) |
                  TextDecoration::Underline);
    return out;
  }

  TextProps overlined() const {
    TextProps out{*this};
    out.decoration_ =
        stx::Some(decoration_.clone().unwrap_or(TextDecoration::None) |
                  TextDecoration::Overline);
    return out;
  }

  TextProps strikethrough() const {
    TextProps out{*this};
    out.decoration_ =
        stx::Some(decoration_.clone().unwrap_or(TextDecoration::None) |
                  TextDecoration::StrikeThrough);
    return out;
  }

  TextProps decoration(TextDecoration new_decoration) const {
    TextProps out{*this};
    out.decoration_ = stx::Some(std::move(new_decoration));
    return out;
  }

  TextProps decoration(stx::NoneType) const {
    TextProps out{*this};
    out.decoration_ = stx::None;
    return out;
  }

  auto decoration() const { return decoration_; }

  TextProps decoration_color(Color color) const {
    TextProps out{*this};
    out.decoration_color_ = stx::Some(std::move(color));
    return out;
  }

  TextProps decoration_color(stx::NoneType) const {
    TextProps out{*this};
    out.decoration_color_ = stx::None;
    return out;
  }

  auto decoration_color() const { return decoration_color_; }

  TextProps decoration_style(TextDecorationStyle style) const {
    TextProps out{*this};
    out.decoration_style_ = stx::Some(std::move(style));
    return out;
  }

  TextProps decoration_style(stx::NoneType) const {
    TextProps out{*this};
    out.decoration_style_ = stx::None;
    return out;
  }

  auto decoration_style() const { return decoration_style_; }

  TextProps antialias(bool value) const {
    TextProps out{*this};
    out.antialiased_ = stx::Some(std::move(value));
    return out;
  }

  auto antialias() const { return antialiased_; }

  bool operator==(TextProps const& other) const {
    return color_ == other.color_ &&
           background_color_ == other.background_color_ &&
           font_size_ == other.font_size_ &&
           letter_spacing_ == other.letter_spacing_ &&
           word_spacing_ == other.word_spacing_ && locale_ == other.locale_ &&
           decoration_ == other.decoration_ &&
           decoration_color_ == other.decoration_color_ &&
           decoration_style_ == other.decoration_style_ &&
           font_ == other.font_ && antialiased_ == other.antialiased_;
  }

  bool operator!=(TextProps const& other) const { return !(*this == other); }

 private:
  stx::Option<Color> color_ = stx::None;
  stx::Option<Color> background_color_ = stx::None;
  stx::Option<float> font_size_ = stx::None;
  stx::Option<float> letter_spacing_ = stx::None;
  stx::Option<float> word_spacing_ = stx::None;
  stx::Option<std::string> locale_ = stx::None;
  stx::Option<TextDecoration> decoration_ = stx::None;
  stx::Option<Color> decoration_color_ = stx::None;
  stx::Option<TextDecorationStyle> decoration_style_ = stx::None;
  stx::Option<FontSource> font_ = stx::None;
  stx::Option<bool> antialiased_ = stx::None;
};

namespace impl {

struct ResolvedTextProps {
  // no reflow
  Color color = colors::Black;
  // no reflow
  Color background_color = colors::Transparent;
  // reflow
  float font_size = 14.0f;
  // reflow
  float letter_spacing = 0.0f;
  // reflow
  float word_spacing = 0.0f;
  // reflow
  std::string locale = "";
  // no reflow
  TextDecoration decoration = TextDecoration::None;
  // no reflow
  Color decoration_color = colors::Black;
  // no reflow
  TextDecorationStyle decoration_style = TextDecorationStyle::Solid;
  // reflow
  FontSource font = SystemFont{};
  // no reflow
  bool antialiased = true;

  bool operator==(ResolvedTextProps const& other) const {
    return color == other.color && background_color == other.background_color &&
           font_size == other.font_size &&
           letter_spacing == other.letter_spacing &&
           word_spacing == other.word_spacing && locale == other.locale &&
           decoration == other.decoration &&
           decoration_color == other.decoration_color &&
           decoration_style == other.decoration_style && font == other.font &&
           antialiased == other.antialiased;
  }

  bool operator!=(ResolvedTextProps const& other) const {
    return !(*this == other);
  }
};

}  // namespace impl

struct ParagraphProps {
  ParagraphProps color(Color text_color) const {
    ParagraphProps out{*this};
    out.text_props_.color = text_color;
    return out;
  }

  auto color() const { return text_props_.color; }

  ParagraphProps background_color(Color color) const {
    ParagraphProps out{*this};
    out.text_props_.background_color = color;
    return out;
  }

  auto background_color() const { return text_props_.background_color; }

  ParagraphProps font_size(float size) const {
    ParagraphProps out{*this};
    out.text_props_.font_size = size;
    return out;
  }

  auto font_size() const { return text_props_.font_size; }

  ParagraphProps letter_spacing(float spacing) const {
    ParagraphProps out{*this};
    out.text_props_.letter_spacing = spacing;
    return out;
  }

  auto letter_spacing() const { return text_props_.letter_spacing; }

  ParagraphProps word_spacing(float spacing) const {
    ParagraphProps out{*this};
    out.text_props_.word_spacing = spacing;
    return out;
  }

  auto word_spacing() const { return text_props_.word_spacing; }

  ParagraphProps locale(std::string new_locale) const {
    ParagraphProps out{*this};
    out.text_props_.locale = std::move(new_locale);
    return out;
  }

  ParagraphProps default_locale() const {
    ParagraphProps out{*this};
    out.text_props_.locale = "";
    return out;
  }

  auto locale() const { return text_props_.locale; }

  ParagraphProps font(SystemFont system_font) const {
    ParagraphProps out{*this};
    out.text_props_.font = std::move(system_font);
    return out;
  }

  ParagraphProps font(FileFont file_font) const {
    ParagraphProps out{*this};
    out.text_props_.font = std::move(file_font);
    return out;
  }

  ParagraphProps font(MemoryFont memory_font) const {
    ParagraphProps out{*this};
    out.text_props_.font = std::move(memory_font);
    return out;
  }

  ParagraphProps font(FileTypefaceSource file_source) const {
    ParagraphProps out{*this};
    out.text_props_.font = std::move(file_source);
    return out;
  }

  ParagraphProps font(MemoryTypefaceSource memory_source) const {
    ParagraphProps out{*this};
    out.text_props_.font = std::move(memory_source);
    return out;
  }

  ParagraphProps font(FontSource font_source) const {
    ParagraphProps out{*this};
    out.text_props_.font = std::move(font_source);
    return out;
  }

  auto font() const { return text_props_.font; }

  auto const& font_ref() const { return text_props_.font; }

  ParagraphProps underlined() const {
    ParagraphProps out{*this};
    out.text_props_.decoration =
        text_props_.decoration | TextDecoration::Underline;
    return out;
  }

  ParagraphProps overlined() const {
    ParagraphProps out{*this};
    out.text_props_.decoration =
        text_props_.decoration | TextDecoration::Overline;
    return out;
  }

  ParagraphProps strikethrough() const {
    ParagraphProps out{*this};
    out.text_props_.decoration =
        text_props_.decoration | TextDecoration::StrikeThrough;
    return out;
  }

  ParagraphProps decoration(TextDecoration new_decoration) const {
    ParagraphProps out{*this};
    out.text_props_.decoration = new_decoration;
    return out;
  }

  auto decoration() const { return text_props_.decoration; }

  ParagraphProps decoration_color(Color color) const {
    ParagraphProps out{*this};
    out.text_props_.decoration_color = color;
    return out;
  }

  auto decoration_color() const { return text_props_.decoration_color; }

  ParagraphProps decoration_style(TextDecorationStyle style) const {
    ParagraphProps out{*this};
    out.text_props_.decoration_style = style;
    return out;
  }

  auto decoration_style() const { return text_props_.decoration_style; }

  ParagraphProps antialias(bool value) const {
    ParagraphProps out{*this};
    out.text_props_.antialiased = value;
    return out;
  }

  auto antialias() const { return text_props_.antialiased; }

  ParagraphProps direction(TextDirection direction) const {
    ParagraphProps out{*this};
    out.direction_ = direction;
    return out;
  }

  auto direction() const { return direction_; }

  ParagraphProps align(TextAlign align) const {
    ParagraphProps out{*this};
    out.align_ = align;
    return out;
  }

  auto align() const { return align_; }

  ParagraphProps line_limit(uint32_t limit) const {
    ParagraphProps out{*this};
    out.line_limit_ = limit;
    return out;
  }

  auto line_limit() const { return line_limit_; }

  bool operator==(ParagraphProps const& other) const {
    return text_props_ == other.text_props_ && direction_ == other.direction_ &&
           align_ == other.align_ && line_limit_ == other.line_limit_;
  }

  bool operator!=(ParagraphProps const& other) const {
    return !(*this == other);
  }

 private:
  impl::ResolvedTextProps text_props_;

  // reflow
  TextDirection direction_ = TextDirection::Ltr;
  // reflow
  TextAlign align_ = TextAlign::Left;
  // reflow
  uint32_t line_limit_ = u32_max;
};

struct InlineText {
  std::string text;
  //! uses the paragraph's default style if none is set
  TextProps props = TextProps{};
};

enum class TextState : uint8_t { Begin, FontsLoading, FontsLoadDone };

namespace impl {

struct InlineTextStorage {
  std::string text;
  TextProps props;
  // this is always held on to and never released once loaded, because it is
  // expensive to re-load and re-layout the fonts after the text hasn't been
  // in view for long and would cause undesired reflow
  stx::Option<std::shared_ptr<TypefaceAsset const>> typeface = stx::None;
  TextState state = TextState::Begin;
};

struct ParagraphStorage {
  ParagraphProps props;
  stx::Option<std::shared_ptr<TypefaceAsset const>> typeface = stx::None;
  TextState state = TextState::Begin;
};

enum class TextDiff : uint16_t {
  //! nothing changed in the inline text
  None = 0,
  //! the color of the inline text changed
  Color = 1 << 0,
  //! the background color of the inline text changed
  BgColor = 1 << 1,
  //! the font size of the inline text changed
  FontSize = 1 << 2,
  LetterSpacing = 1 << 3,
  WordSpacing = 1 << 4,
  Locale = 1 << 5,
  Decoration = 1 << 6,
  DecorationColor = 1 << 7,
  DecorationStyle = 1 << 8,
  Font = 1 << 9,
  Antialias = 1 << 10,
  //! the length of the inline text or the contents of the inline texts have
  //! changed
  Text = 1 << 11,
  Direction = 1 << 12,
  Align = 1 << 13,
  LineLimit = 1 << 14,
  All = (LineLimit << 1) - 1
};

VLK_DEFINE_ENUM_BIT_OPS(TextDiff)

}  // namespace impl

// Requirements
// - we want to have multiple inline texts joined into one text widget.
// - we want to be able to change the properties of the paragraph's inline texts
// without causing a layout reflow. i.e. a case where only the decoration or
// render properties of the inline text or paragraph changes. though this would
// still require a full rebuild of the skia paragraph. typically done for text
// highlighting which is expected to be fast.
//
// Nice to haves:
// - update text but continue using style and props
//
//
//
struct Text : public Widget {
  Text(std::string utf8_text, TextProps text_props = TextProps{},
       ParagraphProps paragraph_props = ParagraphProps{})
      : Text{std::vector{
                 {InlineText{std::move(utf8_text), std::move(text_props)}}},
             std::move(paragraph_props)} {}

  Text(std::vector<InlineText> inline_texts,
       ParagraphProps paragraph_props = ParagraphProps{}) {
    update_paragraph_props(std::move(paragraph_props));
    update_text(std::move(inline_texts));
    rebuild_paragraph();
  }

  std::vector<impl::InlineTextStorage> get_inline_texts() const {
    return inline_texts_;
  }

  ParagraphProps get_paragraph_props() const {
    return paragraph_storage_.props;
  }

  TextState get_paragraph_state() const { return paragraph_storage_.state; }

  void update_text(std::string utf8_text, TextProps text_props) {
    update_text(
        std::vector{{InlineText{std::move(utf8_text), std::move(text_props)}}});
  }

  void update_text(std::vector<InlineText> inline_texts);

  void update_paragraph_props(ParagraphProps paragraph_props);

  virtual Extent trim(Extent) override;

  virtual void draw(Canvas&) override;

  virtual void tick(std::chrono::nanoseconds,
                    AssetManager& asset_manager) override;

 private:
  impl::ParagraphStorage paragraph_storage_;
  std::vector<impl::InlineTextStorage> inline_texts_;

  std::unique_ptr<skia::textlayout::Paragraph> paragraph_ = nullptr;
  impl::TextDiff diff_ = impl::TextDiff::All;

  void rebuild_paragraph();
};

}  // namespace ui
}  // namespace vlk
