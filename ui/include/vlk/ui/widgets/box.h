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
struct BoxProps {
  //! if extent is not specified, Box shrinks down enough to accomodate the size
  //! of its child
  BoxProps extent(SelfExtent self_extent) const {
    BoxProps out{*this};
    out.extent_ = stx::Some(std::move(self_extent));
    return out;
  }

  BoxProps extent(Extent self_extent) const {
    BoxProps out{*this};
    out.extent_ = stx::Some(SelfExtent::absolute(self_extent));
    return out;
  }

  BoxProps extent(uint32_t width, uint32_t height) const {
    BoxProps out{*this};
    out.extent_ = stx::Some(SelfExtent::absolute(width, height));
    return out;
  }

  BoxProps extent(stx::NoneType) const {
    BoxProps out{*this};
    out.extent_ = stx::None;
    return out;
  }

  auto extent() const { return extent_; }

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

  BoxProps image(FileImageSource source) const {
    BoxProps out{*this};
    out.background_ = stx::Some(ImageSource{std::move(source)});
    return out;
  }

  BoxProps image(MemoryImageSource source) const {
    BoxProps out{*this};
    out.background_ = stx::Some(ImageSource{std::move(source)});
    return out;
  }

  BoxProps image(stx::NoneType) const {
    BoxProps out{*this};
    out.background_ = stx::None;
    return out;
  }

  auto image() const { return background_; }

  auto const &image_ref() const { return background_; }

  BoxProps color(Color value) const {
    BoxProps out{*this};
    out.color_ = value;
    return out;
  }

  auto color() const { return color_; }

  //! background blur
  BoxProps blur(Blur blur) const {
    BoxProps out{*this};
    out.blur_ = stx::Some(std::move(blur));
    return out;
  }

  BoxProps blur(stx::NoneType) const {
    BoxProps out{*this};
    out.blur_ = stx::None;
    return out;
  }

  auto blur() const { return blur_; }

  BoxProps blend(BoxBlend blend) const {
    BoxProps out{*this};
    out.blend_ = blend;
    return out;
  }

  auto blend() const { return blend_; }

  BoxProps flex(Flex box_flex) const {
    BoxProps out{*this};
    out.flex_ = box_flex;
    return out;
  }

  auto flex() const { return flex_; }

 private:
  stx::Option<SelfExtent> extent_;
  Padding padding_ = Padding::all(0);
  Border border_ = Border::all(colors::Transparent, 0);
  BorderRadius border_radius_ = BorderRadius::all(0);
  Color color_ = colors::Transparent;
  stx::Option<Blur> blur_ = stx::None;
  BoxBlend blend_ = BoxBlend::ColorOver;
  stx::Option<ImageSource> background_ = stx::None;
  Flex flex_ = Flex{};
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
  Padding = 1 << 1,
  Border = 1 << 2,
  BorderRadius = 1 << 3,
  Color = 1 << 4,
  Blur = 1 << 5,
  Blend = 1 << 6,
  // needs to reload image asset but shouldn't cause layout reflow
  BackgroundImage = 1 << 7,
  Flex = 1 << 8,
  All = (Flex << 1) - 1
};

VLK_DEFINE_ENUM_BIT_OPS(BoxDiff)

struct BoxStorage {
  BoxProps props;
  BoxState state = BoxState::BackgroundStale;
  bool drawn_in_last_tick = false;
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
