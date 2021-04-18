#pragma once

#include <algorithm>
#include <utility>

#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/utils/utils.h"

#include "vlk/assets/image.h"

#include "stx/option.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkMaskFilter.h"  // src
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"

namespace vlk {
namespace ui {

struct Border {
  Color color;
  Edges edges;

  static constexpr Border all(Color color, uint32_t value) {
    return Border{color, Edges::all(value)};
  }

  static constexpr Border symmetric(Color color, uint32_t x, uint32_t y) {
    return Border{color, Edges::symmetric(x, y)};
  }

  static constexpr Border trbl(Color color, uint32_t t, uint32_t r, uint32_t b,
                               uint32_t l) {
    return Border{color, Edges::trbl(t, r, b, l)};
  }
};

using BorderRadius = Corners;

struct BoxProps {
  constexpr BoxProps padding(Padding const &value) const {
    BoxProps out{*this};
    out.padding_ = value;
    return out;
  }

  constexpr Padding padding() const { return padding_; }

  constexpr BoxProps border(Border const &value) const {
    BoxProps out{*this};
    out.border_ = value;
    return out;
  }

  constexpr Border border() const { return border_; }

  constexpr BoxProps border_radius(BorderRadius const &value) const {
    BoxProps out{*this};
    out.border_radius_ = value;
    return out;
  }

  constexpr BorderRadius border_radius() const { return border_radius_; }

  // TODO(lamarrr): sizing information

 private:
  Padding padding_ = Padding::all(0);
  Border border_ = Border::all(colors::Transparent, 0);
  BorderRadius border_radius_ = BorderRadius::all(0);
};

struct BoxDecoration {
  using Image = data::Image;

  enum class Blend : uint8_t { ColorOver = 0, ImageOver = 1 };

  BoxDecoration color(Color const &value) && {
    blend_ = Blend::ColorOver;
    [[maybe_unused]] stx::Option previous = background_color_.replace(value);
    return std::move(*this);
  }

  stx::Option<Color> color() const { return background_color_.clone(); }

  BoxDecoration image(Image &&image) && {
    blend_ = Blend::ImageOver;
    [[maybe_unused]] stx::Option previous =
        background_image_.replace(std::move(image));
    return std::move(*this);
  }

  stx::Option<stx::ConstRef<Image>> image() const {
    return background_image_.as_cref();
  }

  BoxDecoration blur(float gaussian_blur_sigma) && {
    VLK_ENSURE(gaussian_blur_sigma > 0.0f,
               "Gaussian Blur Sigma must be greater than 0.0f");
    [[maybe_unused]] float previous =
        gaussian_blur_.replace(gaussian_blur_sigma);
    return std::move(*this);
  }

  stx::Option<float> blur() const { return gaussian_blur_.clone(); }

  BoxDecoration blend(Blend blend) && {
    blend_ = blend;
    return std::move(*this);
  }

  Blend blend() const { return blend_; }

 private:
  Blend blend_ = Blend::ColorOver;
  stx::Option<Color> background_color_ = stx::None;
  stx::Option<Image> background_image_ = stx::None;
  // must be greater than 0 if set
  stx::Option<float> gaussian_blur_ = stx::None;
};

struct Box : public Widget {
  Box(Widget *child, BoxProps const &properties = {},
      BoxDecoration decoration = BoxDecoration{}, Flex const &flex = {})
      : children_{child},
        properties_{properties},
        decoration_{std::move(decoration)} {
    // only border information should be stored

    Widget::init_type(Widget::Type::Render);
    Widget::init_is_flex(true);
    Widget::update_children(children_);
    Widget::update_flex(flex);
    Widget::update_self_extent(SelfExtent{Constrain{1.0f}, Constrain{1.0f}});
    Widget::update_padding(properties_.padding());
  }

  ~Box() { delete children_[0]; }

  virtual void draw(Canvas &canvas) override {
    SkCanvas *sk_canvas = canvas.as_skia().expect("canvas backend is not skia");

    Extent const widget_extent = canvas.extent();

    Border const border = properties_.border();
    BorderRadius const border_radius = properties_.border_radius();
    // TODO(lamarrr): do we need a notion of borders within our layout system?
    // the child's layout does not respect this presently, we need it since we
    // can't afford to make it a widget.
    // TODO(lamarrr): add paths for ones with 0.0 border radius and unused
    // properties

    uint32_t const border_x =
        std::min(border.edges.left + border.edges.right, widget_extent.width);
    uint32_t const border_y =
        std::min(border.edges.top + border.edges.bottom, widget_extent.height);

    uint32_t const border_left = std::min(border.edges.left, border_x);
    uint32_t const border_right = border_x - border_left;

    uint32_t const border_top = std::min(border.edges.top, border_y);
    uint32_t const border_bottom = border_y - border_top;

    Extent const content_extent =
        Extent{widget_extent.width - border_x, widget_extent.height - border_y};

    SkPaint blur_paint;
    blur_paint.setAntiAlias(true);

    SkPaint effect_paint;
    effect_paint.setAntiAlias(true);
    effect_paint.setColor(border.color.to_argb());

    decoration_.blur().match(
        [&](float sigma) {
          blur_paint.setMaskFilter(
              SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, sigma));
        },
        []() {});

    SkRRect content_rrect;
    SkRRect border_rrect;

    SkVector const content_radii[] = {
        SkVector::Make(border_radius.top_left, border_radius.top_left),
        SkVector::Make(border_radius.top_right, border_radius.top_right),
        SkVector::Make(border_radius.bottom_left, border_radius.bottom_left),
        SkVector::Make(border_radius.bottom_right, border_radius.bottom_right),
    };

    SkVector const border_radii[] = {
        SkVector::Make(border_radius.top_left + border_left,
                       border_radius.top_left + border_top),
        SkVector::Make(border_radius.top_right + border_right,
                       border_radius.top_right + border_top),
        SkVector::Make(border_radius.bottom_left + border_left,
                       border_radius.bottom_left + border_bottom),
        SkVector::Make(border_radius.bottom_right + border_right,
                       border_radius.bottom_right + border_bottom),
    };

    content_rrect.setRectRadii(
        SkRect::MakeXYWH(border_left, border_top, content_extent.width,
                         content_extent.height),
        content_radii);
    border_rrect.setRectRadii(
        SkRect::MakeXYWH(0, 0, widget_extent.width, widget_extent.height),
        border_radii);

    sk_canvas->save();
    sk_canvas->clipRRect(content_rrect, true);

    auto image_draw_op = [&] {
      decoration_.image().match(
          [&](stx::ConstRef<BoxDecoration::Image> imref) {
            // SKia expects pixels to be aligned on a 32-bit word.
            BoxDecoration::Image const &image = imref.get();

            VLK_ENSURE(image.width() != 0 && image.height() != 0 &&
                       image.channels() != 0);

            SkColorType color_type;

            switch (image.format()) {
              case data::Image::Format::RGBA:
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
            sk_sp<SkImage> sk_image = SkImage::MakeRasterData(
                SkImageInfo::Make(image.width(), image.height(), color_type,
                                  SkAlphaType::kPremul_SkAlphaType),
                SkData::MakeWithoutCopy(image.bytes().data(), image.size()),
                image.width() * image.channels());

            VLK_ENSURE(sk_image != nullptr);

            sk_canvas->drawImageRect(
                sk_image, SkRect::MakeXYWH(0, 0, image.width(), image.height()),
                SkRect::MakeXYWH(border_left, border_top, content_extent.width,
                                 content_extent.height),
                &blur_paint);
          },
          []() {});
    };

    auto color_draw_op = [&] {
      decoration_.color().match(
          [&](Color const &color) {
            blur_paint.setColor(color.to_argb());
            sk_canvas->drawRect(
                SkRect::MakeXYWH(border_left, border_top, content_extent.width,
                                 content_extent.height),
                blur_paint);
          },
          []() {});
    };

    if (decoration_.blend() == BoxDecoration::Blend::ImageOver) {
      color_draw_op();
      image_draw_op();
    } else {
      image_draw_op();
      color_draw_op();
    }

    sk_canvas->restore();

    sk_canvas->drawDRRect(border_rrect, content_rrect, effect_paint);
  }

 private:
  Widget *children_[1];
  BoxProps properties_;
  BoxDecoration decoration_;
};

}  // namespace ui
}  // namespace vlk
