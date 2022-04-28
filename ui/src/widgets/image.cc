#include "vlk/ui/widgets/image.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "stx/fn.h"
#include "vlk/image_asset.h"
#include "vlk/subsystems/asset_loader.h"
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

  // extent has already been taken care of
  Extent const widget_extent = canvas.extent();

  if (storage_.image.is_some()) {
    auto const &image_future = storage_.image.value().image.future_;

    if (image_future.is_done()) {
      switch (image_future.fetch_status()) {
        case stx::FutureStatus::Completed: {
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
                      to_skia(storage_.props.border_radius());

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
                    SkSamplingOptions{}, (SkPaint *)nullptr,
                    SkCanvas::kFast_SrcRectConstraint);

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

void Image::tick(std::chrono::nanoseconds interval,
                 SubsystemsContext const &context) {
  // TODO(lamarrr): update widget extent to extent of the actual texture
  // Future awaiter to await results of futures?
  auto tmp = context.get("VLK_AssetLoader").unwrap();
  auto asset_loader = tmp.handle->as<AssetLoader>().unwrap();

  storage_.image.match(
      [interval](impl::LoadedImage &image) { image.image.tick(interval); },
      []() {});

  if ((storage_.image.is_some() &&
       storage_.props.source_ref() != storage_.image.value().source) ||
      storage_.image.is_none()) {
    auto const &source = storage_.props.source_ref();

    if (std::holds_alternative<MemoryImageSource>(source)) {
      storage_.image = stx::Some{impl::LoadedImage{
          source,
          FutureAwaiter{
              asset_loader->load_image(std::get<MemoryImageSource>(source)),
              stx::fn::rc::make_functor(stx::os_allocator, [this]() {
                Widget::mark_render_dirty();
                Widget::mark_layout_dirty();
              }).unwrap()}}};
    } else {
      storage_.image = stx::Some{impl::LoadedImage{
          source,
          FutureAwaiter{
              asset_loader->load_image(std::get<FileImageSource>(source)),
              stx::fn::rc::make_functor(stx::os_allocator, [this]() {
                Widget::mark_render_dirty();
                Widget::mark_layout_dirty();
              }).unwrap()}}};
    }
  }

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

  // we failed to load the image, we proceed to render error image.
  // the error image uses whatever extent the widget has available.
  /*if (storage_.state == ImageState::LoadFailed) {
    // no-op
  }
  */

  if (diff_ != impl::ImageDiff::None) {
    WidgetDirtiness dirtiness = impl::map_diff(diff_);

    // TODO(lamarrr): abstract the default extent to a constexpr global
    // visible to the user
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