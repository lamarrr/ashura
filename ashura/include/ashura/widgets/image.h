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

/// Usage Needs
///
/// - Add image to asset manager and upload to GPU for fast transfers (i.e. zero
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

  virtual mat4 get_transform(Context &context) override
  {
    return rotate_x(rotation * PI / 180) * rotate_y(rotation * PI / 180);
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

    char const *str       = "Full Squads Hunt The Movement Player...";
    char        arstr[]   = {0x27, 0xd8, 0xa8, 0xd9, 0x90, 0xd8, 0xb3, 0xd9, 0x92, 0xd9, 0x85, 0xd9, 0x90, 0x20, 0xd9, 0xb1, 0xd9, 0x84, 0xd9, 0x84, 0xd9, 0x8e, 0xd9, 0x91, 0xd9, 0xb0, 0xd9, 0x87, 0xd9, 0x90, 0x20, 0xd9, 0xb1, 0xd9, 0x84, 0xd8, 0xb1, 0xd9, 0x8e, 0xd9, 0x91, 0xd8, 0xad, 0xd9, 0x92, 0xd9, 0x85, 0xd9, 0x8e, 0xd9, 0xb0, 0xd9, 0x86, 0xd9, 0x90, 0x20, 0xd9, 0xb1, 0xd9, 0x84, 0xd8, 0xb1, 0xd9, 0x8e, 0xd9, 0x91, 0xd8, 0xad, 0xd9, 0x90, 0xd9, 0x8a, 0xd9, 0x85, 0xd9, 0x90};
    char        jpstr[]   = {0xe4, 0xba, 0xba, 0xe7, 0x9a, 0x86, 0xe7, 0x94, 0x9f, 0xe8, 0x80, 0x8c};
    char const  iconstr[] = "verified";

    TextRun runs[] = {
        {.text = {iconstr, strlen(iconstr)}, .props = stx::Some(TextProps{.font = "MaterialIcons", .font_height = 26, .foreground_color = color::from_rgb(29, 155, 240)})},
        {.text = {str, strlen(str)}},
        {.text = arstr, .props = stx::Some(TextProps{.font = "Arabic", .font_height = 26, .foreground_color = colors::GREEN, .stroke_color = colors::BLACK.with_alpha(200), .letter_spacing = 0, .word_spacing = 10, .line_height = 2.0f, .direction = TextDirection::RightToLeft, .script = Script::Arabic, .language = languages::ARABIC})},
        {.text = jpstr, .props = stx::Some(TextProps{.font = "JP", .font_height = 26, .foreground_color = colors::YELLOW, .stroke_color = colors::BLACK.with_alpha(200), .word_spacing = 10, .line_height = 2.0f, .direction = TextDirection::LeftToRight, .script = Script::Hiragana, .language = languages::JAPANESE})}};

    Paragraph p{
        .runs  = runs,
        .props = TextProps{.font = "Roboto", .font_height = 26, .foreground_color = colors::WHITE, .background_color = colors::BLACK, .stroke_color = colors::BLACK.with_alpha(200)}};

    TextLayout layout;
    layout.layout(p, context.font_bundle, area.extent.x);

    canvas.shear_x(0.25, 0.25).draw_text(p, layout, context.font_bundle, area.offset + vec2{20, 20});
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

  virtual void on_mouse_enter(Context &context, vec2 screen_position, quad quad) override
  {
    spdlog::info("mouse over");
  }

  virtual void on_mouse_leave(Context &context, stx::Option<vec2> screen_position)
  {
    spdlog::info("mouse leave");
  }

  constexpr virtual void on_click(Context &context, MouseButton button, vec2 screen_position, u32 nclicks, quad quad)
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

  virtual simdjson::dom::element save(Context &context, simdjson::dom::parser &parser) override
  {
    std::string      source;
    std::string_view source_type;
    ash::extent      extent;

    // T0D0(lamarrr): format
    if (std::holds_alternative<ImageBuffer>(props.source))
    {
      ImageBuffer &src = std::get<ImageBuffer>(props.source);
      source           = base64_encode(src.span().as_char());
      source_type      = "memory";
      extent           = src.extent;
    }
    else if (std::holds_alternative<FileImageSource>(props.source))
    {
      source      = std::get<FileImageSource>(props.source).path.c_str();
      source_type = "file";
    }
    else if (std::holds_alternative<NetworkImageSource>(props.source))
    {
      source      = std::get<NetworkImageSource>(props.source).uri.c_str();
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
        std::string_view{props.alt.c_str()});

    return parser.parse(json.data(), json.size());
  }

  virtual void restore(Context &context, simdjson::dom::element const &element) override
  {
    if (image_load_future.is_some())
    {
      image_load_future.value().request_cancel();
      image_load_future = stx::None;
    }

    if (state == ImageState::Loaded)
    {
      ImageManager *mgr = context.get_plugin<ImageManager>("ImageManager").unwrap();
      mgr->remove(image);
    }

    std::string_view source      = element["source"].get_string();
    std::string_view source_type = element["source_type"].get_string();
    ash::extent      extent{.width = AS(u32, element["extent_width"].get_uint64()), .height = AS(u32, element["extent_height"].get_uint64())};

    if (source_type == "memory")
    {
      // TODO(lamarrr): fix up
      std::string  enc = base64_decode(source);
      stx::Vec<u8> pixels;
      pixels.extend(stx::Span{enc}.as_u8()).unwrap();
      // props.source =
      //     MemoryImageSource{.buffer pixels = std::move(pixels), .extent = extent};
    }
    else if (source_type == "file")
    {
      props.source = FileImageSource{.path = stx::string::make(stx::os_allocator, source).unwrap()};
    }
    else if (source_type == "network")
    {
      props.source = NetworkImageSource{.uri = stx::string::make(stx::os_allocator, source).unwrap()};
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
    props.alt            = stx::string::make(stx::os_allocator, element["alt"].get_string()).unwrap();
    state                = ImageState::Inactive;
  }

  ImageProps                                                         props;
  ImageState                                                         state = ImageState::Inactive;
  gfx::image                                                         image = 0;
  extent                                                             image_extent;
  stx::Option<stx::Future<stx::Result<ImageBuffer, ImageLoadError>>> image_load_future;
};

}        // namespace ash
