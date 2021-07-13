#pragma once

#include <memory>
#include <utility>

#include "stx/option.h"
#include "vlk/ui/image_source.h"
#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

//
// box shadow
//
// 	Default value. No shadow is displayed
// h-offset	Required. The horizontal offset of the shadow. A positive value
// puts the shadow on the right side of the box, a negative value puts the
// shadow on the left side of the box v-offset	Required. The vertical offset of
// the shadow. A positive value puts the shadow below the box, a negative value
// puts the shadow above the box blur	Optional. The blur radius. The higher
// the number, the more blurred the shadow will be spread	Optional. The
// spread radius. A positive value increases the size of the shadow, a negative
// value decreases the size of the shadow color	Optional. The color of the
// shadow. The default value is the text color. Look at CSS Color Values for a
// complete list of possible color values.
//
// Note: In Safari (on PC) the color parameter is required. If you do not
// specify the color, the shadow is not displayed at all. inset	Optional.
// Changes the shadow from an outer shadow (outset) to an inner shadow
//
//

enum class BoxBlend : uint8_t { ColorOver = 0, ImageOver = 1 };

//
// TODO(lamarrr): background image fit
enum class BoxFit {
  //! center the background image on the box whilst preserving aspect ratio
  None,
  //! resize image to cover entire box without distorting the image, some parts
  //! of the image may become invisible
  Cover,
  // make the image fully visible, without distorting the image, the image may
  // not cover the entire area of the widget and leave some parts stale
  Contain,
  // fill the whole area even if it means distorting the width and height
  Fill
};

namespace impl {

constexpr IRect box_fit_crop(BoxFit fit, Extent image_extent,
                             Extent widget_extent) {
  IRect dst_rect{};

  switch (fit) {
    case BoxFit::None: {
      // dst_rect
    } break;
    case BoxFit::Cover:
      break;
    case BoxFit::Contain:
      break;
    case BoxFit::Fill:
      break;
  }
}

}  // namespace impl

struct BoxProps {
  //! if extent is not specified, Box shrinks down enough to accomodate the size
  //! of its child
  BoxProps extent(SelfExtent self_extent) const {
    BoxProps out{*this};
    out.extent_ = self_extent;
    return out;
  }

  BoxProps extent(Extent self_extent) const {
    BoxProps out{*this};
    out.extent_ = SelfExtent::absolute(self_extent);
    return out;
  }

  BoxProps extent(uint32_t width, uint32_t height) const {
    BoxProps out{*this};
    out.extent_ = SelfExtent::absolute(width, height);
    return out;
  }

  BoxProps constrain(Extent min = Extent{u32_min, u32_min},
                     Extent max = Extent{u32_max, u32_max}) const {
    BoxProps out{*this};
    out.extent_.width.min = min.width;
    out.extent_.width.max = max.width;
    out.extent_.height.min = min.height;
    out.extent_.height.max = max.height;
    return out;
  }

  auto extent() const { return extent_; }

  BoxProps fit(BoxFit value) const {
    BoxProps out{*this};
    out.fit_ = value;
    return out;
  }

  auto fit() const { return fit_; }

  BoxProps flex(Flex box_flex) const {
    BoxProps out{*this};
    out.flex_ = box_flex;
    return out;
  }

  auto flex() const { return flex_; }

  BoxProps padding(Padding value) const {
    BoxProps out{*this};
    out.padding_ = value;
    return out;
  }

  auto padding() const { return padding_; }

  BoxProps border(Border value) const {
    BoxProps out{*this};
    out.border_ = value;
    return out;
  }

  auto border() const { return border_; }

  BoxProps border_radius(BorderRadius value) const {
    BoxProps out{*this};
    out.border_radius_ = value;
    return out;
  }

  auto border_radius() const { return border_radius_; }

  BoxProps color(Color value) const {
    BoxProps out{*this};
    out.color_ = value;
    return out;
  }

  auto color() const { return color_; }

  //! background blur
  BoxProps blur(Blur blur) const {
    BoxProps out{*this};
    out.blur_ = blur;
    return out;
  }

  auto blur() const { return blur_; }

  BoxProps image(FileImageSource source) const {
    BoxProps out{*this};
    out.background_image_ = stx::Some(ImageSource{std::move(source)});
    return out;
  }

  BoxProps image(MemoryImageSource source) const {
    BoxProps out{*this};
    out.background_image_ = stx::Some(ImageSource{std::move(source)});
    return out;
  }

  BoxProps image(stx::NoneType) const {
    BoxProps out{*this};
    out.background_image_ = stx::None;
    return out;
  }

  auto image() const { return background_image_; }

  auto const &image_ref() const { return background_image_; }

  BoxProps blend(BoxBlend blend) const {
    BoxProps out{*this};
    out.blend_ = blend;
    return out;
  }

  auto blend() const { return blend_; }

 private:
  SelfExtent extent_ = SelfExtent::relative(1.0f, 1.0f);
  BoxFit fit_ = BoxFit::None;
  Flex flex_ = Flex{};
  Padding padding_ = Padding::all(0);
  Border border_ = Border::all(colors::Transparent, 0);
  BorderRadius border_radius_ = BorderRadius::all(0);
  Color color_ = colors::Transparent;
  Blur blur_;
  stx::Option<ImageSource> background_image_ = stx::None;
  BoxBlend blend_ = BoxBlend::ColorOver;
};

enum class BoxState : uint8_t {
  BackgroundStale,
  BackgroundLoading,
  BackgroundLoaded,
  BackgroundLoadFailed
};

namespace impl {
enum class BoxDiff : uint16_t {
  None = 0,
  Extent = 1 << 0,
  Fit = 1 << 1,
  Flex = 1 << 2,
  Padding = 1 << 3,
  Border = 1 << 4,
  BorderRadius = 1 << 5,
  Color = 1 << 6,
  Blur = 1 << 7,
  BackgroundImage = 1 << 8,
  Blend = 1 << 9,
  All = (1 << 10) - 1
};

VLK_DEFINE_ENUM_BIT_OPS(BoxDiff)

struct BoxStorage {
  BoxProps props;
  BoxState state = BoxState::BackgroundStale;
  Ticks asset_stale_ticks = Ticks{0};
  stx::Option<std::shared_ptr<ImageAsset const>> asset = stx::None;
};

}  // namespace impl

struct Box : public Widget {
  Box(Widget *child, BoxProps props) {
    update_props(std::move(props));
    update_child(child);
  }

  BoxProps get_props() const { return storage_.props; }

  BoxState get_state() const { return storage_.state; }

  void update_props(BoxProps new_props);

  // takes ownership
  void update_child(Widget *widget) {
    if (children_[0] != nullptr) {
      delete children_[0];
    }

    children_[0] = widget;

    Widget::update_children(children_);
  }

  virtual ~Box() override {
    // shouldn't be null but just added
    if (children_[0] != nullptr) delete children_[0];
  }

  virtual void draw(Canvas &) override;

  virtual void tick(std::chrono::nanoseconds, AssetManager &) override;

 private:
  Widget *children_[1] = {nullptr};
  impl::BoxDiff diff_ = impl::BoxDiff::All;
  impl::BoxStorage storage_;
};

}  // namespace ui
}  // namespace vlk
