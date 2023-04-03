#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "ashura/base64.h"
#include "ashura/image.h"
#include "ashura/image_decoder.h"
#include "ashura/plugins/image_bundle.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "stx/option.h"
#include "stx/scheduler/scheduling/schedule.h"
#include "stx/span.h"
#include "stx/string.h"

namespace ash
{

struct MemoryImageSource
{
  ImageBuffer buffer;
};

struct FileImageSource
{
  stx::String path;
};

struct NetworkImageSource
{
  stx::String uri;
};

using ImageSource =
    std::variant<MemoryImageSource, FileImageSource, NetworkImageSource, stx::NoneType>;

struct ImageProps
{
  ImageSource      source = stx::None;
  constraint       width;
  constraint       height;
  vec4             border_radius;
  stx::Option<f32> aspect_ratio;
  bool             resize_on_load = true;
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
/// - Add image to asset bundle and upload to GPU for fast transfers (i.e. zero
/// copy over PCIE from CPU to GPU during rendering)
/// - once the image arrives, get a reference to it
/// - Update widget state to show that the image is loading
///
static f32 rotation = 0;

struct Image : public Widget
{
  explicit Image(ImageProps image_props) :
      props{std::move(image_props)}
  {}

  virtual WidgetInfo get_info() override
  {
    return WidgetInfo{.type = "Image", .id = Widget::id};
  }

  virtual Layout layout(rect area) override
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

  virtual void draw(gfx::Canvas &canvas, rect area) override
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
        f32 s0 = 0;
        f32 s1 = 1;
        f32 t0 = 0;
        f32 t1 = 1;
        canvas.rotate(rotation, rotation, 0);

        if (props.aspect_ratio.is_some())
        {
          f32 aspect_ratio = props.aspect_ratio.value();

          vec2 clipped_extent{
              std::min(image_extent.height * aspect_ratio, AS(f32, image_extent.width)),
              std::min(image_extent.width / aspect_ratio, AS(f32, image_extent.height))};

          vec2 original_extent{AS(f32, image_extent.width), AS(f32, image_extent.height)};

          vec2 space = original_extent - clipped_extent;

          s0 = (space.x / 2) / original_extent.x;
          s1 = (space.x / 2 + clipped_extent.x) / original_extent.x;
          t0 = (space.y / 2) / original_extent.y;
          t1 = (space.y / 2 + clipped_extent.y) / original_extent.y;
        }

        if (props.border_radius == vec4{0, 0, 0, 0})
        {
          canvas.draw_image(image, area, s0, t0, s1, t1);
        }
        else
        {
          canvas.draw_rounded_image(
              image, area, rect{.offset = vec2{s0, t0}, .extent = vec2{s1 - s0, t1 - t0}},
              props.border_radius, 360);
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

  virtual void tick(WidgetContext &context, std::chrono::nanoseconds interval) override
  {
    ImageBundle *bundle = context.get_plugin<ImageBundle>("ImageBundle").unwrap();

    switch (state)
    {
      case ImageState::Inactive:
      {
        if (std::holds_alternative<MemoryImageSource>(props.source))
        {
          MemoryImageSource const &source = std::get<MemoryImageSource>(props.source);
          image                           = bundle->add(source.buffer, false);
          state                           = ImageState::Loaded;
        }
        else if (std::holds_alternative<FileImageSource>(props.source))
        {
          image_load_future = stx::Some(stx::sched::fn(
              *context.task_scheduler,
              [path = std::get<FileImageSource>(props.source)
                          .path.copy(stx::os_allocator)
                          .unwrap()]() -> stx::Result<ImageBuffer, ImageLoadError> {
                if (!std::filesystem::exists(path.c_str()))
                  return stx::Err(ImageLoadError::InvalidPath);

                std::ifstream stream{path.c_str(), std::ios::ate | std::ios::binary};

                ASH_CHECK(stream.is_open());

                usize file_size = stream.tellg();

                stx::Memory memory =
                    stx::mem::allocate(stx::os_allocator, file_size).unwrap();

                stream.seekg(0);

                stream.read(AS(char *, memory.handle), file_size);
                spdlog::info("decided");
                return decode_image(stx::Span{AS(u8 const *, memory.handle), file_size});
              },
              stx::INTERACTIVE_PRIORITY, stx::TaskTraceInfo{}));
          state             = ImageState::Loading;
        }
        else if (std::holds_alternative<NetworkImageSource>(props.source))
        {
          ASH_PANIC("unimplemented");
        }
      }
      break;
      case ImageState::Loading:
      {
        if (image_load_future.value().is_done())
        {
          stx::Result load_result = image_load_future.value().move().unwrap();
          // TODO(lamarrr): log trace and error
          if (load_result.is_ok())
          {
            spdlog::info("loaded image successfully");
            ImageBuffer &buffer = load_result.value();
            image               = bundle->add(buffer, false);
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
            spdlog::error("failed to load image");
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

  virtual void on_mouse_enter(WidgetContext &context, vec2 screen_position,
                              quad quad) override
  {
    spdlog::info("mouse over");
  }

  virtual void on_mouse_leave(WidgetContext &context, stx::Option<vec2> screen_position)
  {
    spdlog::info("mouse leave");
  }

  constexpr virtual void on_click(WidgetContext &context, MouseButton button,
                                  vec2 screen_position, u32 nclicks, quad quad)
  {
    if (button == MouseButton::Secondary)
    {
      props.border_radius = vec4{props.border_radius.x + 10, props.border_radius.y + 10,
                                 props.border_radius.z + 10, props.border_radius.w + 10};
    }
    else
    {
      rotation += 1;
    }
  }

  virtual simdjson::dom::element save(WidgetContext         &context,
                                      simdjson::dom::parser &parser) override
  {
    std::string      source;
    std::string_view source_type;
    ash::extent      extent;

    // T0D0(lamarrr): format
    if (std::holds_alternative<MemoryImageSource>(props.source))
    {
      MemoryImageSource &memory_source = std::get<MemoryImageSource>(props.source);
      stx::Span          pixels        = memory_source.buffer.span();
      source                           = base64_encode(
          stx::Span{reinterpret_cast<char const *>(pixels.data()), pixels.size()});
      source_type = "memory";
      extent      = memory_source.buffer.extent;
    }
    else if (std::holds_alternative<FileImageSource>(props.source))
    {
      source      = std::get<FileImageSource>(props.source).path;
      source_type = "file";
    }
    else if (std::holds_alternative<NetworkImageSource>(props.source))
    {
      source      = std::get<NetworkImageSource>(props.source).uri;
      source_type = "network";
    }

    std::string json = fmt::format(
        fmt::runtime(R"({{"source": "{}",
    "source_type": "{}",
    "extent_width": {},
    "extent_height": {},
    "width_bias": {},
    "width_scale": {},
    "width_min": {},
    "width_max": {},
    "width_min_rel": {},
    "width_max_rel": {},
    "height_bias": {},
    "height_scale": {},
    "height_min": {},
    "height_max": {},
    "height_min_rel": {},
    "height_max_rel": {},
    "border_radius_x": {},
    "border_radius_y": {},
    "border_radius_z": {},
    "border_radius_w": {},
    "has_aspect_ratio": {},
    "aspect_ratio": {},
    "resize_on_load": {},
    "alt": "{}"}})"),
        source, source_type, extent.width, extent.height, props.width.bias,
        props.width.scale, props.width.min, props.width.max, props.width.min_rel,
        props.width.max_rel, props.height.bias, props.height.scale, props.height.min,
        props.height.max, props.height.min_rel, props.height.max_rel,
        props.border_radius.x, props.border_radius.y, props.border_radius.z,
        props.border_radius.w, props.aspect_ratio.is_some(),
        props.aspect_ratio.copy().unwrap_or(1.0f), props.resize_on_load,
        std::string_view{props.alt});

    return parser.parse(json.data(), json.size());
  }

  virtual void restore(WidgetContext                &context,
                       simdjson::dom::element const &element) override
  {
    if (image_load_future.is_some())
    {
      image_load_future.value().request_cancel();
      image_load_future = stx::None;
    }

    if (state == ImageState::Loaded)
    {
      ImageBundle *bundle = context.get_plugin<ImageBundle>("ImageBundle").unwrap();
      (void) bundle->remove(image);
    }

    std::string_view source      = element["source"].get_string();
    std::string_view source_type = element["source_type"].get_string();
    ash::extent      extent{.width  = AS(u32, element["extent_width"].get_uint64()),
                            .height = AS(u32, element["extent_height"].get_uint64())};

    if (source_type == "memory")
    {
      // TODO(lamarrr): fix up
      std::string  enc = base64_decode(source);
      stx::Vec<u8> pixels{stx::os_allocator};
      pixels.extend(stx::Span{enc}.as_u8()).unwrap();
      // props.source =
      //     MemoryImageSource{.buffer pixels = std::move(pixels), .extent = extent};
    }
    else if (source_type == "file")
    {
      props.source =
          FileImageSource{.path = stx::string::make(stx::os_allocator, source).unwrap()};
    }
    else if (source_type == "network")
    {
      props.source = NetworkImageSource{
          .uri = stx::string::make(stx::os_allocator, source).unwrap()};
    }

    props.width.bias      = AS(f32, element["width_bias"].get_double());
    props.width.scale     = AS(f32, element["width_scale"].get_double());
    props.width.min       = AS(f32, element["width_min"].get_double());
    props.width.max       = AS(f32, element["width_max"].get_double());
    props.width.min_rel   = AS(f32, element["width_min_rel"].get_double());
    props.width.max_rel   = AS(f32, element["width_max_rel"].get_double());
    props.height.bias     = AS(f32, element["height_bias"].get_double());
    props.height.scale    = AS(f32, element["height_scale"].get_double());
    props.height.min      = AS(f32, element["height_min"].get_double());
    props.height.max      = AS(f32, element["height_max"].get_double());
    props.height.min_rel  = AS(f32, element["height_min_rel"].get_double());
    props.height.max_rel  = AS(f32, element["height_max_rel"].get_double());
    props.border_radius.x = AS(f32, element["border_radius_x"].get_double());
    props.border_radius.y = AS(f32, element["border_radius_y"].get_double());
    props.border_radius.z = AS(f32, element["border_radius_z"].get_double());
    props.border_radius.w = AS(f32, element["border_radius_w"].get_double());

    if (element["has_aspect_ratio"].get_bool())
    {
      props.aspect_ratio = stx::Some(AS(f32, element["aspect_ratio"].get_double()));
    }
    else
    {
      props.aspect_ratio = stx::None;
    }

    props.resize_on_load = element["resize_on_load"].get_bool();
    props.alt =
        stx::string::make(stx::os_allocator, element["alt"].get_string()).unwrap();

    state = ImageState::Inactive;
  }

  ImageProps                                                         props;
  ImageState                                                         state = ImageState::Inactive;
  gfx::image                                                         image = 0;
  extent                                                             image_extent;
  stx::Option<stx::Future<stx::Result<ImageBuffer, ImageLoadError>>> image_load_future;
};

}        // namespace ash
