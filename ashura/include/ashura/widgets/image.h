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
#include "ashura/plugins/image_loader.h"
#include "ashura/plugins/image_manager.h"
#include "ashura/primitives.h"
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
  constraint       width;
  constraint       height;
  vec4             border_radius;
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

int nsegments = 2;
/// Usage Needs
///
/// - Add image to asset manager and upload to GPU for fast transfers (i.e. zero
/// copy over PCIE from CPU to GPU during rendering)
/// - once the image arrives, get a reference to it
/// - Update widget state to show that the image is loading
///
struct Image : public Widget
{
  explicit Image(ImageProps image_props) :
      props{std::move(image_props)}
  {}

  virtual WidgetInfo get_info(Context &context) override
  {
    return WidgetInfo{.type = "Image"};
  }

  virtual Layout layout(Context &context, rect area) override
  {
    f32 width  = props.width.resolve(area.extent.x);
    f32 height = props.height.resolve(area.extent.y);
    if (props.aspect_ratio.is_some())
    {
      f32 aspect_ratio = props.aspect_ratio.value();
      return Layout{.area = rect{.offset = vec2{0, 0},
                                 .extent = vec2{std::min(height * aspect_ratio, width),
                                                std::min(width / aspect_ratio, height)}}};
    }
    else
    {
      return Layout{.area = rect{.offset = area.offset, .extent = vec2{width, height}}};
    }
  }

  virtual void draw(Context &context, gfx::Canvas &canvas, rect area) override
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
        rect_uv texture_region{.uv0 = {0, 0}, .uv1 = {1, 1}};

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

        if (props.border_radius == vec4{0, 0, 0, 0})
        {
          canvas.draw_image(image, area, texture_region, props.tint);
        }
        else
        {
          canvas.draw_rounded_image(image, area, props.border_radius, 360, texture_region, props.tint);
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

    canvas.draw_round_rect_stroke(rect{{100, 100}, {400, 400}}, vec4::splat(25), colors::GREEN, 20, nsegments*10);
    // canvas.draw_circle_stroke(  {100,100}, 200, 10, colors::CYAN, 5  );
  }

  virtual void tick(Context &context, std::chrono::nanoseconds interval) override
  {
    ImageManager *mgr    = context.get_plugin<ImageManager>("ImageManager").unwrap();
    ImageLoader  *loader = context.get_plugin<ImageLoader>("ImageLoader").unwrap();

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
              props.width  = constraint{.bias = AS(f32, buffer.extent.width)};
              props.height = constraint{.bias = AS(f32, buffer.extent.height)};
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

  virtual void on_mouse_down(Context &context, MouseButton button, vec2 mouse_position, u32 nclicks, quad quad)
  {
    nsegments++;
  }

  ImageProps                                                         props;
  ImageState                                                         state = ImageState::Inactive;
  gfx::image                                                         image = 0;
  extent                                                             image_extent;
  stx::Option<stx::Future<stx::Result<ImageBuffer, ImageLoadError>>> image_load_future;
};

}        // namespace ash
