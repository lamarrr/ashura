

#include "vlk/ui/widgets/box.h"

#include <variant>

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkImageFilters.h"
#include "vlk/asset_loader.h"
#include "vlk/asset_registry.h"
#include "vlk/ui/sk_utils.h"

namespace vlk {
namespace ui {

namespace impl {

//! 60Hz * (60 seconds * 2) = 60Hz * 120 seconds = 2 Minutes @60Hz = 1 Minute
//! @120Hz
constexpr Ticks default_texture_asset_timeout = Ticks{60 * 60 * 2};

constexpr WidgetDirtiness map_diff(BoxDiff diff) {
  WidgetDirtiness dirtiness = WidgetDirtiness::None;

  if ((diff & BoxDiff::Extent) != BoxDiff::None) {
    dirtiness |= WidgetDirtiness::Layout;
  }

  if ((diff & BoxDiff::Padding) != BoxDiff::None) {
    dirtiness |= WidgetDirtiness::Layout;
  }

  if ((diff & BoxDiff::Border) != BoxDiff::None) {
    dirtiness |= WidgetDirtiness::Layout | WidgetDirtiness::Render;
  }

  if ((diff & BoxDiff::BorderRadius) != BoxDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & BoxDiff::Color) != BoxDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & BoxDiff::Blur) != BoxDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & BoxDiff::Blend) != BoxDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & BoxDiff::BackgroundImage) != BoxDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & BoxDiff::Fit) != BoxDiff::None) {
    dirtiness |= WidgetDirtiness::Render;
  }

  if ((diff & BoxDiff::Flex) != BoxDiff::None) {
    dirtiness |= WidgetDirtiness::Layout;
  }

  return dirtiness;
}

inline BoxDiff box_props_diff(BoxProps const &props,
                              BoxProps const &new_props) {
  BoxDiff diff = BoxDiff::None;

  if (props.extent() != new_props.extent()) {
    diff |= BoxDiff::Extent;
  }

  if (props.padding() != new_props.padding()) {
    diff |= BoxDiff::Padding;
  }

  if (props.border() != new_props.border()) {
    diff |= BoxDiff::Border;
  }

  if (props.border_radius() != new_props.border_radius()) {
    diff |= BoxDiff::BorderRadius;
  }

  if (props.color() != new_props.color()) {
    diff |= BoxDiff::Color;
  }

  if (props.blur() != new_props.blur()) {
    diff |= BoxDiff::Blur;
  }

  if (props.blend() != new_props.blend()) {
    diff |= BoxDiff::Blend;
  }

  if (props.image_ref() != new_props.image_ref()) {
    diff |= BoxDiff::BackgroundImage;
  }

  if (props.fit() != new_props.fit()) {
    diff |= BoxDiff::Fit;
  }

  if (props.flex() != new_props.flex()) {
    diff |= BoxDiff::Flex;
  }

  return diff;
}

// TODO(lamarrr): move to sk utils
constexpr std::array<SkVector, 4> to_skia(BorderRadius const &border_radius) {
  return {
      SkVector::Make(border_radius.top_left, border_radius.top_left),
      SkVector::Make(border_radius.top_right, border_radius.top_right),
      SkVector::Make(border_radius.bottom_left, border_radius.bottom_left),
      SkVector::Make(border_radius.bottom_right, border_radius.bottom_right),
  };
}

/*
inline auto get_image_asset(AssetManager &asset_manager,
                            ImageSource const &source)
    -> stx::Result<std::shared_ptr<ImageAsset const>, AssetError> {
  if (std::holds_alternative<MemoryImageSource>(source)) {
    return get_asset(asset_manager, std::get<MemoryImageSource>(source));
  } else if (std::holds_alternative<FileImageSource>(source)) {
    return get_asset(asset_manager, std::get<FileImageSource>(source));
  } else {
    VLK_PANIC();
  }
}
*/

/*
inline auto add_image_asset(AssetManager &asset_manager,
                            ImageSource const &source)
    -> stx::Result<stx::NoneType, AssetError> {
  if (std::holds_alternative<MemoryImageSource>(source)) {
    return add_asset(asset_manager, std::get<MemoryImageSource>(source));
  } else if (std::holds_alternative<FileImageSource>(source)) {
    return add_asset(asset_manager, std::get<FileImageSource>(source));
  } else {
    VLK_PANIC();
  }*/

}  // namespace impl

void Box::update_props(BoxProps new_props) {
  Widget::init_type(WidgetType::Render);
  Widget::init_is_flex(true);

  diff_ |= impl::box_props_diff(storage_.props, new_props);

  storage_.props = std::move(new_props);
  // actually update
}

void Box::draw(Canvas &canvas) {
  SkCanvas &sk_canvas = canvas.to_skia();

  Extent const widget_extent = canvas.extent();

  Border const border = storage_.props.border();
  bool const border_visible = border.color.visible() && border.edges != Edges{};

  BorderRadius const border_radius = storage_.props.border_radius();

  uint32_t const border_x =
      std::min(border.edges.left + border.edges.right, widget_extent.width);
  uint32_t const border_y =
      std::min(border.edges.top + border.edges.bottom, widget_extent.height);

  uint32_t const border_width_left = std::min(border.edges.left, border_x);
  uint32_t const border_width_right = border_x - border_width_left;

  uint32_t const border_width_top = std::min(border.edges.top, border_y);
  uint32_t const border_width_bottom = border_y - border_width_top;

  Extent const content_extent =
      Extent{widget_extent.width - border_x, widget_extent.height - border_y};

  SkPaint border_paint;
  border_paint.setAntiAlias(true);
  border_paint.setColor(border.color.to_argb());

  SkRRect content_rrect;
  SkRRect outer_border_rrect;

  auto const outer_border_radii = impl::to_skia(border_radius);
  auto const inner_border_radii = impl::to_skia(border_radius);

  content_rrect.setRectRadii(
      SkRect::MakeXYWH(border_width_left, border_width_top,
                       content_extent.width, content_extent.height),
      inner_border_radii.data());

  outer_border_rrect.setRectRadii(
      SkRect::MakeXYWH(0, 0, widget_extent.width, widget_extent.height),
      outer_border_radii.data());

  sk_canvas.save();

  // cut out content area
  sk_canvas.clipRRect(content_rrect, true);

  Blur blur = storage_.props.blur();

  // draw backdrop blur filter
  if (blur.visible()) {
    SkPaint blur_paint;
    blur_paint.setImageFilter(
        SkImageFilters::Blur(static_cast<float>(blur.width),
                             static_cast<float>(blur.height), nullptr));
    sk_canvas.saveLayer(
        to_sk_rect(Rect{Offset{border_width_left, border_width_top},
                        Extent{content_extent.width, content_extent.height}}),
        &blur_paint);
  }

  

// TODO(lamarrr):

storage_.background.match([](impl::BoxBackgroundImage image){

image.image

}, [](){});

    auto image_draw_op = [&]() {
      storage_.asset.as_cref().match(
          [&](std::shared_ptr<ImageAsset const> const &image_asset) {
            image_asset->get_ref().match(
                [&](sk_sp<SkImage> const &image) {
                  // TODO(lamarrr): use specific portion or fit of the image?
                  // TODO(lamarrr): direct write if not transparent image

                  BoxFit const fit = storage_.props.fit();

                  // TODO(lamarrr): optimize this for transparency blending
    (paint
                  // blend mode), we might need more info from the image
    decoding
                  // process

                  if (fit == BoxFit::None) {
                    sk_canvas.drawImageRect(
                        image,
                        SkRect::MakeXYWH(0, 0, image->width(), image->height()),
                        to_sk_rect(Rect{
                            Offset{border_width_left, border_width_top},
                            Extent{content_extent.width,
    content_extent.height}}), nullptr); } else if (fit == BoxFit::Cover) {
                    // TODO
                  } else if (fit == BoxFit::Contain) {
                  } else if (fit == BoxFit::Fill) {
                  }
                },
                [](ImageLoadError) {});
          },
          []() {});
    };
    

  auto color_draw_op = [&] {
    if (!storage_.props.color().transparent()) {
      SkPaint background_color_paint;
      background_color_paint.setColor(storage_.props.color().to_argb());
      sk_canvas.drawRect(
          SkRect::MakeXYWH(border_width_left, border_width_top,
                           content_extent.width, content_extent.height),
          background_color_paint);
    }
  };

  // draw background image and color
  if (storage_.props.blend() == BoxBlend::ImageOver) {
    color_draw_op();
    // image_draw_op();
  } else {
    // image_draw_op();
    color_draw_op();
  }

  // remove saveLayer and clip for backdrop filter blur
  if (blur.visible()) {
    sk_canvas.restore();
  }

  // restore clip for border
  sk_canvas.restore();

  // border drawing
  sk_canvas.drawDRRect(outer_border_rrect, content_rrect, border_paint);
}

void Box::tick(std::chrono::nanoseconds, SubsystemsContext const &context) {
  auto tmp = context.get("AssetLoader").unwrap();
  auto asset_loader = tmp.handle->as<AssetLoader>().unwrap();
  //
  //
  // TODO(lamarrr):
  // WE NEED:
  // - a way to load images only once, i.e. not loading and reloading on every
  // tick
  // - a way to backup widget state so we don't get partial or incorrect paints
  //
  //  if (storage_.props.image_ref().is_some()) {
  //   storage_.background.as_cref().match([]() {}, []() {});
  //}

  if (storage_.state == BoxState::BackgroundStale && !Widget::is_stale() &&
      storage_.props.image_ref().is_some()) {
    // check if image is already loaded
    auto next_image = storage_.props.image_ref().value();
    auto &loaded_image = storage_.background;

    // check if tags are matching
    // if matching, we don't need to do any work, otherwise we load the new
    // image
    if (loaded_image.is_some() && loaded_image.value().source == next_image) {
      // already loaded, do no work
    } else {
      if (std::holds_alternative<MemoryImageSource>(next_image)) {
        auto &source = std::get<MemoryImageSource>(next_image);
        storage_.background = Some(
            impl::BoxBackgroundImage{source, asset_loader->load_image(source)});

      } else {
        auto &source = std::get<FileImageSource>(next_image);
        storage_.background = Some(
            impl::BoxBackgroundImage{source, asset_loader->load_image(source)});
      }
    }

    

    Widget::mark_render_dirty();
  }
  /*
    if (storage_.state == BoxState::BackgroundStale && !Widget::is_stale() &&
        storage_.props.image_ref().is_some()) {
      auto image_source = storage_.props.image_ref().copy().unwrap();
      if (std::holds_alternative<MemoryImageSource>(image_source)) {
        auto tag = std::get<MemoryImageSource>(image_source).get_tag().as_str();
        auto pos = cache.find(tag);
        if (pos == cache.end()) {
          auto insert_pos =
              cache.emplace(tag, asset_loader->load_image(
                                     std::get<MemoryImageSource>(image_source)));
        }
      } else {
        auto tag = std::get<FileImageSource>(image_source).get_tag().as_str();
        auto pos = cache.find(tag);
        if (pos == cache.end()) {
          auto insert_pos = cache.emplace(
              tag,
              asset_loader->load_image(std::get<FileImageSource>(image_source)));
        }
      }

      storage_.state = BoxState::BackgroundLoading;

      // mark the widget as dirty so a loading image is displayed, or the image
      // itself if it is already loaded
      Widget::mark_render_dirty();
    }
    */

  // we've submitted the image to the asset manager or it has previously
  // been submitted by another widget and we are awaiting the status of
  // the image
  /*if (storage_.state == BoxState::BackgroundLoading) {

auto image_source = storage_.props.image_ref().copy().unwrap();
    if (std::holds_alternative<MemoryImageSource>(image_source)) {
      auto tag = std::get<MemoryImageSource>(image_source).get_tag().as_str();
    } else {
      auto tag = std::get<FileImageSource>(image_source).get_tag().as_str();
    cache.find(tag)->second.copy;

    }
*/
  /*
  storage_.state =
      impl::get_image_asset(asset_manager, storage_.props.image_ref().value())
          .match(
              [&](auto &&asset) -> BoxState {
                return asset->get_ref().match(
                    [&](auto &) {
                      storage_.asset = stx::Some(std::move(asset));
                      return BoxState::BackgroundLoaded;
                    },
                    [&](ImageLoadError error) {
                      VLK_WARN("Failed to load image for {}, error: {}",
                               format(*this), format(error));
                      return BoxState::BackgroundLoadFailed;
                    });
              },
              [&](AssetError error) -> BoxState {
                switch (error) {
                    // image is still loading
                  case AssetError::IsLoading:
                    return BoxState::BackgroundLoading;
                  default:
                    VLK_PANIC("Unexpected State");
                }
              });
*/
  // if state changed from image loading (to success or failure), mark as
  // dirty so the failure or sucess image can be displayed
  /*    if (storage_.state != BoxState::BackgroundLoading) {
        Widget::mark_render_dirty();
      }
    }

    // the image has been successfully loaded
    if (storage_.state == BoxState::BackgroundLoaded) {
      // image asset usage tracking
      if (Widget::is_stale()) {
        storage_.asset_stale_ticks++;
        // mark widget as dirty since the asset has been discarded after not
        // being used for long
        if (storage_.asset_stale_ticks >= impl::default_texture_asset_timeout) {
    //      storage_.asset = stx::None;
          Widget::mark_render_dirty();
          storage_.state = BoxState::BackgroundStale;
        }
      } else {
        storage_.asset_stale_ticks.reset();
      }
    }

    // we failed to load the image, we proceed to render error image.
    // the error image uses whatever extent the widget has available.
    if (storage_.state == BoxState::BackgroundLoadFailed) {
      // no-op
    }
  */
  if (diff_ != impl::BoxDiff::None) {
    Widget::update_flex(storage_.props.flex());
    Widget::update_self_extent(storage_.props.extent());
    Padding const padding = storage_.props.padding();
    Border const border = storage_.props.border();
    Widget::update_padding(Padding::trbl(padding.top + border.edges.top,
                                         padding.right + border.edges.right,
                                         padding.bottom + border.edges.bottom,
                                         padding.left + border.edges.left));

    // these diffs are already tracked by the widget system
    WidgetDirtiness dirtiness = impl::map_diff(diff_);

    /*
        if ((diff_ & impl::BoxDiff::BackgroundImage) != impl::BoxDiff::None) {
          if (storage_.props.image_ref().is_none())
            storage_.background = Some(impl::BoxBackgroundImage{})
        }
    */
    Widget::add_dirtiness(dirtiness);
    diff_ = impl::BoxDiff::None;
  }
}

}  // namespace ui
}  // namespace vlk
