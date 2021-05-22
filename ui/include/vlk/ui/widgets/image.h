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

#include "stx/option.h"
#include "stx/span.h"
#include "vlk/ui/image_asset.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

enum class Dirtiness : uint8_t {
  None = 0b00,
  Render = 0b01,
  Layout = 0b10,
  Children = 0b11
};

VLK_DEFINE_ENUM_BIT_OPS(Dirtiness)

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

// TODO(lamarrr): on effects: users would be able to inherit from the base
// widget and override the draw methods of each widget to provide their own draw
// methods that apply one effect or the other. effects that span acrross
// widgets? i.e. parent opacity and intends to fade along with its children.
VLK_DEFINE_ENUM_BIT_OPS(ImageStretch);

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

  // no data must be referenced beyond these functions
  // async transition: optional
  // i.e. always maintain aspect ration even if a relative dimension is used.
  // must return option on query
  // -1.0f by default
  // input must not be 0.0f nor negative
  // if resulting aspect ratio results in a 0.0 then just not draw the image
  // TODO(lamarrr): we need to properly handle zero-sized widgets
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

  Dirtiness diff(ImageProps const &new_props) const {
    Dirtiness dirtiness = Dirtiness::None;

    if (extent() != new_props.extent()) {
      dirtiness |= Dirtiness::Layout;
    }

    if (border_radius() != new_props.border_radius()) {
      dirtiness |= Dirtiness::Render;
    }

    if (aspect_ratio() != new_props.aspect_ratio()) {
      dirtiness |= Dirtiness::Layout;
    }

    if (crop() != new_props.crop()) {
      dirtiness |= Dirtiness::Render;
    }

    if (stretch() != new_props.stretch()) {
      dirtiness |= Dirtiness::Render;
    }

    return dirtiness;
  }

 private:
  // if extent is specified and different from the extent once loaded...
  stx::Option<SelfExtent> extent_;
  BorderRadius border_radius_;
  stx::Option<Extent> aspect_ratio_;
  // same cond app to extent
  stx::Option<Rect> crop_;
  ImageStretch stretch_ = ImageStretch::None;
};

struct Image : public Widget {
  // Note: we can use sizing here because width and height is known ahead of
  // time.
  static constexpr Extent DefaultImageExtent = Extent{50, 50};

  Image(MemoryImageSource const &source, ImageProps const &props) {
    source_ = source;
    update_props(props);
  }

  Image(FileImageSource const &source, ImageProps const &props) {
    source_ = source;
    update_props(props);
  }

  virtual void draw(Canvas &canvas, AssetManager &asset_manager) override {
    SkCanvas *sk_canvas = canvas.as_skia().unwrap();

    if (std::holds_alternative<MemoryImageSource>(source_)) {
      auto const &image_source = std::get<MemoryImageSource>(source_);
      (void)add_asset(asset_manager, image_source);
      get_asset(asset_manager, image_source)
          .match(
              [&](std::shared_ptr<ImageAsset const> const &image_asset) {
                image_asset->get_ref().match(
                    [](sk_sp<SkSurface> const &texture) {

                    },
                    [](ImageLoadError) {});
              },
              [](AssetError) {});
    } else {
      auto const &image_source = std::get<FileImageSource>(source_);
      (void)add_asset(asset_manager, image_source);
      get_asset(asset_manager, image_source)
          .match([&](std::shared_ptr<ImageAsset const> const &image_asset) {},
                 [](AssetError) {});
    }
  }

  // copy() copy_err()
  // copy() copy_none()
  // void update_source(MemoryImageSource const &);
  // void update_source(FileImageSource const &);
  bool is_memory_sourced() const {
    return std::holds_alternative<MemoryImageSource>(source_);
  }

  bool is_file_sourced() const {
    return std::holds_alternative<FileImageSource>(source_);
  }

  void update_props(ImageProps const &props) {
    // once a file image is loaded and no extent if provided we need to perform
    // a re-layout
    dirtiness_ = props_.diff(props);
    props_ = props;
  }

  //! process any event you need to process here.
  // tick should return Dirtiness and its link tree is then consulted?
  virtual void tick(std::chrono::nanoseconds) {
    if ((dirtiness_ & Dirtiness::Render) != Dirtiness::None) {
      Widget::mark_render_dirty();
    }

    if ((dirtiness_ & Dirtiness::Layout) != Dirtiness::None) {
      Widget::mark_layout_dirty();
    }

    if ((dirtiness_ & Dirtiness::Children) != Dirtiness::None) {
      Widget::mark_children_dirty();
    }
  }

 private:
  ImageProps props_;
  Dirtiness dirtiness_;
  std::variant<stx::NoneType, MemoryImageSource, FileImageSource> source_;
};

}  // namespace ui
};  // namespace vlk
