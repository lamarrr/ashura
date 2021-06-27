#pragma once

#include <memory>
#include <utility>

#include "stx/option.h"

//

#include "vlk/ui/image_source.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

struct ImageProps {
  explicit ImageProps(MemoryImageSource source) : source_{std::move(source)} {}

  explicit ImageProps(FileImageSource source) : source_{std::move(source)} {}

  ImageProps source(MemoryImageSource image_source) const {
    ImageProps out{*this};
    out.source_ = std::move(image_source);
    return out;
  }

  ImageProps source(FileImageSource image_source) const {
    ImageProps out{*this};
    out.source_ = std::move(image_source);
    return out;
  }

  ImageProps source(ImageSource image_source) const {
    ImageProps out{*this};
    out.source_ = std::move(image_source);
    return out;
  }

  ImageSource source() const { return source_; }

  ImageSource const &source_ref() const { return source_; }

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

 private:
  ImageSource source_;
  stx::Option<SelfExtent> extent_;
  BorderRadius border_radius_;
  stx::Option<Extent> aspect_ratio_;
};

enum class ImageState : uint8_t {
  //! the image has not been in view yet (or in a long time, its not holding a
  //! reference to its image asset)
  Stale,
  //! the image's asset is loading
  Loading,
  //! a non-fatal failure occured while loading the image's asset
  LoadFailed,
  //! the image's asset has been successfully loaded
  Loaded
};

namespace impl {

enum class ImageDiff : uint8_t {
  None = 0,
  Source = 1,
  Extent = 2,
  BorderRadius = 4,
  AspectRatio = 8,
  All = 16
};

VLK_DEFINE_ENUM_BIT_OPS(ImageDiff)

struct ImageStorage {
  ImageProps props;
  ImageState state = ImageState::Stale;
  Ticks asset_stale_ticks = Ticks{0};
  stx::Option<std::shared_ptr<ImageAsset const>> asset = stx::None;
};

}  // namespace impl

//
// Usage Needs
//
// - Add image to asset registry for offloading to GPU for fast transfers when
// needed (i.e. zero copy over PCIE from CPU to GPU during rendering)
// - Fetch image from the asset registry and inform the target widget once the
// image arrives and mark its render as dirty
// - Inform the widget that the image is loading
// NOTE: Partial invalidations will occur to the image widget while its still in
// view, so the widget itself doesn't have a means of determining when this
// invalidation will happen or when and which part of the widget the system will
// need
// - The assets need to be discarded when not in use specifically when not in
// use by the widget's draw method
//
//
//
// track the image last hit tick in the draw method
// once the asset is not hit for a long time remove shared_ptr to the asset and
// mark widget as dirty.
//
//
struct Image : public Widget {
  explicit Image(ImageProps props) : storage_{std::move(props)} {
    // called to intialize Widget's extent and aspect ratio
    update_props(storage_.props);
  }

  ImageState get_state() const { return storage_.state; }

  ImageProps get_props() const { return storage_.props; }

  void update_props(ImageProps props);

  virtual Extent trim(Extent extent) override {
    return aspect_ratio_trim(
        storage_.props.aspect_ratio().expect(
            "Image::trim() called without an aspect ratio set"),
        extent);
  }

  //! implement this to draw a custom loading image/animation
  virtual void draw_loading_image(Canvas &) {}

  //! implement this to draw a custom loading image/animation
  virtual void draw_error_image(Canvas &) {}

  //! NOTE: only part of an image might be actually needed during drawing, any
  //! ref.
  virtual void draw(Canvas &canvas) override;

  virtual void tick(std::chrono::nanoseconds,
                    AssetManager &asset_manager) override;

 private:
  impl::ImageStorage storage_;
  impl::ImageDiff diff_ = impl::ImageDiff::All;
};

}  // namespace ui
}  // namespace vlk
