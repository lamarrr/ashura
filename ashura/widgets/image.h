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
namespace gui
{

struct FileImageSource
{
  stx::String path;
};

struct NetworkImageSource
{
  stx::String uri;
};

using ImageSource = std::variant<ImageBuffer, FileImageSource,
                                 NetworkImageSource, stx::NoneType>;

// TODO(lamarrr): image width and height should have a size limit for it to be
// scaled to when stored on the gpu
// TODO(lamarrr): fix image layout
struct ImageProps
{
  ImageSource      source = stx::None;
  Constraint2D     size;
  BorderRadius     border_radius;
  stx::Option<f32> aspect_ratio;
  bool             resize_on_load = true;
  Vec4             tint           = {1, 1, 1, 1};
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
// TODO(lamarrr): this is a static image. no unloading is presently done. do
// that ONCE props change
//
// TODO(lamarrrr): resource multiple deletion with move???
//
//
struct Image : public Widget
{
  explicit Image(ImageProps image_props) : props{std::move(image_props)}
  {
  }

  STX_DISABLE_COPY(Image)
  STX_DEFAULT_MOVE(Image)

  virtual WidgetDebugInfo get_debug_info(Context &ctx) override
  {
    return WidgetDebugInfo{.type = "Image"};
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
  {
    Vec2 extent = props.size.resolve(allocated_size);
    if (props.aspect_ratio.is_some())
    {
      f32 aspect_ratio = props.aspect_ratio.value();
      return Vec2{std::min(extent.y * aspect_ratio, extent.x),
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
        Vec2 uv0 = {0, 0};
        Vec2 uv1 = {1, 1};

        if (props.aspect_ratio.is_some())
        {
          f32 aspect_ratio = props.aspect_ratio.value();

          Vec2 clipped_extent{std::min(image_extent.y * aspect_ratio,
                                       static_cast<f32>(image_extent.x)),
                              std::min(image_extent.x / aspect_ratio,
                                       static_cast<f32>(image_extent.y))};

          Vec2 original_extent{static_cast<f32>(image_extent.x),
                               static_cast<f32>(image_extent.y)};

          Vec2 space = original_extent - clipped_extent;

          uv0.x = (space.x / 2) / original_extent.x;
          uv1.x = (space.x / 2 + clipped_extent.x) / original_extent.x;
          uv0.y = (space.y / 2) / original_extent.y;
          uv1.y = (space.y / 2 + clipped_extent.y) / original_extent.y;
        }

        Vec4 border_radius = props.border_radius.resolve(area.extent);

        if (border_radius == Vec4{0, 0, 0, 0})
        {
          canvas.draw_image(image, area.offset, area.extent, props.tint, uv0,
                            uv1);
        }
        else
        {
          canvas.draw_rounded_image(image, area.offset, area.extent,
                                    border_radius, 360, props.tint, uv0, uv1);
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
    ImageManager *mgr =
        ctx.get_subsystem<ImageManager>("ImageManager").unwrap();
    ImageLoader *loader =
        ctx.get_subsystem<ImageLoader>("ImageLoader").unwrap();

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
          image_load_future = stx::Some(loader->load_from_file(
              std::get<FileImageSource>(props.source).path));
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
              props.size =
                  Constraint2D::absolute(static_cast<f32>(buffer.extent.x),
                                         static_cast<f32>(buffer.extent.y));
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

  ImageProps props;
  ImageState state = ImageState::Inactive;
  gfx::image image = 0;
  Extent     image_extent;
  stx::Option<stx::Future<stx::Result<ImageBuffer, ImageLoadError>>>
      image_load_future;
};
}        // namespace gui
}        // namespace ash
