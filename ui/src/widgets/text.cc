

#include <algorithm>
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

using namespace vlk::ui;

// TODO(lamarrr): text splitting after layout. this would mean that the children
// would change and invoke a large re-layout and re-paint when they change,
// which is normal

SkPaint const& impl::ref_background_paint(
    TextProperties const& props) noexcept {
  return props.background_paint_;
}

SkPaint const& impl::ref_foreground_paint(
    TextProperties const& props) noexcept {
  return props.foreground_paint_;
}

STX_FORCE_INLINE static std::pair<skia::textlayout::ParagraphStyle,
                                  skia::textlayout::TextStyle>
to_skia_text_style(TextProperties const& front,
                   stx::Span<TextShadow const> const& shadows) {
  namespace sktext = skia::textlayout;

  sktext::ParagraphStyle paragraph_style;
  sktext::TextStyle text_style;

  // paragraph_style.setDrawOptions(
  //          DrawOptions::kDirect);  // ?

  paragraph_style.setMaxLines(front.line_limit());

  switch (front.align()) {
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

  switch (front.direction()) {
    case TextDirection::Ltr:
      paragraph_style.setTextDirection(sktext::TextDirection::kLtr);
      break;

    case TextDirection::Rtl:
      paragraph_style.setTextDirection(sktext::TextDirection::kRtl);
      break;
  }

  text_style.setBackgroundColor(impl::ref_background_paint(front));
  text_style.setForegroundColor(impl::ref_foreground_paint(front));

  sktext::TextDecoration decoration = sktext::TextDecoration::kNoDecoration;

  TextDecoration front_decorations = front.get_decorations();

  if (vlk::enum_ut_and(front_decorations, TextDecoration::Overline)) {
    decoration = vlk::enum_or(decoration, sktext::kOverline);
  }

  if (vlk::enum_ut_and(front_decorations, TextDecoration::StrikeThrough)) {
    decoration = vlk::enum_or(decoration, sktext::kLineThrough);
  }

  if (vlk::enum_ut_and(front_decorations, TextDecoration::Underline)) {
    decoration = vlk::enum_or(decoration, sktext::kUnderline);
  }

  text_style.setDecoration(decoration);

  switch (front.decoration_style()) {
    case DecorationStyle::Dashed:
      text_style.setDecorationStyle(sktext::kDashed);
      break;

    case DecorationStyle::Dotted:
      text_style.setDecorationStyle(sktext::kDotted);
      break;

    case DecorationStyle::Double:
      text_style.setDecorationStyle(sktext::kDouble);
      break;

    case DecorationStyle::Solid:
      text_style.setDecorationStyle(sktext::kSolid);
      break;

    case DecorationStyle::Wavy:
      text_style.setDecorationStyle(sktext::kWavy);
      break;
  }

  std::string front_font_family = front.font_family();
  text_style.setFontFamilies({SkString(front_font_family.c_str())});

  text_style.setFontSize(front.font_size());

  SkFontStyle::Slant slant = SkFontStyle::kUpright_Slant;

  switch (front.slant()) {
    case TextSlant::Italic:
      slant = SkFontStyle::kItalic_Slant;
      break;
    case TextSlant::Oblique:
      slant = SkFontStyle::kOblique_Slant;
      break;
    default:
      break;
  }

  SkFontStyle font_style{static_cast<int>(front.font_weight()),
                         static_cast<int>(SkFontStyle::kNormal_Width), slant};

  text_style.setFontStyle(font_style);

  text_style.setLetterSpacing(front.letter_spacing());
  std::string locale = front.locale();
  text_style.setLocale(SkString{locale.c_str()});
  text_style.setWordSpacing(front.word_spacing());

  for (auto const& shadow : shadows)
    text_style.addShadow(sktext::TextShadow{
        shadow.color.to_argb(), SkPoint::Make(shadow.offset.x, shadow.offset.y),
        shadow.blur_radius});

  return std::make_pair(std::move(paragraph_style), std::move(text_style));
}

Text::Text(std::string_view const& str, TextProperties const& properties,
           stx::Span<TextShadow const> const& shadows)
    : text_data_{str} {
  namespace sktext = skia::textlayout;
  auto [paragraph_style, text_style] = to_skia_text_style(properties, shadows);

  // TODO(lamarrr): global????
  sk_sp<sktext::FontCollection> font_collection =
      sk_make_sp<sktext::FontCollection>();
  VLK_ENSURE(font_collection != nullptr);

  // TODO(lamarrr): get system fonts list
  // global
  sk_sp<SkFontMgr> font_mgr = SkFontMgr::RefDefault();

  VLK_ENSURE(font_mgr != nullptr, "font manager is null");
  VLK_ENSURE(font_mgr->countFamilies() > 0, "No font family in font manager");
  // TODO(lamarrr): what does it do when it does not find the required font?

  font_collection->setDefaultFontManager(font_mgr);

  sktext::ParagraphBuilderImpl builder{paragraph_style, font_collection};

  builder.pushStyle(text_style);
  builder.addText(text_data_.data(), text_data_.size());
  builder.pop();  // pushStyle -> addText -> Pop (adds text in chunks)

  paragraph_ = builder.Build();

  VLK_ENSURE(paragraph_ != nullptr);
}

/*
Rect Text::compute_area(Extent const& allotted_extent,
                        [[maybe_unused]] stx::Span<Rect> const& children_area) {
  paragraph_->layout(allotted_extent.width);

  // it is laid out on the specified width above, its MaxIntrinsicWidth
  // specifies the max width if the whole text were to be laid out on a straight
  // line.
  uint32_t width = static_cast<uint32_t>(
      std::ceil(std::min(paragraph_->getMaxIntrinsicWidth(),
                         static_cast<float>(allotted_extent.width))));
  uint32_t height = static_cast<uint32_t>(std::ceil(paragraph_->getHeight()));

  return Rect{Offset{0, 0}, Extent{width, height}};
}

void Text::draw(Canvas& canvas,
                [[maybe_unused]] Extent const& requested_extent) {
  SkCanvas* sk_canvas = canvas.as_skia();
  paragraph_->paint(sk_canvas, 0, 0);
}
*/
