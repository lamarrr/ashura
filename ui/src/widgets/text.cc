

#include "vlk/ui/widgets/text.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "include/core/SkFontMgr.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "vlk/utils/utils.h"

namespace sktext = skia::textlayout;

namespace vlk {
namespace ui {

namespace impl {

constexpr WidgetDirtiness map_diff(TextDiff diff) {
  WidgetDirtiness dirtiness = WidgetDirtiness::None;

  if ((diff & TextDiff::Color) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::BgColor) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::FontSize) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::LetterSpacing) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::WordSpacing) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::Locale) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::Decoration) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::DecorationColor) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::DecorationStyle) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::Font) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::Antialias) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::Text) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::Direction) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::Align) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  if ((diff & TextDiff::LineLimit) != TextDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  return dirtiness;
}

template <typename PropsType>
inline TextDiff text_props_diff(PropsType const& a, PropsType const& b) {
  TextDiff diff = TextDiff::None;

  if (a.color() != b.color()) {
    diff |= TextDiff::Color;
  }

  if (a.background_color() != b.background_color()) {
    diff |= TextDiff::BgColor;
  }

  if (a.font_size() != b.font_size()) {
    diff |= TextDiff::FontSize;
  }

  if (a.letter_spacing() != b.letter_spacing()) {
    diff |= TextDiff::LetterSpacing;
  }

  if (a.word_spacing() != b.word_spacing()) {
    diff |= TextDiff::WordSpacing;
  }

  if (a.locale() != b.locale()) {
    diff |= TextDiff::Locale;
  }

  if (a.decoration() != b.decoration()) {
    diff |= TextDiff::Decoration;
  }

  if (a.decoration_color() != b.decoration_color()) {
    diff |= TextDiff::DecorationColor;
  }

  if (a.decoration_style() != b.decoration_style()) {
    diff |= TextDiff::DecorationStyle;
  }

  if (a.font_ref() != b.font_ref()) {
    diff |= TextDiff::Font;
  }

  if (a.antialias() != b.antialias()) {
    diff |= TextDiff::Antialias;
  }

  return diff;
}

inline TextDiff paragraph_props_diff(ParagraphProps const& a,
                                     ParagraphProps const& b) {
  TextDiff diff = TextDiff::None;

  if (a.direction() != b.direction()) {
    diff |= TextDiff::Direction;
  }

  if (a.align() != b.align()) {
    diff |= TextDiff::Align;
  }

  if (a.line_limit() != b.line_limit()) {
    diff |= TextDiff::LineLimit;
  }

  diff |= text_props_diff(a, b);

  return diff;
}

inline TextDiff inline_texts_diff(stx::Span<InlineTextStorage const> a,
                                  stx::Span<InlineText const> b) {
  TextDiff diff = TextDiff::None;

  if (a.size() != b.size()) {
    diff |= TextDiff::All;
    // immediately return since the text size has changed
    return diff;
  }

  for (size_t i = 0; i < a.size(); i++) {
    if (a[i].text != b[i].text) {
      diff |= TextDiff::Text;
    }

    diff |= text_props_diff(a[i].props, b[i].props);
  }

  return diff;
}

inline auto add_font_asset_helper(AssetManager& asset_manager,
                                  FontSource const& font_source)
    -> stx::Result<stx::NoneType, AssetError> {
  if (std::holds_alternative<SystemFont>(font_source)) {
    return add_font_asset(asset_manager, std::get<SystemFont>(font_source));
  } else if (std::holds_alternative<FileFont>(font_source)) {
    return add_font_asset(asset_manager, std::get<FileFont>(font_source));
  } else if (std::holds_alternative<MemoryFont>(font_source)) {
    return add_font_asset(asset_manager, std::get<MemoryFont>(font_source));
  } else if (std::holds_alternative<FileTypefaceSource>(font_source)) {
    return add_font_asset(asset_manager,
                          std::get<FileTypefaceSource>(font_source));
  } else if (std::holds_alternative<MemoryTypefaceSource>(font_source)) {
    return add_font_asset(asset_manager,
                          std::get<MemoryTypefaceSource>(font_source));
  } else {
    VLK_PANIC();
  }
}

inline auto get_font_asset_helper(AssetManager& asset_manager,
                                  FontSource const& font_source)
    -> stx::Result<std::shared_ptr<TypefaceAsset const>, AssetError> {
  if (std::holds_alternative<SystemFont>(font_source)) {
    return get_font_asset(asset_manager, std::get<SystemFont>(font_source));
  } else if (std::holds_alternative<FileFont>(font_source)) {
    return get_font_asset(asset_manager, std::get<FileFont>(font_source));
  } else if (std::holds_alternative<MemoryFont>(font_source)) {
    return get_font_asset(asset_manager, std::get<MemoryFont>(font_source));
  } else if (std::holds_alternative<FileTypefaceSource>(font_source)) {
    return get_font_asset(asset_manager,
                          std::get<FileTypefaceSource>(font_source));
  } else if (std::holds_alternative<MemoryTypefaceSource>(font_source)) {
    return get_font_asset(asset_manager,
                          std::get<MemoryTypefaceSource>(font_source));
  } else {
    VLK_PANIC();
  }
}

inline std::string_view get_source_identifier(FontSource const& font_source) {
  if (std::holds_alternative<SystemFont>(font_source)) {
    return std::get<SystemFont>(font_source).data_ref()->identifier;
  } else if (std::holds_alternative<FileFont>(font_source)) {
    return std::get<FileFont>(font_source).source.data_ref()->identifier;
  } else if (std::holds_alternative<MemoryFont>(font_source)) {
    return std::get<MemoryFont>(font_source).source.data_ref()->identifier;
  } else if (std::holds_alternative<FileTypefaceSource>(font_source)) {
    return std::get<FileTypefaceSource>(font_source).data_ref()->identifier;
  } else if (std::holds_alternative<MemoryTypefaceSource>(font_source)) {
    return std::get<MemoryTypefaceSource>(font_source).data_ref()->identifier;
  } else {
    VLK_PANIC();
  }
}

inline sktext::TextStyle make_text_style(
    impl::InlineTextStorage const& text_storage,
    impl::ParagraphStorage const& paragraph_storage) {
  sktext::TextStyle text_style;

  SkPaint foreground_paint;
  SkPaint background_paint;

  auto const& props = text_storage.props;
  auto const& default_props = paragraph_storage.props;

  foreground_paint.setAntiAlias(
      props.antialias().unwrap_or(default_props.antialias()));

  foreground_paint.setColor(
      props.color().unwrap_or(default_props.color()).to_argb());

  background_paint.setAntiAlias(
      props.antialias().unwrap_or(default_props.antialias()));

  background_paint.setColor(props.background_color()
                                .unwrap_or(default_props.background_color())
                                .to_argb());

  text_style.setForegroundColor(foreground_paint);
  text_style.setBackgroundColor(background_paint);

  sktext::TextDecoration decoration = sktext::TextDecoration::kNoDecoration;

  if ((props.decoration().unwrap_or(default_props.decoration()) &
       TextDecoration::Overline) != TextDecoration::None) {
    decoration = (sktext::TextDecoration)(decoration |
                                          sktext::TextDecoration::kOverline);
  }

  if ((props.decoration().unwrap_or(default_props.decoration()) &
       TextDecoration::StrikeThrough) != TextDecoration::None) {
    decoration = (sktext::TextDecoration)(decoration |
                                          sktext::TextDecoration::kLineThrough);
  }

  if ((props.decoration().unwrap_or(default_props.decoration()) &
       TextDecoration::Underline) != TextDecoration::None) {
    decoration = (sktext::TextDecoration)(decoration |
                                          sktext::TextDecoration::kUnderline);
  }

  text_style.setDecoration(decoration);
  text_style.setDecorationColor(props.decoration_color()
                                    .unwrap_or(default_props.decoration_color())
                                    .to_argb());

  switch (
      props.decoration_style().unwrap_or(default_props.decoration_style())) {
    case TextDecorationStyle::Dashed:
      text_style.setDecorationStyle(sktext::kDashed);
      break;
    case TextDecorationStyle::Dotted:
      text_style.setDecorationStyle(sktext::kDotted);
      break;
    case TextDecorationStyle::Double:
      text_style.setDecorationStyle(sktext::kDouble);
      break;
    case TextDecorationStyle::Solid:
      text_style.setDecorationStyle(sktext::kSolid);
      break;
    case TextDecorationStyle::Wavy:
      text_style.setDecorationStyle(sktext::kWavy);
      break;
  }

  auto const& font_source = props.font_ref();

  if (font_source.is_some()) {
    if (text_storage.state == TextState::FontsLoadDone &&
        text_storage.typeface.is_some()) {
      std::string_view source_identifier =
          get_source_identifier(font_source.value());
      text_style.setFontFamilies(std::vector{
          SkString{source_identifier.data(), source_identifier.size()}});
    } else {
    }
  } else {
    if (paragraph_storage.state == TextState::FontsLoadDone &&
        paragraph_storage.typeface.is_some()) {
      std::string_view source_identifier =
          get_source_identifier(paragraph_storage.props.font_ref());
      text_style.setFontFamilies(std::vector{
          SkString{source_identifier.data(), source_identifier.size()}});
    }
  }

  text_style.setFontSize(
      props.font_size().unwrap_or(default_props.font_size()));
  text_style.setLetterSpacing(
      props.letter_spacing().unwrap_or(default_props.letter_spacing()));
  text_style.setWordSpacing(
      props.word_spacing().unwrap_or(default_props.word_spacing()));

  props.locale().match(
      [&](std::string locale) {
        if (!locale.empty()) {
          text_style.setLocale(SkString{locale.data(), locale.size()});
        }
      },
      [&]() {
        std::string default_props_locale = default_props.locale();
        if (!default_props_locale.empty()) {
          text_style.setLocale(SkString{default_props_locale.data(),
                                        default_props_locale.size()});
        }
      });

  return text_style;
}

inline sktext::ParagraphStyle make_paragraph_style(
    ParagraphProps const& paragraph_props) {
  sktext::ParagraphStyle paragraph_style;

  switch (paragraph_props.direction()) {
    case TextDirection::Ltr:
      paragraph_style.setTextDirection(sktext::TextDirection::kLtr);
      break;

    case TextDirection::Rtl:
      paragraph_style.setTextDirection(sktext::TextDirection::kRtl);
      break;
  }

  switch (paragraph_props.align()) {
    case TextAlign::Center:
      paragraph_style.setTextAlign(sktext::TextAlign::kCenter);
      break;

    case TextAlign::End:
      paragraph_style.setTextAlign(sktext::TextAlign::kEnd);
      break;

    case TextAlign::Justify:
      paragraph_style.setTextAlign(sktext::TextAlign::kJustify);
      break;

    case TextAlign::Left:
      paragraph_style.setTextAlign(sktext::TextAlign::kLeft);
      break;

    case TextAlign::Right:
      paragraph_style.setTextAlign(sktext::TextAlign::kRight);
      break;

    case TextAlign::Start:
      paragraph_style.setTextAlign(sktext::TextAlign::kStart);
      break;
  }

  paragraph_style.setMaxLines(paragraph_props.line_limit());

  return paragraph_style;
}

inline std::unique_ptr<sktext::Paragraph> build_paragraph(
    stx::Span<impl::InlineTextStorage const> inline_texts,
    impl::ParagraphStorage const& paragraph_storage) {
  // Skia's Font collection contains several font managers. one can be the
  // default system font manager that loads fonts that belongs to the system,
  // and the others can be custom font managers that load fonts from other paths
  // or sources
  //
  // Fonts contain multiple typefaces. Font for example is Arial.
  //
  // Typeface is a variant of the Font. Example is Arial Bold Ultracondensed.
  // Typefaces are typically stored into files (i.e. .ttf, .woff files)
  //

  sktext::ParagraphStyle paragraph_style =
      make_paragraph_style(paragraph_storage.props);

  sk_sp<sktext::FontCollection> font_collection =
      sk_make_sp<sktext::FontCollection>();

  font_collection->getParagraphCache()->turnOn(false);

  sk_sp font_provider = sk_make_sp<sktext::TypefaceFontProvider>();

  for (auto const& inline_text : inline_texts) {
    inline_text.typeface.as_ref().match(
        [&](std::shared_ptr<TypefaceAsset const> const& typeface) {
          typeface->load_result_ref().as_cref().match(
              [&](sk_sp<SkTypeface> sk_typeface) {
                FontSource const& font =
                    inline_text.props.font_ref().as_cref().unwrap_or(
                        paragraph_storage.props.font_ref());
                std::string_view identifier = get_source_identifier(font);
                font_provider->registerTypeface(
                    std::move(sk_typeface),
                    SkString{identifier.data(), identifier.size()});
              },
              [](auto error) {});
        },
        []() {});
  }

  font_collection->setAssetFontManager(font_provider);

  sk_sp<SkFontMgr> font_mgr = SkFontMgr::RefDefault();

  if (font_mgr != nullptr) font_collection->setDefaultFontManager(font_mgr);

  std::unique_ptr<sktext::ParagraphBuilder> paragraph_builder =
      sktext::ParagraphBuilderImpl::make(paragraph_style, font_collection);

  VLK_ENSURE(paragraph_builder != nullptr);

  for (auto const& inline_text : inline_texts) {
    sktext::TextStyle const text_style =
        make_text_style(inline_text, paragraph_storage);
    paragraph_builder->pushStyle(text_style);
    paragraph_builder->addText(inline_text.text.data(),
                               inline_text.text.size());
  }

  std::unique_ptr<sktext::Paragraph> paragraph = paragraph_builder->Build();

  VLK_ENSURE(paragraph != nullptr);

  return paragraph;
}

}  // namespace impl

void Text::rebuild_paragraph() {
  paragraph_ = impl::build_paragraph(inline_texts_, paragraph_storage_);
  VLK_ENSURE(paragraph_ != nullptr);

  // perform layout pass to get minimum and maximum width
  paragraph_->layout(0.0f);

  float const min_intrinsic_width = paragraph_->getMinIntrinsicWidth();
  float const max_intrinsic_width = paragraph_->getMaxIntrinsicWidth();

  // perform another layout pass to get the (maximum length, minimum height)
  // values
  paragraph_->layout(min_intrinsic_width);
  float const max_intrinsic_height = paragraph_->getHeight();

  // perform another layout pass to get the (minimum length, maximum height)
  // values
  paragraph_->layout(max_intrinsic_width);
  float const min_intrinsic_height = paragraph_->getHeight();

  // we request for max extent but still layout for the given extent.
  SelfExtent extent{};
  extent.width.min = static_cast<int64_t>(std::ceil(min_intrinsic_width));
  extent.width.max = static_cast<int64_t>(std::ceil(max_intrinsic_width));
  extent.width.bias = extent.width.max;

  extent.height.min = static_cast<int64_t>(std::ceil(min_intrinsic_height));
  extent.height.max = static_cast<int64_t>(std::ceil(max_intrinsic_height));
  extent.height.bias = extent.width.max;

  // NOTE: only updated if extent actually changes, in which reflow would occur
  Widget::update_self_extent(extent);
}

void Text::update_text(std::vector<InlineText> inline_texts) {
  diff_ |= impl::inline_texts_diff(inline_texts_, inline_texts);

  inline_texts_.resize(0);

  for (InlineText& inline_text : inline_texts)
    inline_texts_.push_back(impl::InlineTextStorage{
        std::move(inline_text.text), std::move(inline_text.props), stx::None,
        TextState::Begin});
}

void Text::update_paragraph_props(ParagraphProps paragraph_props) {
  Widget::update_needs_trimming(true);
  diff_ |=
      impl::paragraph_props_diff(paragraph_storage_.props, paragraph_props);

  // this means the inline text's style and font might change
  paragraph_storage_ = impl::ParagraphStorage{std::move(paragraph_props),
                                              stx::None, TextState::Begin};
}

Extent Text::trim(Extent extent) {
  VLK_ENSURE(paragraph_ != nullptr);

  // whatever width is given to the text widget is used and its height is
  // trimmed to fit the text's height
  paragraph_->layout(extent.width);
  return Extent{extent.width,
                static_cast<uint32_t>(std::ceil(paragraph_->getHeight()))};
}

void Text::draw(Canvas& canvas) {
  VLK_ENSURE(paragraph_ != nullptr);

  SkCanvas& sk_canvas = canvas.to_skia();

  sk_canvas.save();

  Extent const widget_extent = canvas.extent();

  // the text might overflow so we clip just in case it actually does
  sk_canvas.clipRect(SkRect::MakeWH(widget_extent.width, widget_extent.height));

  // if there's leftover space, we might need to perform another layout step or
  // add the notion of text layout to the system?
  paragraph_->paint(&sk_canvas, 0.0f, 0.0f);

  sk_canvas.restore();
}

void Text::tick(std::chrono::nanoseconds, AssetManager& asset_manager) {
  // fonts are loaded immediately, irregardless of whether they are in use or
  // not
  if (paragraph_storage_.state == TextState::Begin) {
    impl::add_font_asset_helper(asset_manager,
                                paragraph_storage_.props.font_ref())
        .match(
            [&](stx::NoneType) {
              paragraph_storage_.state = TextState::FontsLoading;
            },
            [&](AssetError error) {
              switch (error) {
                case AssetError::TagExists:
                  paragraph_storage_.state = TextState::FontsLoading;
                  break;
                default:
                  VLK_PANIC("Unexpected State");
              }
            });
  }

  if (paragraph_storage_.state == TextState::FontsLoading) {
    paragraph_storage_.state =
        impl::get_font_asset_helper(asset_manager,
                                    paragraph_storage_.props.font_ref())
            .match(
                [&](std::shared_ptr<TypefaceAsset const>&& typeface) {
                  return typeface->load_result_ref().match(
                      [&](auto&) {
                        paragraph_storage_.typeface =
                            stx::Some(std::move(typeface));
                        return TextState::FontsLoadDone;
                      },
                      [&](FontLoadError error) {
                        VLK_WARN("Failed to load font for {}, error: {}",
                                 format(*this), format(error));
                        return TextState::FontsLoadDone;
                      });
                },
                [](AssetError error) {
                  switch (error) {
                      // image is still loading
                    case AssetError::IsLoading:
                      return TextState::FontsLoading;
                    default:
                      VLK_PANIC("Unexpected State");
                  }
                });

    if (paragraph_storage_.state == TextState::FontsLoadDone) {
      diff_ |= impl::TextDiff::Font;
    }
  }

  for (auto& inline_text : inline_texts_) {
    if (inline_text.state == TextState::Begin) {
      impl::add_font_asset_helper(
          asset_manager,
          inline_text.props.font().unwrap_or(paragraph_storage_.props.font()))
          .match(
              [&](stx::NoneType) {
                inline_text.state = TextState::FontsLoading;
              },
              [&](AssetError error) {
                switch (error) {
                  case AssetError::TagExists:
                    inline_text.state = TextState::FontsLoading;
                    break;
                  default:
                    VLK_PANIC("Unexpected State");
                }
              });
    }

    if (inline_text.state == TextState::FontsLoading) {
      inline_text.state =
          impl::get_font_asset_helper(asset_manager,
                                      inline_text.props.font().unwrap_or(
                                          paragraph_storage_.props.font()))
              .match(
                  [&](std::shared_ptr<TypefaceAsset const>&& typeface) {
                    return typeface->load_result_ref().match(
                        [&](auto&) {
                          inline_text.typeface = stx::Some(std::move(typeface));
                          return TextState::FontsLoadDone;
                        },
                        [&](FontLoadError error) {
                          VLK_WARN("Failed to load font for {}, error: {}",
                                   format(*this), format(error));
                          return TextState::FontsLoadDone;
                        });
                  },
                  [](AssetError error) {
                    switch (error) {
                        // image is still loading
                      case AssetError::IsLoading:
                        return TextState::FontsLoading;
                      default:
                        VLK_PANIC("Unexpected State");
                    }
                  });

      if (inline_text.state == TextState::FontsLoadDone) {
        diff_ |= impl::TextDiff::Font;
      }
    }
  }

  if (diff_ != impl::TextDiff::None) {
    rebuild_paragraph();
    WidgetDirtiness dirtiness = impl::map_diff(diff_);
    Widget::add_dirtiness(dirtiness);
    diff_ = impl::TextDiff::None;
  }
}

}  // namespace ui
}  // namespace vlk
