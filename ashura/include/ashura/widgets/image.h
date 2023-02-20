#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "ashura/base64.h"
#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "stx/option.h"
#include "stx/span.h"
#include "stx/string.h"

namespace ash {

// RGBA image
struct MemoryImageSource {
  stx::Vec<u8> pixels{stx::os_allocator};
  extent extent;
};

struct FileImageSource {
  stx::String path;
};

struct NetworkImageSource {
  stx::String uri;
};

using ImageSource =
    std::variant<MemoryImageSource, FileImageSource, NetworkImageSource>;

struct ImageProps {
  ImageSource source;
  constraint width;
  constraint height;
  vec4 border_radius;
  stx::Option<f32> aspect_ratio;
  bool resize_on_load = true;
  stx::String alt;
};

enum class ImageState : u8 {
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
struct Image : public Widget {
  explicit Image(ImageProps image_props) : props{std::move(image_props)} {}

  virtual WidgetInfo get_info() override {
    return WidgetInfo{.type = "Image", .id = Widget::id};
  }

  virtual Layout layout(rect area) override {
    f32 width = props.width.resolve(area.extent.x);
    f32 height = props.height.resolve(area.extent.y);
    if (props.aspect_ratio.is_some()) {
      f32 aspect_ratio = props.aspect_ratio.value();
      return Layout{
          .area = rect{.offset = vec2{0, 0},
                       .extent = vec2{std::min(height * aspect_ratio, width),
                                      std::min(width / aspect_ratio, height)}}};
    } else {
      return Layout{
          .area = rect{.offset = area.offset, .extent = vec2{width, height}}};
    }
  }

  virtual void draw(gfx::Canvas& canvas, rect area) override {
    switch (state) {
      case ImageState::Inactive: {
      } break;
      case ImageState::Loading: {
      } break;
      case ImageState::Loaded: {
        canvas.save();
        canvas.brush.fill = true;
        canvas.brush.texture = image;
        if (props.border_radius == vec4{0, 0, 0, 0}) {
          canvas.draw_rect(area);
        } else {
          canvas.draw_round_rect(area, props.border_radius, 360);
        }
        canvas.restore();
        // draw alt text?
      } break;
      case ImageState::LoadFailed: {
      } break;
      default: {
      } break;
    }
  }

  virtual void tick(WidgetContext& context,
                    std::chrono::nanoseconds interval) override {}

  virtual void on_hover(KeyModifiers modifiers, vec2 position) override {}

  virtual simdjson::dom::element save(simdjson::dom::parser& parser) override {
    std::string source;
    std::string_view source_type;
    extent extent;

    if (std::holds_alternative<MemoryImageSource>(props.source)) {
      MemoryImageSource& memory_source =
          std::get<MemoryImageSource>(props.source);
      stx::Span pixels = memory_source.pixels.span();
      source = base64_encode(stx::Span{
          reinterpret_cast<char const*>(pixels.data()), pixels.size()});
      source_type = "memory";
      extent = memory_source.extent;
    } else if (std::holds_alternative<FileImageSource>(props.source)) {
      source = std::get<FileImageSource>(props.source).path;
      source_type = "file";
    } else if (std::holds_alternative<NetworkImageSource>(props.source)) {
      source = std::get<NetworkImageSource>(props.source).uri;
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
        props.width.scale, props.width.min, props.width.max,
        props.width.min_rel, props.width.max_rel, props.height.bias,
        props.height.scale, props.height.min, props.height.max,
        props.height.min_rel, props.height.max_rel, props.border_radius.x,
        props.border_radius.y, props.border_radius.z, props.border_radius.w,
        props.aspect_ratio.is_some(), props.aspect_ratio.copy().unwrap_or(1.0f),
        props.resize_on_load, std::string_view{props.alt});

    return parser.parse(json.data(), json.size());
  }

  virtual void restore(simdjson::dom::element const& element) override {
    std::string_view source = element["source"].get_string();
    std::string_view source_type = element["source_type"].get_string();
    extent extent{.width = AS_U32(element["extent_width"].get_uint64()),
                  .height = AS_U32(element["extent_height"].get_uint64())};

    if (source_type == "memory") {
      std::string enc = base64_decode(source);
      stx::Vec<u8> pixels{stx::os_allocator};
      pixels.extend(stx::Span{enc}.as_u8()).unwrap();
      props.source =
          MemoryImageSource{.pixels = std::move(pixels), .extent = extent};
    } else if (source_type == "file") {
      props.source = FileImageSource{
          .path = stx::string::make(stx::os_allocator, source).unwrap()};
    } else if (source_type == "network") {
      props.source = NetworkImageSource{
          .uri = stx::string::make(stx::os_allocator, source).unwrap()};
    }

    props.width.bias = AS_F32(element["width_bias"].get_double());
    props.width.scale = AS_F32(element["width_scale"].get_double());
    props.width.min = AS_F32(element["width_min"].get_double());
    props.width.max = AS_F32(element["width_max"].get_double());
    props.width.min_rel = AS_F32(element["width_min_rel"].get_double());
    props.width.max_rel = AS_F32(element["width_max_rel"].get_double());
    props.height.bias = AS_F32(element["height_bias"].get_double());
    props.height.scale = AS_F32(element["height_scale"].get_double());
    props.height.min = AS_F32(element["height_min"].get_double());
    props.height.max = AS_F32(element["height_max"].get_double());
    props.height.min_rel = AS_F32(element["height_min_rel"].get_double());
    props.height.max_rel = AS_F32(element["height_max_rel"].get_double());
    props.border_radius.x = AS_F32(element["border_radius_x"].get_double());
    props.border_radius.y = AS_F32(element["border_radius_y"].get_double());
    props.border_radius.z = AS_F32(element["border_radius_z"].get_double());
    props.border_radius.w = AS_F32(element["border_radius_w"].get_double());

    if (element["has_aspect_ratio"].get_bool()) {
      props.aspect_ratio =
          stx::Some(AS_F32(element["aspect_ratio"].get_double()));
    } else {
      props.aspect_ratio = stx::None;
    }

    props.resize_on_load = element["resize_on_load"].get_bool();
    props.alt =
        stx::string::make(stx::os_allocator, element["alt"].get_string())
            .unwrap();
  }

  ImageProps props;
  ImageState state = ImageState::Inactive;
  gfx::image image = 0;
};

}  // namespace ash
