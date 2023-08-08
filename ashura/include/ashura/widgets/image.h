#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "ashura/base64.h"
#include "ashura/image.h"
#include "ashura/image_decoder.h"
#include "ashura/loggers.h"
#include "ashura/primitives.h"
#include "ashura/subsystems/image_loader.h"
#include "ashura/subsystems/image_manager.h"
#include "ashura/widget.h"
#include "stx/option.h"
#include "stx/scheduler/scheduling/schedule.h"
#include "stx/span.h"
#include "stx/string.h"

namespace ash
{

struct FileImageSource
{
  stx::String path;
};

struct NetworkImageSource
{
  stx::String uri;
};

using ImageSource = std::variant<ImageBuffer, FileImageSource, NetworkImageSource, stx::NoneType>;

struct ImageProps
{
  ImageSource      source = stx::None;
  SizeConstraint   size;
  BorderRadius     border_radius;
  stx::Option<f32> aspect_ratio;
  bool             resize_on_load = true;
  color            tint           = colors::WHITE;
  stx::String      alt;
};

enum class ImageState : u8
{
  /// the image has not been in view yet
  Inactive,

  /// the image is loading
  Loading,

  /// the image has been successfully loaded
  Loaded,

  /// a failure occured while loading the image
  LoadFailed
};

/// Usage Needs
///
/// - Add image to asset manager and upload to GPU for fast transfers (i.e. zero
/// copy over PCIE from CPU to GPU during rendering)
/// - once the image arrives, get a reference to it
/// - Update widget state to show that the image is loading
///
// TODO(lamarrr): this is a static image. no unloading is presently done. do that ONCE props change
//
// TODO(lamarrrr): resource multiple deletion with move???
//
//
struct Image : public Widget
{
  explicit Image(ImageProps image_props) :
      props{std::move(image_props)}
  {}

  STX_DISABLE_COPY(Image)
  STX_DEFAULT_MOVE(Image)

  virtual WidgetDebugInfo get_debug_info(Context &ctx) override
  {
    return WidgetDebugInfo{.type = "Image"};
  }

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_allocations, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    vec2 extent = props.size.resolve(allocated_size);
    if (props.aspect_ratio.is_some())
    {
      f32 aspect_ratio = props.aspect_ratio.value();
      return vec2{std::min(extent.y * aspect_ratio, extent.x),
                  std::min(extent.x / aspect_ratio, extent.y)};
    }
    else
    {
      return extent;
    }
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    switch (state)
    {
      case ImageState::Inactive:
      {
      }
      break;
      case ImageState::Loading:
      {
      }
      break;
      case ImageState::Loaded:
      {
        texture_rect texture_region{.uv0 = {0, 0}, .uv1 = {1, 1}};

        if (props.aspect_ratio.is_some())
        {
          f32 aspect_ratio = props.aspect_ratio.value();

          vec2 clipped_extent{
              std::min(image_extent.height * aspect_ratio, AS(f32, image_extent.width)),
              std::min(image_extent.width / aspect_ratio, AS(f32, image_extent.height))};

          vec2 original_extent{AS(f32, image_extent.width), AS(f32, image_extent.height)};

          vec2 space = original_extent - clipped_extent;

          texture_region.uv0.x = (space.x / 2) / original_extent.x;
          texture_region.uv1.x = (space.x / 2 + clipped_extent.x) / original_extent.x;
          texture_region.uv0.y = (space.y / 2) / original_extent.y;
          texture_region.uv1.y = (space.y / 2 + clipped_extent.y) / original_extent.y;
        }

        vec4 border_radius = props.border_radius.resolve(area.extent);

        if (border_radius == vec4{0, 0, 0, 0})
        {
          canvas.draw_image(image, area, texture_region, props.tint);
        }
        else
        {
          canvas.draw_rounded_image(image, area, border_radius, 360, texture_region, props.tint);
        }
      }
      break;
      case ImageState::LoadFailed:
      {
      }
      break;
      default:
      {
      }
      break;
    }
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    ImageManager *mgr    = ctx.get_subsystem<ImageManager>("ImageManager").unwrap();
    ImageLoader  *loader = ctx.get_subsystem<ImageLoader>("ImageLoader").unwrap();

    switch (state)
    {
      case ImageState::Inactive:
      {
        if (std::holds_alternative<ImageBuffer>(props.source))
        {
          ImageBuffer const &source = std::get<ImageBuffer>(props.source);
          image                     = mgr->add(source, false);
          state                     = ImageState::Loaded;
        }
        else if (std::holds_alternative<FileImageSource>(props.source))
        {
          image_load_future = stx::Some(loader->load_from_file(std::get<FileImageSource>(props.source).path));
          state             = ImageState::Loading;
        }
        else if (std::holds_alternative<NetworkImageSource>(props.source))
        {
          ASH_UNIMPLEMENTED();
        }
      }
      break;
      case ImageState::Loading:
      {
        if (image_load_future.value().is_done())
        {
          stx::Result load_result = image_load_future.value().move().unwrap();
          if (load_result.is_ok())
          {
            ImageBuffer &buffer = load_result.value();
            image               = mgr->add(buffer, false);
            state               = ImageState::Loaded;
            if (props.resize_on_load)
            {
              props.size = SizeConstraint::absolute(AS(f32, buffer.extent.width), AS(f32, buffer.extent.height));
            }
            image_extent = buffer.extent;
          }
          else
          {
            state = ImageState::LoadFailed;
          }

          image_load_future = stx::None;
        }
      }
      break;

      case ImageState::Loaded:
      case ImageState::LoadFailed:
      default:
      {
        break;
      }
    }
  }

  ImageProps                                                         props;
  ImageState                                                         state = ImageState::Inactive;
  gfx::image                                                         image = 0;
  extent                                                             image_extent;
  stx::Option<stx::Future<stx::Result<ImageBuffer, ImageLoadError>>> image_load_future;
};

}        // namespace ash
