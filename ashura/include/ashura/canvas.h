#pragma once

#include <cinttypes>
#include <cmath>
#include <filesystem>
#include <fstream>

#include "ashura/primitives.h"
#include "ashura/vulkan.h"
#include "stx/string.h"
#include "stx/vec.h"
#include "vulkan/vulkan.h"

// TODO(lamarrr): we'll acrtually generate 3D vertices with them so they can
// play well with 3D graphics and animations

namespace asr {
namespace gfx {

// TODO(lamarrr): child must inherit parent's transformation and opacity

struct TextMetrics {
  // x-direction
  f32 width = 0.0f;
  f32 actual_bounding_box_left = 0.0f;
  f32 actual_bounding_box_right = 0.0f;

  // y-direction
  f32 font_bounding_box_ascent = 0.0f;
  f32 font_bounding_box_descent = 0.0f;
  f32 actual_bounding_box_ascent = 0.0f;
  f32 actual_bounding_box_descent = 0.0f;
  f32 ascent = 0.0f;
  f32 descent = 0.0f;
  f32 hanging_baseline = 0.0f;
  f32 alphabetic_baseline = 0.0f;
  f32 ideographic_baseline = 0.0f;
};

enum class ImageSmoothingQuality : u8 { Low, Medium, High };

enum class BlendMode : u8 { SrcOver, Src };

struct Compositing {
  f32 global_alpha = 1.0f;
  BlendMode blend_mode = BlendMode::SrcOver;
};

struct ImageSmoothing {
  bool enabled = true;
  ImageSmoothingQuality quality = ImageSmoothingQuality::Medium;
};

struct Shadow {
  f32 offset_x = 0.0f;
  f32 offset_y = 0.0f;
  f32 blur_radius = 0.0f;
  Color color = colors::BLACK;
};

struct Filter {
  /* None by default */
};

enum class TextAlign : u8 {
  Start /* detect locale and other crap */,
  End,
  Left,
  Right,
  Center
};

enum class TextBaseline : u8 {
  Top,
  Hanging,
  Middle,
  Alphabetic,
  Ideographic,
  Bottom
};

enum class TextDirection : u8 { Ltr, Rtl, Ttb, Btt };

enum class FontKerning : u8 { Normal, None };

enum class FontStretch : u8 {
  UltraCondensed,
  ExtraCondensed,
  Condensed,
  SemiCondensed,
  Normal,
  SemiExpanded,
  Expanded,
  ExtraExpanded,
  UltraExpanded
};

struct PathStyle {
  f32 line_width = 1.0f;
};

enum class FontWeight : u32 {
  Thin = 100,
  ExtraLight = 200,
  Light = 300,
  Normal = 400,
  Medium = 500,
  Semi = 600,
  Bold = 700,
  ExtraBold = 800,
  Black = 900,
  ExtraBlack = 950
};

// requirements:
// - easy resource specification
// - caching so we don't have to reupload on every frame
// - GPU texture/image
// NOTE: canvas is low-level so we don't use paths, urls, or uris
struct Image {};

// requirements:
// -
struct Typeface {};

// requirements:
struct Shader {};

// TODO(lamarrr): embed font into a cpp file
//
// on font loading
//
struct TextStyle {
  stx::String font_family = stx::string::make_static("SF Pro");
  FontWeight font_weight = FontWeight::Normal;
  u32 font_size = 10;
  TextAlign align = TextAlign::Start;
  TextBaseline baseline = TextBaseline::Alphabetic;
  TextDirection direction = TextDirection::Ltr;
  u32 letter_spacing = 0;
  FontKerning font_kerning = FontKerning::None;
  FontStretch font_stretch = FontStretch::Normal;
  u32 word_spacing = 0;
};

struct Brush {
  bool fill = true;
  Color color = colors::BLACK;
  stx::Option<Image> pattern;
  PathStyle path_style;
  TextStyle text_style;
  Filter filter;
  Shadow shadow;
  ImageSmoothing smoothing;
  Compositing compositing;
};

// TODO(lamarrr): invert these rows and columns
namespace transforms {
constexpr mat4x4 translate(vec3 t) {
  return mat4x4{
      vec4{1.0f, 0.0f, 0.0f, t.x},
      vec4{0.0f, 1.0f, 0.0f, t.y},
      vec4{0.0f, 0.0f, 1.0f, t.z},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

constexpr mat4x4 scale(vec3 s) {
  return mat4x4{
      vec4{s.x, 0.0f, 0.0f, 0.0f},
      vec4{0.0f, s.y, 0.0f, 0.0f},
      vec4{0.0f, 0.0f, s.z, 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

constexpr mat4x4 rotate_x(f32 degree) {
  return mat4x4{
      vec4{1.0f, 0.0f, 0.0f, 0.0f},
      vec4{0.0f, std::cos(degree), -std::sin(degree), 0.0f},
      vec4{0.0f, std::sin(degree), std::cos(degree), 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

constexpr mat4x4 rotate_y(f32 degree) {
  return mat4x4{
      vec4{std::cos(degree), 0.0f, std::sin(degree), 0.0f},
      vec4{0.0f, 1.0f, 0.0f, 0.0f},
      vec4{-std::sin(degree), 0, std::cos(degree), 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

constexpr mat4x4 rotate_z(f32 degree) {
  return mat4x4{
      vec4{std::cos(degree), -std::sin(degree), 0.0f, 0.0f},
      vec4{std::sin(degree), std::cos(degree), 0.0f, 0.0f},
      vec4{0.0f, 0.0f, 1.0f, 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

};  // namespace transforms

// TODO(lamarrr): what about positioning?
struct DrawCommand {
  u32 indices_offset = 0;
  u32 num_triangles = 0;
  vec3 opacity;
  mat4x4 transform;  //  transform contains position (translation from origin)
  Color color = colors::BLACK;
  stx::Option<Image> texture;
  Shader vert_shader;
  Shader frag_shader;  // - clip options will apply in the fragment
                       // and vertex shaders
                       // - blending
};

struct DrawList {
  stx::Vec<vec3> vertices{stx::os_allocator};
  stx::Vec<u32> indices{stx::os_allocator};
  stx::Vec<DrawCommand> commands{stx::os_allocator};
};

// TODO(lamarrr): how do we handle selection of transformed widgets?
// Topleft origin coordinate system
struct Canvas {
  vec3 position;
  ExtentF extent;
  Brush brush;
  mat4x4 transform = mat4x4::identity();
  stx::Vec<mat4x4> transform_state_stack{stx::os_allocator};
  DrawList draw_list;
  //   stx::Vec<RectF> clips{stx::os_allocator};

  // push state (transform and clips) on state stack
  Canvas& save() {
    transform_state_stack.push_inplace(transform).unwrap();
    return *this;
  }

  // save current transform and clip state
  // pop state (transform and clips) stack and restore state
  Canvas& restore() {
    ASR_ENSURE(!transform_state_stack.is_empty());
    transform = *(transform_state_stack.end() - 1);
    transform_state_stack.resize(transform_state_stack.size() - 1).unwrap();

    return *this;
  }

  // reset the rendering context to its default state (transform
  // and clips)
  Canvas& reset() {
    transform = mat4x4::identity();
    return *this;
  }

  Canvas& translate(f32 x, f32 y, f32 z) {
    transform = transforms::translate(vec3{x, y, z}) * transform;
    return *this;
  }

  Canvas& translate(f32 x, f32 y) { return translate(x, y, 0.0f); }

  Canvas& rotate(f32 degree) {
    transform = transforms::rotate_z(degree) * transform;
    return *this;
  }

  Canvas& rotate(f32 x, f32 y, f32 z) {
    transform = transforms::rotate_z(z) * transforms::rotate_y(y) *
                transforms::rotate_x(x) * transform;
    return *this;
  }

  Canvas& scale(f32 x, f32 y, f32 z) {
    transform = transforms::scale(vec3{x, y, z}) * transform;
    return *this;
  }

  Canvas& scale(f32 x, f32 y) { return scale(x, y, 1.0f); }

  Canvas& move_to(f32 x, f32 y, f32 z) {
    position = vec3{x, y, z};
    return *this;
  }

  Canvas& move_to(f32 x, f32 y) { return move_to(x, y, position.z); }

  // TODO(lamarrr): we can reuse this for others without needing to
  // allocate anything
  u32 __reserve_rect() {
    u32 start = draw_list.vertices.size();

    draw_list.vertices.push(vec3{0.0f, 0.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{1.0f, 0.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{1.0f, 1.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{0.0f, 1.0f, 0.0f}).unwrap();

    draw_list.indices.push_inplace(start).unwrap();
    draw_list.indices.push_inplace(start + 1).unwrap();
    draw_list.indices.push_inplace(start + 2).unwrap();
    draw_list.indices.push_inplace(start + 2).unwrap();
    draw_list.indices.push_inplace(start).unwrap();
    draw_list.indices.push_inplace(start + 3).unwrap();

    return start;
  }

  Canvas& clear() {
    u32 start = __reserve_rect();

    draw_list.commands
        .push(DrawCommand{
            .indices_offset = start,
            .num_triangles = 2,
            .transform = transforms::scale(vec3{extent.w, extent.h, 1.0f}),
            .color = brush.color,
            .texture = stx::None,
            .vert_shader = {},
            .frag_shader = {}})
        .unwrap();

    return *this;
  }

  // clip_rect(), clip_rrect(), clip_slanted_rect();

  Canvas& draw_line_to(OffsetF point) {
    position;
    point;

    u32 start = draw_list.vertices.size();
    u64 line_width = brush.path_style.line_width;
    vec3 p1 = position;
    vec3 p2{point.x, point.y, position.z};

    draw_list.vertices.push(vec3{0.0f, 0.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{1.0f, 0.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{1.0f, 1.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{0.0f, 1.0f, 0.0f}).unwrap();

    draw_list.indices.push_inplace(start).unwrap();
    draw_list.indices.push_inplace(start + 1).unwrap();
    draw_list.indices.push_inplace(start + 2).unwrap();
    draw_list.indices.push_inplace(start + 2).unwrap();
    draw_list.indices.push_inplace(start).unwrap();
    draw_list.indices.push_inplace(start + 3).unwrap();

    return *this;
  }

  Canvas& draw_rect(RectF area) {
    // TODO(lamarrr): this will only work for filled rects
    u32 start = __reserve_rect();

    mat4x4 transforms =
        transform *
        (transforms::translate(vec3{area.offset.x, area.offset.y, 0.0f}) *
         transforms::scale(vec3{area.extent.w, area.extent.h, 1.0f}));

    brush.pattern;  // TODO(lamarrr): what about textured backgrounds?
    draw_list.commands
        .push(DrawCommand{.indices_offset = start,
                          .num_triangles = 2,
                          .transform = transforms,
                          .color = brush.color,
                          .texture = stx::None,
                          .vert_shader = {},
                          .frag_shader = {}})
        .unwrap();

    return *this;
  }

  Canvas& draw_round_rect(RectF area, vec4 radii);
  Canvas& draw_slanted_rect(RectF area);
  // within circle and within a rect that comtains
  // that circle (for filled arc)
  Canvas& draw_arc(OffsetF p1, OffsetF p2, f32 radius);
  Canvas& draw_circle(OffsetF center, f32 radius);
  Canvas& draw_ellipse(OffsetF center, ExtentF radius, f32 rotation,
                       f32 start_angle, f32 end_angle);

  // Text API
  Canvas& draw_text(stx::StringView text, OffsetF position);

  // Image API
  Canvas& draw_image(Image image, OffsetF position);
  Canvas& draw_image(Image image, RectF target);
  Canvas& draw_image(Image image, RectF portion, RectF target);
};

void sample(Canvas& canvas) {
  canvas.save()
      .rotate(45)
      .draw_circle({0, 0}, 20.0f)
      .draw_image(Image{}, {{0.0, 0.0}, {20, 40}})
      .restore()
      .move_to(0, 0)
      .scale(2.0f, 2.0f)
      .draw_line_to({200, 200})
      .draw_text("Hello World, こんにちは世界", {10.0f, 10.0f})
      .draw_rect({0, 0, 20, 20})
      .draw_round_rect({{0.0f, 0.0f}, {20.0f, 20.0f}},
                       {10.0f, 10.0f, 10.0f, 10.0f});
}

void record(DrawList const& draw_list, stx::Rc<vk::Device*> const& device,
            vk::CommandQueueFamilyInfo const& graphics_command_queue,
            VkPhysicalDeviceMemoryProperties const& memory_properties) {
  VkCommandBuffer command_buffer;

  auto vertex_buffer = upload_vertices(device, graphics_command_queue,
                                       memory_properties, draw_list.vertices);
  auto index_buffer = upload_indices(device, graphics_command_queue,
                                     memory_properties, draw_list.indices);
  // texture uploads
  // buffer uploads
  // - upload vertex buffer
  // - upload index buffer
  // - upload textures

  // we need to retain texture uploads, we can't be reuploading them every time

  for (DrawCommand const& draw_command : draw_list.commands) {
    // vkCmdBindPipeline();
    // vkCmdBindDescriptorSets();
    // vkCmdBindVertexBuffers(command_buffer, 0, 1,
    // &vertex_buffer.handle->buffer,
    //    0);
    // vkCmdBindIndexBuffer(command_buffer,
    // index_buffer.handle->buffer, 0,
    //  VK_INDEX_TYPE_UINT32);
    //
    //
    // vkCmdDrawIndexed(command_buffer, 0, 0, 0, 0, 0);
    // vkCmdSetScissor();
    // vkCmdSetViewport();
  }
}

}  // namespace gfx
};  // namespace asr
