#pragma once

#include <algorithm>
#include <utility>

#include "vlk/ui2d/primitives.h"
#include "vlk/utils/utils.h"

#include "stx/option.h"

#include "include/core/SkMaskFilter.h"  // src
#include "include/core/SkRRect.h"

namespace vlk {
namespace ui2d {

// TODO(lamarrr): Add z-index tracking. what about stateful widgets?
// widget popping.
// spatial input detection on overlapping widgets. choose one with higher
// z-index

struct Gradient {
  // linear, radial, sweep
};

struct BoxAlignment {
  RelativeOffset offset;
};

namespace box_alignment {
constexpr BoxAlignment TopLeft = BoxAlignment({0.0f, 0.0f});
constexpr BoxAlignment TopCenter = BoxAlignment({0.5f, 0.0f});
constexpr BoxAlignment TopRight = BoxAlignment({1.0f, 0.f});
constexpr BoxAlignment CenterLeft = BoxAlignment({0.0f, 0.5f});
constexpr BoxAlignment CenterCenter = BoxAlignment({0.5f, 0.5f});
constexpr BoxAlignment CenterRight = BoxAlignment({1.0f, 0.5f});
constexpr BoxAlignment BottomLeft = BoxAlignment({0.f, 1.0f});
constexpr BoxAlignment BottomCenter = BoxAlignment({0.5f, 1.0f});
constexpr BoxAlignment BottomRight = BoxAlignment({1.0f, 1.0f});
};  // namespace box_alignment

struct Padding : public TopRightBottomLeft {
  static constexpr Padding uniform(uint32_t value) noexcept {
    return Padding{value, value, value, value};
  }

  static constexpr Padding xy(uint32_t x, uint32_t y) noexcept {
    return Padding{y, x, y, x};
  }

  static constexpr Padding trbl(uint32_t t, uint32_t r, uint32_t b,
                                uint32_t l) noexcept {
    return Padding{t, r, b, l};
  }
};

struct Border : public TopRightBottomLeft {
  Color color;

  static constexpr Border uniform(Color color, uint32_t value) noexcept {
    return Border{value, value, value, value, color};
  }

  static constexpr Border xy(Color color, uint32_t x, uint32_t y) noexcept {
    return Border{y, x, y, x, color};
  }

  static constexpr Border trbl(Color color, uint32_t t, uint32_t r, uint32_t b,
                               uint32_t l) noexcept {
    return Border{t, r, b, l, color};
  }
};

struct BorderRadius {
  uint32_t top_left;
  uint32_t top_right;
  uint32_t bottom_right;
  uint32_t bottom_left;

  static constexpr BorderRadius uniform(uint32_t value) noexcept {
    return BorderRadius{value, value, value, value};
  }

  static constexpr BorderRadius across(uint32_t tl_br,
                                       uint32_t tr_bl) noexcept {
    return BorderRadius{tl_br, tr_bl, tl_br, tr_bl};
  }

  static constexpr BorderRadius spec(uint32_t tl, uint32_t tr, uint32_t br,
                                     uint32_t bl) noexcept {
    return BorderRadius{tl, tr, br, bl};
  }
};

enum class BoxLayout : uint8_t {
  Fit,
  ExpandVertical,
  ExpandHorizontal,
  Expand
};

struct BoxProperties {
  constexpr BoxProperties padding(Padding const &value) const noexcept {
    BoxProperties out{*this};
    out.padding_ = value;
    return out;
  }

  constexpr BoxProperties padding(uint32_t value) const noexcept {
    return padding(Padding::uniform(value));
  }

  constexpr BoxProperties padding(uint32_t x, uint32_t y) const noexcept {
    return padding(Padding::xy(x, y));
  }

  constexpr BoxProperties padding(uint32_t top, uint32_t right, uint32_t bottom,
                                  uint32_t left) const noexcept {
    return padding(Padding::trbl(top, right, bottom, left));
  }

  constexpr Padding padding() const noexcept { return padding_; }

  constexpr BoxProperties border(Border const &value) const noexcept {
    BoxProperties out{*this};
    out.border_ = value;
    return out;
  }

  constexpr BoxProperties border(Color color, uint32_t value) const noexcept {
    return border(Border::uniform(color, value));
  }

  constexpr BoxProperties border(Color color, uint32_t x, uint32_t y) const
      noexcept {
    return border(Border::xy(color, x, y));
  }

  constexpr BoxProperties border(Color color, uint32_t top, uint32_t right,
                                 uint32_t bottom, uint32_t left) const
      noexcept {
    return border(Border::trbl(color, top, right, bottom, left));
  }

  constexpr Border border() const noexcept { return border_; }

  constexpr BoxProperties border_radius(BorderRadius const &value) const
      noexcept {
    BoxProperties out{*this};
    out.border_radius_ = value;
    return out;
  }

  constexpr BoxProperties border_radius(uint32_t value) const noexcept {
    return border_radius(BorderRadius::uniform(value));
  }

  constexpr BoxProperties border_radius(uint32_t tl_br, uint32_t tr_bl) const
      noexcept {
    return border_radius(BorderRadius::across(tl_br, tr_bl));
  }

  constexpr BoxProperties border_radius(uint32_t tl, uint32_t tr, uint32_t br,
                                        uint32_t bl) const noexcept {
    return border_radius(BorderRadius::spec(tl, tr, br, bl));
  }

  constexpr BorderRadius border_radius() const noexcept {
    return border_radius_;
  }

  constexpr BoxProperties layout(BoxLayout value) const noexcept {
    BoxProperties out{*this};
    out.layout_ = value;
    return out;
  }

  constexpr BoxLayout layout() const noexcept { return layout_; }

  constexpr BoxProperties align(BoxAlignment const &alignment) const noexcept {
    BoxProperties out{*this};
    out.alignment_ = alignment;
    return out;
  }

  constexpr BoxAlignment align() const noexcept { return alignment_; }

 private:
  Padding padding_ = Padding::uniform(0);
  Border border_ = Border::uniform(colors::Transparent, 0);
  BorderRadius border_radius_ = BorderRadius::uniform(0);
  BoxLayout layout_ = BoxLayout::Fit;
  BoxAlignment alignment_ = box_alignment::TopLeft;
};

struct BoxDecoration {
  struct Image {
    data::Image2D image = {};
    Normalized<float> opacity = 1.0f;
    Sizing sizing = Sizing::relative(1.0f, 1.0f);
    Stretch stretch = Stretch::None;
  };

  enum class DrawOrder : uint8_t { None, ImageFirst, ColorFirst };

  BoxDecoration color(Color const &value) && noexcept {
    [[maybe_unused]] auto previous = background_color_.replace(value);
    if (draw_order_ == DrawOrder::None) draw_order_ = DrawOrder::ColorFirst;
    return std::move(*this);
  }

  stx::Option<Color> color() const noexcept {
    return background_color_.clone();
  }

  BoxDecoration image(Image &&image) && noexcept {
    if (draw_order_ == DrawOrder::None) draw_order_ = DrawOrder::ImageFirst;
    [[maybe_unused]] auto previous =
        background_image_.replace(std::move(image));
    return std::move(*this);
  }

  BoxDecoration image(data::Image2D &&image, Normalized<float> opacity,
                      Sizing sizing, Stretch stretch) &&
      noexcept {
    return std::move(*this).image(
        Image{std::move(image), opacity, sizing, stretch});
  }

  BoxDecoration image(data::Image2D &&image, Normalized<float> opacity,
                      Sizing sizing) &&
      noexcept {
    return std::move(*this).image(Image{std::move(image), opacity, sizing});
  }

  BoxDecoration image(data::Image2D &&image, Normalized<float> opacity) &&
      noexcept {
    return std::move(*this).image(Image{std::move(image), opacity});
  }

  BoxDecoration image(data::Image2D &&image) && noexcept {
    return std::move(*this).image(Image{std::move(image)});
  }

  stx::Option<stx::ConstRef<Image>> image() const noexcept {
    return background_image_.as_cref();
  }

  BoxDecoration blur(float gaussian_blur_sigma) && noexcept {
    VLK_DEBUG_ENSURE(gaussian_blur_sigma > 0.0f,
                     "Gaussian Blur Sigma must be greater than 0.0f");
    [[maybe_unused]] auto previous =
        gaussian_blur_.replace(gaussian_blur_sigma);
    return std::move(*this);
  }

  stx::Option<float> blur() const noexcept { return gaussian_blur_.clone(); }

  BoxDecoration draw_order(DrawOrder order) && noexcept {
    draw_order_ = order;
    return std::move(*this);
  }

  DrawOrder draw_order() const noexcept { return draw_order_; }

 private:
  DrawOrder draw_order_ = DrawOrder::None;
  // draw color, clip rrect
  stx::Option<Color> background_color_ = stx::None;
  // draw image, clip rrect
  stx::Option<Image> background_image_ = stx::None;
  // must be greater than 0 if set
  stx::Option<float> gaussian_blur_ = stx::None;
};

/* sk_sp<SkImageFilter> filters[] = {
        nullptr,
        SkImageFilters::DropShadow(7.0f, 0.0f, 0.0f, 3.0f, SK_ColorBLUE,
   nullptr), SkImageFilters::DropShadow(0.0f, 7.0f, 3.0f, 0.0f, SK_ColorBLUE,
   nullptr), SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
   nullptr), SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
   std::move(cfif)), SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f,
   SK_ColorBLUE, nullptr, &cropRect),
        SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
   nullptr, &bogusRect), SkImageFilters::DropShadowOnly(7.0f, 7.0f, 3.0f, 3.0f,
   SK_ColorBLUE, nullptr),
    };
    */
struct BoxShadow {
  // greater than or equal to 0
  float blur_radius;
  // greater than 0
  float blur_sigma;
};

template <bool IsStateful, typename ChildDeleter = DefaultWidgetChildDeleter,
          typename BoxShadowAllocator = std::allocator<BoxShadow>>
struct BasicBox : public Widget {
  BasicBox(Widget *child, BoxProperties const &properties,
           BoxDecoration &&decoration = BoxDecoration{},
           stx::Span<BoxShadow const> const &shadows = {})
      : children_{child},
        properties_{properties},
        decoration_{std::move(decoration)},
        shadows_{shadows.begin(), shadows.end()} {}

  ~BasicBox() { ChildDeleter{}(children_[0]); }

  virtual Rect compute_area(Extent const &allotted_extent,
                            stx::Span<Rect> const &children_area) override {
    // - Get the available extent
    // - cut border and padding from the available extent
    // - query the child's extent using the remaining extent
    // -
    Padding padding = properties_.padding();
    Border border = properties_.border();

    //////////////////////////////////////////////////////////////////
    // REPEATED
    //////////////////////////////////////////////////////////////////
    uint32_t border_padding_x =
        std::min(allotted_extent.width,
                 padding.left + padding.right + border.left + border.right);
    uint32_t border_padding_y =
        std::min(allotted_extent.height,
                 padding.top + padding.bottom + border.top + border.bottom);

    uint32_t padding_x = border_padding_x -
                         std::min(border_padding_x, border.left + border.right);
    uint32_t padding_y = border_padding_y -
                         std::min(border_padding_y, border.top + border.bottom);
    uint32_t border_x = border_padding_x - padding_x;
    uint32_t border_y = border_padding_y - padding_y;

    uint32_t padding_right = padding_x - std::min(padding_x, padding.left);
    uint32_t padding_left = padding_x - padding_right;
    uint32_t padding_bottom = padding_y - std::min(padding_y, padding.top);
    uint32_t padding_top = padding_y - padding_bottom;

    uint32_t border_right = border_x - std::min(border_x, border.left);
    uint32_t border_left = border_x - border_right;
    uint32_t border_bottom = border_y - std::min(border_y, border.top);
    uint32_t border_top = border_y - border_bottom;

    //////////////////////////////////////////////////////////////////

    std::vector<Rect> child_children_area;
    child_children_area.resize(children_[0]->get_children().size());

    // give child the maximum available area after slicing off border and
    // padding
    Extent allotted_child_extent = {0, 0};
    allotted_child_extent.width = allotted_extent.width - border_padding_x;
    allotted_child_extent.height = allotted_extent.height - border_padding_y;

    Rect child_area =
        children_[0]->compute_area(allotted_child_extent, child_children_area);

    // clamp to area
    uint32_t child_offset_x =
        std::min(allotted_child_extent.width, child_area.offset.x);
    uint32_t child_offset_y =
        std::min(allotted_child_extent.height, child_area.offset.y);

    uint32_t child_width = std::min(
        allotted_child_extent.width - child_offset_x, child_area.extent.width);
    uint32_t child_height =
        std::min(allotted_child_extent.height - child_offset_y,
                 child_area.extent.height);

    children_area[0] =
        Rect{Offset{border_left + padding_left, border_top + padding_top},
             Extent{child_width, child_height}};

    Offset offset = {0, 0};
    Extent extent = {0, 0};

    if (properties_.layout() == BoxLayout::ExpandHorizontal ||
        properties_.layout() == BoxLayout::Expand) {
      extent.width = allotted_extent.width;
    } else {
      extent.width = border_padding_x + child_offset_x + child_width;
    }

    if (properties_.layout() == BoxLayout::ExpandVertical ||
        properties_.layout() == BoxLayout::Expand) {
      extent.height = allotted_extent.height;
    } else {
      extent.height = border_padding_y + child_offset_y + child_height;
    }

    return Rect{offset, extent};
  }

  virtual bool is_dirty() const noexcept override { return false; }
  virtual void mark_clean() noexcept override {
    // no-op
  }

  virtual bool is_stateful() const noexcept override { return IsStateful; }
  virtual bool is_layout_type() const noexcept override {
    // TODO(lamarrr): if has nothing tho draw
    return false;
  }

  virtual stx::Span<Widget *const> get_children() const noexcept override {
    return children_;
  }

  virtual void draw(Canvas &canvas, Extent const &requested_extent) override {
    auto sk_canvas = canvas.as_skia();

    auto border = properties_.border();
    auto border_radius = properties_.border_radius();
    auto padding = properties_.padding();

    //////////////////////////////////////////////////////////////////
    // REPEATED
    //////////////////////////////////////////////////////////////////
    uint32_t border_padding_x =
        std::min(requested_extent.width,
                 padding.left + padding.right + border.left + border.right);
    uint32_t border_padding_y =
        std::min(requested_extent.height,
                 padding.top + padding.bottom + border.top + border.bottom);

    uint32_t padding_x = border_padding_x -
                         std::min(border_padding_x, border.left + border.right);
    uint32_t padding_y = border_padding_y -
                         std::min(border_padding_y, border.top + border.bottom);
    uint32_t border_x = border_padding_x - padding_x;
    uint32_t border_y = border_padding_y - padding_y;

    uint32_t padding_right = padding_x - std::min(padding_x, padding.left);
    uint32_t padding_left = padding_x - padding_right;
    uint32_t padding_bottom = padding_y - std::min(padding_y, padding.top);
    uint32_t padding_top = padding_y - padding_bottom;

    uint32_t border_right = border_x - std::min(border_x, border.left);
    uint32_t border_left = border_x - border_right;
    uint32_t border_bottom = border_y - std::min(border_y, border.top);
    uint32_t border_top = border_y - border_bottom;

    uint32_t content_width =
        requested_extent.width - border_left - border_right;
    uint32_t content_height =
        requested_extent.height - border_top - border_bottom;
    //////////////////////////////////////////////////////////////////

    // TODO(lamarrr): abstract these preparations
    SkPaint content_paint;
    content_paint.setAntiAlias(true);
    SkPaint border_paint;
    border_paint.setAntiAlias(true);
    border_paint.setColor(border.color.argb());

    decoration_.blur().match(
        [&](SkScalar sigma) {
          content_paint.setMaskFilter(
              SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, sigma));
        },
        []() {});

    SkRRect content_rrect;
    SkRRect border_rrect;

    const SkVector content_radii[] = {
        SkVector::Make(border_radius.top_left, border_radius.top_left),
        SkVector::Make(border_radius.top_right, border_radius.top_right),
        SkVector::Make(border_radius.bottom_left, border_radius.bottom_left),
        SkVector::Make(border_radius.bottom_right, border_radius.bottom_right),
    };

    const SkVector border_radii[] = {
        SkVector::Make(border_radius.top_left + border_left,
                       border_radius.top_left + border_top),
        SkVector::Make(border_radius.top_right + border_right,
                       border_radius.top_right + border_top),
        SkVector::Make(border_radius.bottom_left + border_left,
                       border_radius.bottom_left + border_bottom),
        SkVector::Make(border_radius.bottom_right + border_right,
                       border_radius.bottom_right + border_bottom),
    };

    content_rrect.setRectRadii(SkRect::MakeXYWH(border_left, border_top,
                                                content_width, content_height),
                               content_radii);
    border_rrect.setRectRadii(
        SkRect::MakeXYWH(0, 0, requested_extent.width, requested_extent.height),
        border_radii);

    sk_canvas->save();
    sk_canvas->clipRRect(content_rrect, true);

    auto image_draw_op = [&] {
      decoration_.image().match(
          [&](auto imref) {
            // SKia expects pixels to be aligned on a 32-bit word.
            BoxDecoration::Image const &decoration_image = imref.get();
            data::Image2D const &image = decoration_image.image;
            VLK_ENSURE(image.width() != 0 && image.height() != 0 &&
                       image.channels() != 0);

            Sizing const &sizing = decoration_image.sizing;
            Stretch stretch = decoration_image.stretch;

            SkColorType color_type;

            switch (image.format()) {
              case data::Image2D::Format::RGBA:
                color_type = SkColorType::kRGBA_8888_SkColorType;
                break;
              default:
                VLK_PANIC("Unimplemented Decoration Box Image Format",
                          static_cast<int>(image.format()));
            }

            // TODO(lamarrr): fix alpha type
            // TODO(lamarrr): fix asset loader to use u32_t word alignment? How
            // will this affect Vulkan? does Vulkan also require u32_t
            // alignment?
            // TODO(lamarrr): Image subsetting should be done in the constructor
            // TODO(lamarrr): fix data alignment
            auto sk_image = SkImage::MakeRasterData(
                SkImageInfo::Make(image.width(), image.height(), color_type,
                                  SkAlphaType::kPremul_SkAlphaType),
                SkData::MakeWithoutCopy(image.bytes().data(), image.size()),
                image.width() * image.channels());

            VLK_DEBUG_ENSURE(sk_image != nullptr);

            int img_x0 = 0, img_y0 = 0, img_w = 0, img_h = 0;

            if (sizing.type() == Sizing::Type::Relative) {
              RelativeRect coordinates = sizing.get_relative().unwrap();
              img_x0 = coordinates.offset.x * image.width();
              img_y0 = coordinates.offset.y * image.height();
              img_w = coordinates.extent.width * image.width();
              img_h = coordinates.extent.height * image.height();
            } else {
              Rect coordinates = sizing.get_absolute().unwrap();
              img_x0 = coordinates.offset.x;
              img_y0 = coordinates.offset.y;
              img_w = coordinates.extent.width;
              img_h = coordinates.extent.height;
            }

            int bg_img_width = 0, bg_img_height = 0;

            if ((stretch & Stretch::X) != Stretch::None) {
              bg_img_width = content_width;
            } else {
              bg_img_width = image.width();
            }

            if ((stretch & Stretch::Y) != Stretch::None) {
              bg_img_height = content_height;
            } else {
              bg_img_height = image.height();
            }

            sk_canvas->drawImageRect(
                sk_image, SkRect::MakeXYWH(img_x0, img_y0, img_w, img_h),
                SkRect::MakeXYWH(border_left, border_top, bg_img_width,
                                 bg_img_height),
                &content_paint);
          },
          []() {});
    };

    auto color_draw_op = [&] {
      decoration_.color().match(
          [&](Color const &color) {
            content_paint.setColor(color.argb());
            sk_canvas->drawRect(SkRect::MakeXYWH(border_left, border_top,
                                                 content_width, content_height),
                                content_paint);
          },
          []() {});
    };

    if (decoration_.draw_order() == BoxDecoration::DrawOrder::ImageFirst) {
      image_draw_op();
      color_draw_op();
    } else {
      color_draw_op();
      image_draw_op();
    }

    sk_canvas->restore();

    sk_canvas->drawDRRect(border_rrect, content_rrect, border_paint);
  }

  virtual std::string_view get_type_hint() const noexcept override {
    return "Box";
  }

  // does not need caching for image and color and border without radius

 private:
  Widget *children_[1];
  BoxProperties properties_;
  BoxDecoration decoration_;
  std::vector<BoxShadow, BoxShadowAllocator> shadows_;
};

using Box = BasicBox<false>;

}  // namespace ui2d
}  // namespace vlk
