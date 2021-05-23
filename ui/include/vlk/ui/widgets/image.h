#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string_view>
#include <tuple>
#include <vector>

#include "include/core/SkPoint.h"
#include "stx/option.h"
#include "stx/span.h"
#include "vlk/ui/image_asset.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

// TODO(lamarrr): on effects: users would be able to inherit from the base
// widget and override the draw methods of each widget to provide their own draw
// methods that apply one effect or the other. effects that span acrross
// widgets? i.e. parent opacity and intends to fade along with its children.
// TODO(lamarrr): consider opacity tree? or require manually setting the
// opacitgies?
// we need render props that all widgets must have: i.e. opacity
//
// we need visibility prop
//
// is_layout_changed
// is_render_changed
//
// BaseProps? BaseProps::clean_state()..
//
// TODO(lamarrr): provided widgets must be static

enum class ImageStretch : uint8_t {
  None = 0,
  X = 1,
  Y = 2,
  XY = X | Y,
};

VLK_DEFINE_ENUM_BIT_OPS(ImageStretch)

namespace impl {

// aspect_ratio must be non-zero
static constexpr Extent aspect_ratio_trim(Extent aspect_ratio, Extent extent) {
  // 4 : 1
  // 1000 : 100
  // (100 * (4 / 1)) : 100
  //
  // 4 : 1
  // 100 : 1000
  // 100 : (100  / (4 / 1))
  //

  //
  // 1 : 4
  // 1000 : 100
  // (100 * (1 / 4)) : 100
  //
  // 1 : 4
  // 100 : 1000
  // 100 : (100 / (1 / 4))
  //

  float ratio = aspect_ratio.width / static_cast<float>(aspect_ratio.height);

  return extent.width > extent.height
             ? Extent{static_cast<uint32_t>(extent.height * ratio),
                      extent.height}
             : Extent{extent.width,
                      static_cast<uint32_t>(extent.width / ratio)};
}

}  // namespace impl

// TODO(lamarrr): move out these tests
static_assert(impl::aspect_ratio_trim(Extent{4, 1}, Extent{1000, 100}) ==
              Extent{400, 100});
static_assert(impl::aspect_ratio_trim(Extent{4, 1}, Extent{100, 1000}) ==
              Extent{100, 25});
static_assert(impl::aspect_ratio_trim(Extent{1, 4}, Extent{1000, 100}) ==
              Extent{25, 100});
static_assert(impl::aspect_ratio_trim(Extent{1, 4}, Extent{100, 1000}) ==
              Extent{100, 400});

struct ImageProps {
  //! target extent for the image widget, otherwise initially uses an internally
  //! specified extent and then updates the image widget's extent once the image
  //! is available.
  //! it is recommended to set this to prevent layout shift.
  ImageProps extent(SelfExtent value) const {
    ImageProps out{*this};
    out.extent_ = stx::Some(std::move(value));
    return out;
  }

  ImageProps no_extent() const {
    ImageProps out{*this};
    out.extent_ = stx::None;
    return out;
  }

  ImageProps extent(Extent value) const {
    return extent(SelfExtent::absolute(value));
  }

  ImageProps extent(uint32_t width, uint32_t height) const {
    return extent(Extent{width, height});
  }

  stx::Option<SelfExtent> extent() const { return extent_; }

  ImageProps border_radius(BorderRadius radius) const {
    ImageProps out{*this};
    out.border_radius_ = radius;
    return out;
  }

  BorderRadius border_radius() const { return border_radius_; }

  //! this is applied on the effective extent of this widget
  ImageProps aspect_ratio(Extent value) const {
    VLK_ENSURE(value.visible());
    ImageProps out{*this};
    out.aspect_ratio_ = stx::Some(std::move(value));
    return out;
  }

  ImageProps no_aspect_ratio() const {
    ImageProps out{*this};
    out.aspect_ratio_ = stx::None;
    return out;
  }

  ImageProps aspect_ratio(uint32_t width, uint32_t height) const {
    return aspect_ratio(Extent{width, height});
  }

  stx::Option<Extent> aspect_ratio() const { return aspect_ratio_; }

  ImageProps crop(Rect area) const {
    ImageProps out{*this};
    out.crop_ = stx::Some(std::move(area));
    return out;
  }

  ImageProps no_crop() const {
    ImageProps out{*this};
    out.crop_ = stx::None;
    return out;
  }

  stx::Option<Rect> crop() const { return crop_; }

  ImageProps stretch(ImageStretch value) const {
    ImageProps out{*this};
    out.stretch_ = value;
    return out;
  }

  ImageStretch stretch() const { return stretch_; }

  WidgetDirtiness diff(ImageProps const &new_props) const {
    WidgetDirtiness dirtiness = WidgetDirtiness::None;

    if (extent() != new_props.extent()) {
      dirtiness |= WidgetDirtiness::Layout;
    }

    if (border_radius() != new_props.border_radius()) {
      dirtiness |= WidgetDirtiness::Render;
    }

    if (aspect_ratio() != new_props.aspect_ratio()) {
      dirtiness |= WidgetDirtiness::Layout;
    }

    if (crop() != new_props.crop()) {
      dirtiness |= WidgetDirtiness::Render;
    }

    if (stretch() != new_props.stretch()) {
      dirtiness |= WidgetDirtiness::Render;
    }

    return dirtiness;
  }

 private:
  // if extent is specified and different from the extent once loaded...
  stx::Option<SelfExtent> extent_;
  BorderRadius border_radius_;
  stx::Option<Extent> aspect_ratio_;
  stx::Option<Rect> crop_;
  ImageStretch stretch_ = ImageStretch::None;
};

constexpr std::array<SkVector, 4> to_skia(BorderRadius const &border_radius) {
  return {
      SkVector::Make(border_radius.top_left, border_radius.top_left),
      SkVector::Make(border_radius.top_right, border_radius.top_right),
      SkVector::Make(border_radius.bottom_left, border_radius.bottom_left),
      SkVector::Make(border_radius.bottom_right, border_radius.bottom_right),
  };
}

struct Image : public Widget {
  // Note: we can use sizing here because width and height is known ahead of
  // time.
  // static constexpr Extent DefaultImageExtent = Extent{50, 50};
  // TODO(lamarrr): implement source updating???

  Image(MemoryImageSource const &source, ImageProps const &props) {
    source_ = source;
    update_props(props);
  }

  Image(FileImageSource const &source, ImageProps const &props) {
    source_ = source;
    update_props(props);
  }

  virtual void draw(Canvas &canvas, AssetManager &asset_manager) override {
    // TODO(lamarrr): if props is updated in-between these states?

    // TODO(lamarrr): AssetManager should be accessed in the tick method, not
    // the draw method.
    // otherwise this method will cause partial updates.

    SkCanvas *sk_canvas = canvas.as_skia().unwrap();

    // the widget was just initialized and we need to submit the image asset
    // to the asset manager
  }

  bool is_memory_sourced() const {
    return std::holds_alternative<MemoryImageSource>(source_);
  }

  bool is_file_sourced() const {
    return std::holds_alternative<FileImageSource>(source_);
  }

  void update_props(ImageProps const &props) {
    // once a file image is loaded and no extent is provided we'd need to
    // perform a re-layout
    //
    //
    // once the image arrives, we update the prop to use the new extent of the
    // new image
    //
    //
    props.extent().match(
        [&](SelfExtent const &extent) { Widget::update_self_extent(extent); },
        [&]() { Widget::update_self_extent(SelfExtent{}); });

    props.aspect_ratio().match(
        [&](Extent) { Widget::update_needs_trimming(true); },
        [&]() { Widget::update_needs_trimming(false); });

    WidgetDirtiness dirtiness = props_.diff(props);
    Widget::add_dirtiness(dirtiness);

    props_ = props;

    // updating any of the props will cause the state of the rendering to be
    // disrupted, we thus need to start again
    if (dirtiness == WidgetDirtiness::None) state_ = State::Begin;
  }

  ImageProps get_props() const { return props_; }

  virtual Extent trim(Extent extent) override {
    // TODO(lamarrr): due to aspect ratio cropping we need to place image at the
    // center.
    return impl::aspect_ratio_trim(
        get_props().aspect_ratio().expect(
            "Trim called without an aspect ratio set"),
        extent);
  }

  virtual void tick(std::chrono::nanoseconds interval) override {
    if (state_ == State::Begin) {
      if (is_memory_sourced()) {
        auto const &source = std::get<MemoryImageSource>(source_);
        (void)add_asset(asset_manager, source);
      } else {
        auto const &source = std::get<FileImageSource>(source_);
        (void)add_asset(asset_manager, source);
      }

      state_ = State::ImageSubmitted;
    }

    // we've submitted the image to the asset manager or it has previously
    // been submitted by another widget and we are awaiting the status of
    // the image
    if (state_ == State::ImageSubmitted) {
      stx::Result load_result =
          is_memory_sourced()
              ? get_asset(asset_manager, std::get<MemoryImageSource>(source_))
              : get_asset(asset_manager, std::get<FileImageSource>(source_));

      load_result.as_ref().match(
          [&](std::shared_ptr<ImageAsset const> const &asset) {
            state_ = asset->get_ref().is_ok() ? State::ImageLoadSuccessful
                                              : State::ImageLoadFailed;
          },
          [&](AssetError error) {
            switch (error) {
              case AssetError::InvalidTag: {
                state_ = State::ImageLoadFailed;
              } break;
              case AssetError::IsLoading: {
                // we need to check the image load status again
                state_ = State::ImageSubmitted;
              } break;
              default: {
                VLK_PANIC("Unexpected State");
              }
            }
          });
    }

    // the image has been successfully loaded
    if (state_ == State::ImageLoadSuccessful) {
      stx::Result load_result =
          is_memory_sourced()
              ? get_asset(asset_manager, std::get<MemoryImageSource>(source_))
              : get_asset(asset_manager, std::get<FileImageSource>(source_));

      // props updating? this would take an unnecessarily long time
      if (is_file_sourced() && props_.extent().is_none()) {
        // send layout details out and request for reflow if necessary
        load_result.as_ref().match(
            [&](std::shared_ptr<ImageAsset const> const &image_asset) {
              image_asset->get_ref().match(
                  [&](sk_sp<SkSurface> const &surface) {
                    update_props(get_props().extent(
                        Extent{static_cast<uint32_t>(surface->width()),
                               static_cast<uint32_t>(surface->height())}));
                  },
                  [](auto) { VLK_PANIC("Unexpected State"); });
            },
            [](auto) { VLK_PANIC("Unexpected State"); });

        // layout reflow will occur in next system tick, immediately return as
        // layout needs to be allowed to perform before doing anything else
        mark_layout_dirty();

        // TODO(lamarrr): reflow is processed immediately after tick() so we can
        // move on to rendering immediately
        state_ = State::RequestedRelayout;
        return;
      } else {
        // the image doesn't need reflow and we can just proceed to rendering
        state_ = State::RenderImage;
      }
    }

    // we failed to load the image, render error image.
    // the error image uses whatever extent the widget has which is Extent{0, 0}
    // by default.
    if (state_ == State::ImageLoadFailed) {
      state_ = State::RenderErrorImage;
    }

    if (state_ == State::RequestedRelayout) {
      state_ = State::RenderImage;
    }

    if (state_ == State::RenderImage) {
      // draw actual image
      stx::Result load_result =
          is_memory_sourced()
              ? get_asset(asset_manager, std::get<MemoryImageSource>(source_))
              : get_asset(asset_manager, std::get<FileImageSource>(source_));

      load_result.as_ref().match(
          [&](std::shared_ptr<ImageAsset const> const &image_asset) {
            image_asset->get_ref().match(
                [&](sk_sp<SkSurface> const &surface) {
                  sk_canvas->save();

                  if (props_.border_radius() != BorderRadius::all(0)) {
                    SkRRect round_rect;

                    std::array border_radii = to_skia(props_.border_radius());

                    round_rect.setRectRadii(
                        SkRect::MakeWH(surface->width(), surface->height()),
                        border_radii.data());

                    sk_canvas->clipRRect(round_rect, true);
                  }

                  // we can add transparency effects to the paint here
                  // scale and stretch
                  surface->draw(sk_canvas, 0, 0, nullptr);

                  sk_canvas->restore();
                },
                [](auto) {

                });
          },
          [](auto) {});
    }

    // TODO(lamarrr): if the image is discarded? get_asset will? go back to
    // submitted state?
    if (state_ == State::RenderErrorImage) {
      // draw error image
    }
  }

 private:
  enum class State {
    Begin,
    ImageSubmitted,
    ImageLoadFailed,
    ImageLoadSuccessful,
    // RequestedRelayout,
    // RenderImage,
    // RenderErrorImage
  };

  ImageProps props_;
  std::variant<stx::NoneType, MemoryImageSource, FileImageSource> source_;
  State state_ = State::Begin;
  stx::Option<sk_sp<SkSurface>> texture_ = stx::None;
};  // namespace ui

}  // namespace ui
}  // namespace vlk
