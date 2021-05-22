#pragma once

#include <algorithm>
#include <utility>

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkImageFilters.h"
#include "stx/option.h"
#include "vlk/assets/image.h"
#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/utils/utils.h"

///
#include "vlk/ui/image_asset.h"

namespace vlk {
namespace ui {

enum class BoxBlend : uint8_t { ColorOver = 0, ImageOver = 1 };

struct BoxProps {
  BoxProps padding(Padding const &value) const {
    BoxProps out{*this};
    out.padding_ = value;
    return out;
  }

  Padding padding() const { return padding_; }

  BoxProps border(Border const &value) const {
    BoxProps out{*this};
    out.border_ = value;
    return out;
  }

  Border border() const { return border_; }

  BoxProps border_radius(BorderRadius const &value) const {
    BoxProps out{*this};
    out.border_radius_ = value;
    return out;
  }

  BorderRadius border_radius() const { return border_radius_; }

  // TODO(lamarrr): sizing information

  /*
    BoxProps image(Image &&image) && {
      blend_ = Blend::ImageOver;
      [[maybe_unused]] stx::Option previous =
          background_image_.replace(std::move(image));
      return std::move(*this);
    }

    stx::Option<stx::ConstRef<Image>> image() const {
      return background_image_.as_cref();
    }
  */

  BoxProps color(Color const &value) {
    BoxProps out{*this};
    out.blend_ = BoxBlend::ColorOver;
    out.color_ = value;
    return out;
  }

  Color color() const { return color_; }

  BoxProps blend(BoxBlend blend) {
    BoxProps out{*this};
    out.blend_ = blend;
    return out;
  }

  BoxBlend blend() const { return blend_; }

  BoxProps blur(Blur const &blur) const {
    // TODO(lamarrr): emit warning instead and crash if necessary, make sure to
    // omit and check for nullptr-ness of the filter.
    // clamp?

    BoxProps out{*this};
    out.blur_ = stx::Some(Blur{blur});
    return out;
  }

  stx::Option<Blur> blur() const { return blur_.clone(); }

 private:
  Padding padding_ = Padding::all(0);
  Border border_ = Border::all(colors::Transparent, 0);
  BorderRadius border_radius_ = BorderRadius::all(0);
  Color color_ = colors::Transparent;
  stx::Option<Blur> blur_ = stx::None;
  BoxBlend blend_ = BoxBlend::ColorOver;
  // stx::Option<Image> background_image_ = stx::None;
};

struct Box : public Widget {
  Box(Widget *child, BoxProps const &properties = {}, Flex const &flex = {})
      : children_{child}, props_{properties} {
    // only border information should be stored

    // TODO(lamarrr): prop updating (check which affects layout and render)

    Widget::init_type(Widget::Type::Render);
    Widget::init_is_flex(true);
    Widget::update_children(children_);
    Widget::update_flex(flex);
    Widget::update_self_extent(SelfExtent{Constrain{1.0f}, Constrain{1.0f}});
    auto const padding = props_.padding();
    auto const border = props_.border();
    Widget::update_padding(Padding::trbl(padding.top + border.edges.top,
                                         padding.right + border.edges.right,
                                         padding.bottom + border.edges.bottom,
                                         padding.left + border.edges.left));
  }

  ~Box() { delete children_[0]; }

  virtual void draw(Canvas &canvas, AssetManager &asset_manager) override {
    SkCanvas *sk_canvas = canvas.as_skia().expect("canvas backend is not skia");

    Extent const widget_extent = canvas.extent();

    Border const border = props_.border();
    BorderRadius const border_radius = props_.border_radius();
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

    bool const border_visible = props_.border().color.visible() &&
                                props_.border().edges != Edges{};

    SkPaint blur_paint;
    blur_paint.setAntiAlias(true);

    // TODO(lamarrr): fix naming here
    SkPaint border_paint;
    border_paint.setAntiAlias(true);
    border_paint.setColor(border.color.to_argb());

    props_.blur().clone().match(
        [&](Blur blur) {
          sk_sp blur_filter =
              SkImageFilters::Blur(blur.sigma_x(), blur.sigma_y(), nullptr);
          blur_paint.setImageFilter(blur_filter);
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

    if (border_visible) {
      border_rrect.setRectRadii(
          SkRect::MakeXYWH(0, 0, widget_extent.width, widget_extent.height),
          border_radii);
    }

    sk_canvas->save();
    sk_canvas->clipRRect(content_rrect, true);

    /*  auto image_draw_op = [&] {
        decoration_.image().match(
            [&](stx::ConstRef<BoxDecoration::Image> imref) {*/
    // SKia expects pixels to be aligned on a 32-bit word.
    /*  BoxDecoration::Image const &image = imref.get();

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
*/
    // TODO(lamarrr): fix alpha type
    // TODO(lamarrr): fix asset loader to use u32_t word alignment? How
    // will this affect Vulkan? does Vulkan also require u32_t
    // alignment?
    // TODO(lamarrr): Image subsetting should be done in the constructor
    // TODO(lamarrr): fix data alignment
    /*          sk_sp<SkImage> sk_image = SkImage::MakeRasterData(
                  SkImageInfo::Make(image.width(), image.height(), color_type,
                                    SkAlphaType::kPremul_SkAlphaType),
                  SkData::MakeWithoutCopy(image.bytes().data(), image.size()),
                  image.width() * image.channels());

              VLK_ENSURE(sk_image != nullptr);

              sk_canvas->drawImageRect(
                  sk_image, SkRect::MakeXYWH(0, 0, image.width(),
       image.height()), SkRect::MakeXYWH(border_left, border_top,
       content_extent.width, content_extent.height), &blur_paint);

                  */
    /*    },
        []() {});
  };*/

    auto color_draw_op = [&] {
      if (!props_.color().transparent()) {
        blur_paint.setColor(props_.color().to_argb());
        sk_canvas->drawRect(
            SkRect::MakeXYWH(border_left, border_top, content_extent.width,
                             content_extent.height),
            blur_paint);
      }
    };

    if (props_.blend() == BoxBlend::ImageOver) {
      color_draw_op();
      // image_draw_op();
    } else {
      // image_draw_op();
      color_draw_op();
    }

    sk_canvas->restore();

    if (border_visible) {
      sk_canvas->drawDRRect(border_rrect, content_rrect, border_paint);
    } else {
      sk_canvas->drawRRect(content_rrect, border_paint);
    }
  }

 private:
  Widget *children_[1];
  BoxProps props_;
};

}  // namespace ui
}  // namespace vlk
