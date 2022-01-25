#include "vlk/ui/widgets/image.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "stx/fn.h"
#include "vlk/asset_loader.h"
#include "vlk/image_asset.h"
#include "vlk/ui/sk_utils.h"

namespace vlk {

namespace ui {


namespace impl {

constexpr WidgetDirtiness map_diff(ImageDiff diff) {
  WidgetDirtiness dirtiness = WidgetDirtiness::None;

  if ((diff & ImageDiff::Source) != ImageDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  if ((diff & ImageDiff::Extent) != ImageDiff::None) {
    dirtiness |= WidgetDirtiness::Layout;
  }

  if ((diff & ImageDiff::BorderRadius) != ImageDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & ImageDiff::AspectRatio) != ImageDiff::None) {
    dirtiness |= WidgetDirtiness::Layout;
  }

  return dirtiness;
}

inline ImageDiff image_props_diff(ImageProps const &props,
                                  ImageProps const &new_props) {
  ImageDiff diff = ImageDiff::None;

  if (props.source_ref() != new_props.source_ref()) {
    diff |= ImageDiff::Source;
  }

  if (props.extent() != new_props.extent()) {
    diff |= ImageDiff::Extent;
  }

  if (props.border_radius() != new_props.border_radius()) {
    diff |= ImageDiff::BorderRadius;
  }

  if (props.aspect_ratio() != new_props.aspect_ratio()) {
    diff |= ImageDiff::AspectRatio;
  }

  return diff;
}

// TODO(lamarrr): this should probably be moved to an impl conversions file
constexpr std::array<SkVector, 4> to_skia(BorderRadius const &border_radius) {
  return {
      SkVector::Make(border_radius.top_left, border_radius.top_left),
      SkVector::Make(border_radius.top_right, border_radius.top_right),
      SkVector::Make(border_radius.bottom_left, border_radius.bottom_left),
      SkVector::Make(border_radius.bottom_right, border_radius.bottom_right),
  };
}

}  // namespace impl

void Image::update_props(ImageProps props) {
  // once a file image is loaded and no extent is provided we'd need to
  // perform a re-layout
  //
  //
  // once the image arrives, we update the prop to use the new extent of the
  // new image
  //
  //

  diff_ |= impl::image_props_diff(storage_.props, props);

  storage_ = impl::ImageStorage{std::move(props)};
}
// TODO(lamarrr): once the asset is discarded, the tick calls marK_render_dirty
// which then triggers another draw call, we should wait for another draw call
// before marking it as dirty? partial invalidation?
//
//
// we are thus discarding the texture whilst it is still in view
//
//
// we can somehow notify the widget via the tile_cache we only need to set a
// value in the widget
//
//
// we only need to record for the in_focus tiles and discard recordings for
// non-infocus tiles and thus ignore their dirtiness an almost infinite-extent
// widget for example will not be drawable if complex enough since we'd need to
// have recordings for all of the tiles, both active and inactive ones
//
// draw methods could also perform offscreen rendering which could be costly if
// the widget is not in view
//
// when the texture hasn't been used for long, it is marked as discarded and
// then discarded and immediately added back, and marked dirty unnecassarily?
//

void Image::draw(Canvas &canvas) {
  SkCanvas &sk_canvas = canvas.to_skia();

  // TODO(lamarrr): what's this for?
  /*{
    SkPaint paint;
    paint.setColor(SkColorSetARGB(125, 255, 0, 0));
    sk_canvas.drawRect(vlk::to_sk_rect(Rect{{}, canvas.extent()}), paint);
  }*/

  // extent has already been taken care of
  Extent const widget_extent = canvas.extent();

  if (storage_.image.is_some()) {
    auto const &image_future = storage_.image.value().image;

    if (image_future.is_done()) {
      switch (image_future.fetch_status()) {
        case FutureStatus::Completed: {
          auto *load_result = image_future.ref().unwrap();
          load_result->match(
              [&](ImageAsset asset) {
                sk_sp texture = asset.get_raw();
                int const texture_width = texture->width();
                int const texture_height = texture->height();
                Extent const texture_extent =
                    Extent{static_cast<uint32_t>(texture_width),
                           static_cast<uint32_t>(texture_height)};

                sk_canvas.save();

                if (storage_.props.border_radius() != BorderRadius::all(0)) {
                  SkRRect round_rect;

                  std::array const border_radii =
                      impl::to_skia(storage_.props.border_radius());

                  round_rect.setRectRadii(
                      SkRect::MakeWH(widget_extent.width, widget_extent.height),
                      border_radii.data());

                  sk_canvas.clipRRect(round_rect, true);
                }

                // due to aspect ratio cropping we need to place image at the
                // center.
                Extent const roi =
                    storage_.props.aspect_ratio().is_some()
                        ? aspect_ratio_trim(
                              storage_.props.aspect_ratio().unwrap(),
                              texture_extent)
                        : texture_extent;

                int const start_x = (texture_width - roi.width) / 2;
                int const start_y = (texture_height - roi.height) / 2;

                sk_canvas.drawImageRect(
                    texture,
                    SkRect::MakeXYWH(start_x, start_y, roi.width, roi.height),
                    SkRect::MakeXYWH(0, 0, widget_extent.width,
                                     widget_extent.height),
                    nullptr);

                sk_canvas.restore();
              },
              [&](ImageLoadError error) { draw_error_image(canvas); });
        } break;
        default:
          break;
      }
    }
  }
}

void Image::tick(std::chrono::nanoseconds, SubsystemsContext const &context) {
  // TODO(lamarrr): update widget extent to extent of the actual texture
  // Future awaiter to await results of futures?
  auto tmp = context.get("AssetLoader").unwrap();
  auto asset_loader = tmp.handle->as<AssetLoader>().unwrap();

  if ((storage_.image.is_some() &&
       storage_.props.source_ref() != storage_.image.value().source) ||
      storage_.image.is_none()) {
    auto const &source = storage_.props.source_ref();

    if (std::holds_alternative<MemoryImageSource>(source)) {
      storage_.image = Some{impl::LoadedImage{
          source,
          asset_loader->load_image(std::get<MemoryImageSource>(source))}};
    } else {
      storage_.image = Some{impl::LoadedImage{
          source, asset_loader->load_image(std::get<FileImageSource>(source))}};
    }

    Widget::mark_render_dirty();
  }

  // we've submitted the image to the asset manager or it has previously
  // been submitted by another widget and we are awaiting the status of
  // the image
  // if (storage_.state == ImageState::Loading) {
  /*
  storage_.state =
      impl::get_image_asset(asset_manager, storage_.props.source_ref())
          .match(
              [&](auto &&asset) -> ImageState {
                return asset->get_ref().match(
                    [&](auto &) {
                      storage_.asset = stx::Some(std::move(asset));
                      return ImageState::Loaded;
                    },
                    [&](ImageLoadError error) {
                      VLK_WARN("Failed to load image for {}, error: {}",
                               format(*this), format(error));
                      return ImageState::LoadFailed;
                    });
              },
              [&](AssetError error) -> ImageState {
                switch (error) {
                    // image is still loading
                  case AssetError::IsLoading:
                    return ImageState::Loading;
                  default:
                    VLK_PANIC("Unexpected State");
                }
              });
*/

  // if state changed from image loading (to success or failure), mark as
  // dirty so the failure or sucess image can be displayed
  // if (storage_.state != ImageState::Loading) {
  // Widget::mark_render_dirty();
  //}

  // if the image loaded correctly and the user did not already provide an
  // extent, we need to request for a relayout to the loaded image asset's
  // extent. if a reflow is needed, immediately return so the relayout can
  // be processed by the system before rendering
  // if (storage_.state == ImageState::Loaded &&
  //   storage_.props.extent().is_none()) {
  /*  sk_sp texture = storage_.asset.value()->get_ref().value();
    Widget::update_self_extent(
        SelfExtent::absolute(texture->width(), texture->height()));*/
  //  return;
  //}
  //}

  // the image has been successfully loaded and the required layout for the
  // image has been established
  /*if (storage_.state == ImageState::Loaded) {
    // image asset usage tracking
    if (Widget::is_stale()) {
      storage_.asset_stale_ticks++;
      // mark widget as dirty since the asset has been discarded after not
      // being used for long
      if (storage_.asset_stale_ticks >= impl::default_texture_asset_timeout) {
        storage_.asset = stx::None;
        Widget::mark_render_dirty();
        storage_.state = ImageState::Stale;
      }
    } else {
      storage_.asset_stale_ticks.reset();
    }
  }
  */

  // we failed to load the image, we proceed to render error image.
  // the error image uses whatever extent the widget has available.
  /*if (storage_.state == ImageState::LoadFailed) {
    // no-op
  }
  */

  if (diff_ != impl::ImageDiff::None) {
    WidgetDirtiness dirtiness = impl::map_diff(diff_);

    // TODO(lamarrr): abstract the default extent to a constexpr global visible
    // to the user
    storage_.props.extent().match(
        [&](SelfExtent extent) { Widget::update_self_extent(extent); },
        [&]() { Widget::update_self_extent(SelfExtent::absolute(100, 100)); });

    storage_.props.aspect_ratio().match(
        [&](Extent) { Widget::update_needs_trimming(true); },
        [&]() { Widget::update_needs_trimming(false); });

    Widget::add_dirtiness(dirtiness);
    diff_ = impl::ImageDiff::None;
  }
}

}  // namespace ui

}  // namespace vlk