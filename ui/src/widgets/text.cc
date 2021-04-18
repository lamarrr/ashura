

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "vlk/ui/widgets/text.h"
#include "vlk/utils/utils.h"

#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

namespace ui = vlk::ui;
namespace sktext = skia::textlayout;

// TODO(lamarrr): text splitting after layout. this would mean that the children
// would change and invoke a large re-layout and re-paint when they change,
// which is normal

STX_FORCE_INLINE static SkFontStyle make_font_style(
    ui::TextProps const& props) {
  SkFontStyle::Weight weight =
      static_cast<SkFontStyle::Weight>(static_cast<int>(props.font_weight()));
  SkFontStyle::Width width =
      static_cast<SkFontStyle::Width>(static_cast<int>(props.font_width()));
  SkFontStyle::Slant slant = SkFontStyle::Slant::kUpright_Slant;

  switch (props.slant()) {
    case ui::FontSlant::Italic:
      slant = (SkFontStyle::Slant)(slant | SkFontStyle::kItalic_Slant);
      break;
    case ui::FontSlant::Oblique:
      slant = (SkFontStyle::Slant)(slant | SkFontStyle::kOblique_Slant);
      break;
    case ui::FontSlant::Upright:
      slant = (SkFontStyle::Slant)(slant | SkFontStyle::kUpright_Slant);
      break;

    default:
      VLK_PANIC("Unimplemented Font Slant");
  }

  return SkFontStyle{weight, width, slant};
}

STX_FORCE_INLINE static sktext::TextStyle make_text_style(
    ui::TextProps const& props) {
  sktext::TextStyle text_style;

  SkPaint foreground_paint;
  SkPaint background_paint;

  foreground_paint.setAntiAlias(props.antialias());
  foreground_paint.setColor(props.color().to_argb());

  background_paint.setAntiAlias(props.antialias());
  background_paint.setColor(props.background_color().to_argb());

  text_style.setForegroundColor(foreground_paint);
  text_style.setBackgroundColor(background_paint);

  sktext::TextDecoration decoration = sktext::TextDecoration::kNoDecoration;

  if ((props.decoration() & ui::TextDecoration::Overline) !=
      ui::TextDecoration::None) {
    decoration = (sktext::TextDecoration)(decoration |
                                          sktext::TextDecoration::kOverline);
  }

  if ((props.decoration() & ui::TextDecoration::StrikeThrough) !=
      ui::TextDecoration::None) {
    decoration = (sktext::TextDecoration)(decoration |
                                          sktext::TextDecoration::kLineThrough);
  }

  if ((props.decoration() & ui::TextDecoration::Underline) !=
      ui::TextDecoration::None) {
    decoration = (sktext::TextDecoration)(decoration |
                                          sktext::TextDecoration::kUnderline);
  }

  text_style.setDecoration(decoration);
  text_style.setDecorationColor(props.decoration_color().to_argb());

  switch (props.decoration_style()) {
    case ui::TextDecorationStyle::Dashed:
      text_style.setDecorationStyle(sktext::TextDecorationStyle::kDashed);
      break;
    case ui::TextDecorationStyle::Dotted:
      text_style.setDecorationStyle(sktext::TextDecorationStyle::kDotted);
      break;
    case ui::TextDecorationStyle::Double:
      text_style.setDecorationStyle(sktext::TextDecorationStyle::kDouble);
      break;
    case ui::TextDecorationStyle::Solid:
      text_style.setDecorationStyle(sktext::TextDecorationStyle::kSolid);
      break;
    case ui::TextDecorationStyle::Wavy:
      text_style.setDecorationStyle(sktext::TextDecorationStyle::kWavy);
      break;
    default:
      VLK_PANIC("Unimplemented Text Decoration Style");
      break;
  }

  text_style.setFontStyle(make_font_style(props));
  text_style.setFontSize(props.font_size());
  text_style.setLetterSpacing(props.letter_spacing());
  text_style.setWordSpacing(props.word_spacing());
  std::string const locale = props.locale();

  text_style.setLocale(SkString{locale.data(), locale.size()});

  text_style.setFontFamilies({SkString{"Arial"}});

  // fonts?
  sktext::FontCollection font_collection;

  return text_style;
}

STX_FORCE_INLINE static sktext::ParagraphStyle make_paragraph_style(
    ui::TextProps const& props) {
  sktext::ParagraphStyle paragraph_style;

  paragraph_style.setMaxLines(props.line_limit());

  switch (props.align()) {
    case ui::TextAlign::Center:
      paragraph_style.setTextAlign(sktext::TextAlign::kCenter);
      break;

    case ui::TextAlign::End:
      paragraph_style.setTextAlign(sktext::TextAlign::kEnd);
      break;

    case ui::TextAlign::Justify:
      paragraph_style.setTextAlign(sktext::TextAlign::kJustify);
      break;

    case ui::TextAlign::Left:
      paragraph_style.setTextAlign(sktext::TextAlign::kLeft);
      break;

    case ui::TextAlign::Right:
      paragraph_style.setTextAlign(sktext::TextAlign::kRight);
      break;

    case ui::TextAlign::Start:
      paragraph_style.setTextAlign(sktext::TextAlign::kStart);
      break;

    default:
      VLK_PANIC("Unimplemented Text Align");
  }

  switch (props.direction()) {
    case ui::TextDirection::Ltr:
      paragraph_style.setTextDirection(sktext::TextDirection::kLtr);
      break;

    case ui::TextDirection::Rtl:
      paragraph_style.setTextDirection(sktext::TextDirection::kRtl);
      break;

    default:
      VLK_PANIC("Unimplemented Text Direction");
  }

  paragraph_style.setTextStyle(make_text_style(props));
  // paragraph_style.setDrawOptions(DrawOptions::kDirect);

  return paragraph_style;
}

STX_FORCE_INLINE static std::unique_ptr<sktext::Paragraph> build_paragraph(
    std::string_view const& utf8_text, ui::TextProps const& props) {
  sk_sp<sktext::FontCollection> font_collection =
      sk_make_sp<sktext::FontCollection>();

  // TODO(lamarrr): get system fonts list
  // global
  sk_sp<SkFontMgr> font_mgr = SkFontMgr::RefDefault();

  VLK_ENSURE(font_mgr != nullptr, "Default Font Manager is null");
  VLK_WARN_IF(font_mgr->countFamilies() <= 0,
              "No Font Family found in default Font Manager");
  // TODO(lamarrr): what does it do when it does not find the required font?

  // TODO(lamarrr): global font collection?

  //
  //
  // Skia's Font collection contains several font managers. one can be the
  // default system font manager thhat loads fonts that belongs to the system,
  // and the others can be custom font managers that load fonts from other paths
  // or sources
  //
  // Fonts contain multiple typefaces (comes in different formats, mostly .ttf).
  // Typeface (i.e. Arial) is a particular glyph of a TypeFace (i.e. Arial,
  // Light, Italic).
  //
  //
  //
  //
  //
  //
  //
  //
  //

  font_collection->setDefaultFontManager(font_mgr);

  sktext::ParagraphStyle paragraph_style = make_paragraph_style(props);

  std::unique_ptr<sktext::ParagraphBuilder> paragraph_builder =
      sktext::ParagraphBuilderImpl::make(paragraph_style, font_collection);

  paragraph_builder->addText(utf8_text.data(), utf8_text.size());

  std::unique_ptr<sktext::Paragraph> paragraph = paragraph_builder->Build();

  VLK_ENSURE(paragraph != nullptr);

  return paragraph;
}

void ui::Text::rebuild_paragraph_() {
  paragraph_ = build_paragraph(utf8_text_, properties_);

  paragraph_->layout(0.0f);

  float const min_intrinsic_width = paragraph_->getMinIntrinsicWidth();
  float const max_intrinsic_width = paragraph_->getMaxIntrinsicWidth();

  paragraph_->layout(min_intrinsic_width);
  float const max_intrinsic_height = paragraph_->getHeight();
  paragraph_->layout(max_intrinsic_width);
  float const min_intrinsic_height = paragraph_->getHeight();

  // request for max extent but still layout for the given extent.
  // we need to introduce the notion of layout to the system, lambda callback or
  // text specific?
  ui::SelfExtent extent{};
  extent.width.min = static_cast<int64_t>(std::ceil(min_intrinsic_width));
  extent.width.max = static_cast<int64_t>(std::ceil(max_intrinsic_width));
  extent.width.bias = extent.width.max;

  extent.height.min = static_cast<int64_t>(std::ceil(min_intrinsic_height));
  extent.height.max = static_cast<int64_t>(std::ceil(max_intrinsic_height));
  extent.height.bias = extent.width.max;

  // TODO(lamarrr): make it not perform layout as we did and instead just
  // request for the max extent and 1.0 on the height and the apply the trimmer.

  //
  // it woudln't make sense text shrinker
  // trimmer_op can be aspect ratio as well

  Widget::update_self_extent(extent);
}

ui::Extent ui::Text::trim(ui::Extent const& extent) {
  VLK_ENSURE(paragraph_ != nullptr);
  paragraph_->layout(extent.width);
  return Extent{extent.width, static_cast<uint32_t>(
                                  std::ceil(this->paragraph_->getHeight()))};
}

void ui::Text::draw(Canvas& canvas) {
  VLK_ENSURE(paragraph_ != nullptr);
  VLK_DEBUG_ENSURE(!paragraph_dirty_);

  SkCanvas* sk_canvas = canvas.as_skia().expect("canvas backend is not skia");

  // if there's leftover space, we might need to perform another layout step or
  // add the notion of text layout to the system?

  // TODO(lamarrr): can we cache this?
  paragraph_->layout(canvas.extent().width);
  paragraph_->paint(sk_canvas, 0.0f, 0.0f);
}

//   // ?
// SkTypeface::MakeFromFile("");
// std::string front_font_family = front.font_family();
// text_style.setFontFamilies({SkString(front_font_family.c_str())});
