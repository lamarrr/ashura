#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "modules/skparagraph/include/Paragraph.h"
#include "stx/span.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/utils/limits.h"

// future-TODO(lamarrr): accessibility text scale factor, can be changed at
// runtime? or constant

// PR0
// TODO(lamarrr): IMPORTANT!!! Font loading from path at least

namespace vlk {
namespace ui {

// from file, from byte data, from name
// we need to be able to share font data acrross widgets and thus need a font
// manager

enum class FontLoadError : uint8_t { InvalidPath, InvalidBytes, Loadfailed };

namespace impl {

struct FileTypefaceSourceData {
  std::filesystem::path path;
  std::string identifier;
};

struct MemoryTypefaceSourceData {
  std::vector<uint8_t> bytes;
  std::string identifier;
};

};  // namespace impl

struct FileTypefaceSource {
  explicit FileTypefaceSource(std::filesystem::path path) {
    std::string identifier = std::string("FileTypefaceSource{path: ") +
                             std::string(path) + std::string("}");
    data_ = std::make_shared<impl::FileTypefaceSourceData>(
        impl::FileTypefaceSourceData{std::move(path), std::move(identifier)});
  }

  std::shared_ptr<impl::FileTypefaceSourceData const> data() const {
    return data_;
  }

  bool operator==(FileTypefaceSource const& other) const {
    return data_->identifier == other.data_->identifier;
  }

  bool operator!=(FileTypefaceSource const& other) const {
    return !(*this == other);
  }

 private:
  std::shared_ptr<impl::FileTypefaceSourceData const> data_;
};

struct MemoryTypefaceSource {
  friend struct FontSourceProxy;

  explicit MemoryTypefaceSource(std::vector<uint8_t> bytes) {
    VLK_ENSURE(!bytes.empty());
    uint64_t uid = make_uid();
    std::string identifier = std::string("MemoryTypefaceSource{uid: ") +
                             std::to_string(uid) + std::string("}");
    data_ = std::make_shared<impl::MemoryTypefaceSourceData>(
        impl::MemoryTypefaceSourceData{std::move(bytes),
                                       std::move(identifier)});
  }

  std::shared_ptr<impl::MemoryTypefaceSourceData const> data() const {
    return data_;
  }

  bool operator==(MemoryTypefaceSource const& other) const {
    return data_->identifier == other.data_->identifier;
  }

  bool operator!=(MemoryTypefaceSource const& other) const {
    return !(*this == other);
  }

 private:
  static uint64_t make_uid();

  std::shared_ptr<impl::MemoryTypefaceSourceData const> data_;
};

namespace impl {
struct FileFontSourceData {
  std::vector<FileTypefaceSource> data;
  std::string identifier;
};

struct MemoryFontSourceData {
  std::vector<MemoryTypefaceSource> data;
  std::string identifier;
};

}  // namespace impl

struct FileFontSource {
  explicit FileFontSource(std::vector<FileTypefaceSource> typeface_sources) {
    std::string identifier = "FileFontSource{typefaces: [";

    for (auto const& typeface_source : typeface_sources) {
      identifier += "{file: " + typeface_source.data()->identifier + "}, ";
    }

    identifier += "]}";

    data_ = std::make_shared<impl::FileFontSourceData>(impl::FileFontSourceData{
        std::move(typeface_sources), std::move(identifier)});
  }

  std::shared_ptr<impl::FileFontSourceData const> data() const { return data_; }

  bool operator==(FileFontSource const& other) const {
    return data_->identifier == other.data_->identifier;
  }

  bool operator!=(FileFontSource const& other) const {
    return !(*this == other);
  }

 private:
  std::shared_ptr<impl::FileFontSourceData const> data_;
};

struct MemoryFontSource {
  explicit MemoryFontSource(
      std::vector<MemoryTypefaceSource> typeface_sources) {
    std::string identifier = "MemoryFontSource{typefaces: [";

    for (auto const& typeface_source : typeface_sources) {
      identifier += "{file: " + typeface_source.data()->identifier + "}, ";
    }

    identifier += "]}";

    data_ =
        std::make_shared<impl::MemoryFontSourceData>(impl::MemoryFontSourceData{
            std::move(typeface_sources), std::move(identifier)});
  }

  std::shared_ptr<impl::MemoryFontSourceData const> data() const {
    return data_;
  }

  bool operator==(MemoryFontSource const& other) const {
    return data_->identifier == other.data_->identifier;
  }

  bool operator!=(MemoryFontSource const& other) const {
    return !(*this == other);
  }

 private:
  std::shared_ptr<impl::MemoryFontSourceData const> data_;
};

namespace impl {
uint64_t get_typeface_size(sk_sp<SkTypeface const> const& typeface) {
  uint64_t byte_size = 0;
  uint64_t num_tables = typeface->countTables();
  std::vector<SkFontTableTag> table_tags;
  table_tags.resize(num_tables);
  for (int i = 0; i < num_tables; i++) {
    typeface->getTableTags(table_tags.data());
  }
  for (auto const& table_tag : table_tags) {
    byte_size += typeface->getTableSize(table_tag);
  }

  return byte_size;
}
}  // namespace impl

// TODO(lamarrr): ss
// this should probably just get the size of the text from the default font
// manager
struct TypefaceAsset : public Asset {
  TypefaceAsset(
      stx::Result<sk_sp<SkTypeface const>, FontLoadError>&& load_result)
      : load_result_{std::move(load_result)} {
    load_result_.as_cref().match(
        [&](sk_sp<SkTypeface const> const& typeface) {
          Asset::update_size(impl::get_typeface_size(typeface));
        },
        [&](FontLoadError error) { Asset::update_size(0); });
  }

  // TODO(lamarrr): rename get_ref
  auto const& get_ref() const { return load_result_; }

 private:
  stx::Result<sk_sp<SkTypeface const>, FontLoadError> load_result_;
};

// TODO(lamarrr): fonts must be persistent in memory once loaded

namespace impl {

struct FontLoadArgs : public AssetLoadArgs {
  explicit FontLoadArgs(
      std::shared_ptr<MemoryTypefaceSourceData const> source_data) {
    data_ = std::move(source_data);
  }

  explicit FontLoadArgs(
      std::shared_ptr<FileTypefaceSourceData const> source_data) {
    data_ = std::move(source_data);
  }

  explicit FontLoadArgs(
      std::shared_ptr<MemoryFontSourceData const> source_data) {
    data_ = std::move(source_data);
  }

  explicit FontLoadArgs(std::shared_ptr<FileFontSourceData const> source_data) {
    data_ = std::move(source_data);
  }

  bool is_mem_tf() const {
    return std::holds_alternative<
        std::shared_ptr<MemoryTypefaceSourceData const>>(data_);
  }

  bool is_file_tf() const {
    return std::holds_alternative<
        std::shared_ptr<FileTypefaceSourceData const>>(data_);
  }

  bool is_mem_fnt() const {
    return std::holds_alternative<std::shared_ptr<MemoryFontSourceData const>>(
        data_);
  }

  bool is_file_fnt() const {
    return std::holds_alternative<std::shared_ptr<FileFontSourceData const>>(
        data_);
  }

  auto const& source_data() const { return data_; }

 private:
  std::variant<std::shared_ptr<MemoryTypefaceSourceData const>,
               std::shared_ptr<FileTypefaceSourceData const>,
               std::shared_ptr<MemoryFontSourceData const>,
               std::shared_ptr<FileFontSourceData const>>
      data_;
};

struct FontAssetLoader : public AssetLoader {
  virtual std::unique_ptr<Asset> load(
      RasterContext const& context, AssetLoadArgs const& args) const override {
    sk_sp font_mgr = SkFontMgr::RefDefault();
    // font_mgr->makeFromData();
    FontLoadArgs const& load_args = upcast<FontLoadArgs>(args);
    if (load_args.is_mem_tf()) {
    } else if (load_args.is_mem_fnt()) {
    } else if (load_args.is_file_tf()) {
    } else if (load_args.is_file_fnt()) {
      auto const& source_data =
          std::get<std::shared_ptr<FileFontSourceData const>>(
              load_args.source_data());
      std::vector<TypefaceAsset> load_results;
      for (FileTypefaceSource const& tf_source : source_data->data) {
        // tf_source.data()->path
      }
    }
  }

 private:
  stx::Result<TypefaceAsset, FontLoadError> load_typeface_from_path(
      std::filesystem::path const& path);
  stx::Result<TypefaceAsset, FontLoadError> load_typeface_from_memory(
      stx::Span<uint8_t const> bytes) {
    sk_sp font_mgr = SkFontMgr::RefDefault();

    font_mgr->makeFromData(
        SkData::MakeWithoutCopy(bytes.data(), bytes.size_bytes()));
  }

  static std::shared_ptr<FontAssetLoader const> get_default();
};

}  // namespace impl

enum class TextDecoration : uint8_t {
  None = 0,
  Underline = 1,
  Overline = 2,
  StrikeThrough = 4,
};

VLK_DEFINE_ENUM_BIT_OPS(TextDecoration)

enum class FontSlant : uint8_t {
  Upright = 0,
  Italic = 1,
  Oblique = 2,
};

VLK_DEFINE_ENUM_BIT_OPS(FontSlant)

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
// TODO(lamarrr): review how this works with the new widget system
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

  TextProps get_props() const { return props_; }

  void update_text(std::string_view const& utf8_str) {
    utf8_text_ = utf8_str;
    paragraph_dirty_ = true;
  }

  void update_props(TextProps const& new_props) {
    props_ = new_props;
    paragraph_dirty_ = true;
  }

  // TODO(lamarrr): clip text if it height exceeds the maximum extent?
  virtual void draw(Canvas&, AssetManager&) override final;

  virtual void tick(std::chrono::nanoseconds) override final {
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
  TextProps props_;

  std::unique_ptr<skia::textlayout::Paragraph> paragraph_{nullptr};
  bool paragraph_dirty_ = true;

  void rebuild_paragraph_();
};

}  // namespace ui
}  // namespace vlk
