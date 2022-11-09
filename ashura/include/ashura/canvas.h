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

// See: https://html.spec.whatwg.org/multipage/canvas.html

// TODO(lamarrr): we'll acrtually generate 3D vertices with them so they can
// play well with 3D animations

namespace asr {

// TODO(lamarrr): this should be a GPU texture/image
// struct Image {};

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

// the type of endings that UAs will place on the end of lines
enum class LineCap : u8 { Butt, Round, Square };

// the type of corners that UAs will place where two lines meet
enum class LineJoin : u8 { Round, Bevel, Miter };

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
  LineCap line_cap = LineCap::Butt;
  LineJoin line_join = LineJoin::Miter;
  f32 miter_limit = 10.0f;
};

// struct Typeface {};

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
  stx::Option<stx::Rc<Image *>> pattern;
  PathStyle path_style;
  TextStyle text_style;
  Filter filter;
  Shadow shadow;
  ImageSmoothing smoothing;
  Compositing compositing;
};

// struct Shader {};

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
  u64 indices_offset = 0;
  u64 num_triangles = 0;
  mat4x4 transform;
  Color color = colors::BLACK;
  stx::Option<stx::Rc<Image *>> texture;
  stx::Rc<Shader *> vert_shader;
  stx::Rc<Shader *> frag_shader;  // - clip options will apply in the fragment
                                  // and vertex shaders
                                  // - blending
};

struct DrawList {
  stx::Vec<vec3> vertices{stx::os_allocator};
  stx::Vec<u32> indices{stx::os_allocator};
  stx::Vec<DrawCommand> commands{stx::os_allocator};
};

enum class BuiltinShaderID : u32 {
  FragColored,
  VertMvp,
  FragCircle,
  FragEllipse,
};

struct BuiltinShaderPack {
  virtual stx::Rc<Shader *> get(BuiltinShaderID);
};

// TODO(lamarrr): Builtin ShaderManager and TextureManager that is preloaded at
// runtime and we just use enums to find which we need

// clipping, filling, stroking
struct Path {
  vec3 position;
  stx::Vec<vec3> points{stx::os_allocator};

  void move_to(vec2 point) {
    position = vec3{.x = point.x, .y = point.y, .z = 0.0f};
  }

  void line_to(vec2 point) {
    points.push_inplace(position).unwrap();
    points.push(vec3{.x = point.x, .y = point.y, .z = 0.0f}).unwrap();
  }

  void circle(f32 radius) {}

  void close() {}
};

// TODO(lamarrr): how do we handle selection of transformed widgets?
// Topleft origin coordinate system
struct Canvas {
  vec3 position;
  ExtentF extent;
  Brush brush;
  mat4x4 transform = mat4x4::identity();
  stx::Vec<mat4x4> saved_transform_states{stx::os_allocator};
  stx::Vec<RectF> clips{stx::os_allocator};
  DrawList draw_list;
  stx::Rc<BuiltinShaderPack *> shader_pack;

  // shape???

  Canvas(stx::Rc<BuiltinShaderPack *> ashader_pack)
      : shader_pack{std::move(ashader_pack)} {}

  // rect clip, rrect clip
  // push state (transform and clips) on state stack
  void save() { saved_transform_states.push_inplace(transform).unwrap(); }

  // save current transform and clip state
  // pop state (transform and clips) stack and restore state
  void restore() {
    ASR_ENSURE(!saved_transform_states.is_empty());
    transform = *(saved_transform_states.end() - 1);
    saved_transform_states.resize(saved_transform_states.size() - 1).unwrap();
  }

  //
  // reset the rendering context to its default state (transform
  // and clips)
  void reset() { transform = mat4x4::identity(); }

  void translate(f32 x, f32 y) {
    transform = transforms::translate(vec3{x, y, 0.0f}) * transform;
  }

  void rotate(f32 degree) {
    transform = transforms::rotate_z(degree) * transform;
  }

  void scale(f32 x, f32 y) {
    transform = transforms::scale(vec3{x, y, 1.0f}) * transform;
  }

  // TODO(lamarrr): we can reuse this quad for others without needing to
  // allocate anything
  u64 __reserve_rect() {
    u64 start = draw_list.vertices.size();

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

  void clear() {
    u64 start = __reserve_rect();

    draw_list.commands
        .push(DrawCommand{
            .indices_offset = start,
            .num_triangles = 2,
            .transform = transforms::scale(vec3{extent.w, extent.h, 1.0f}),
            .color = brush.color,
            .texture = stx::None,
            .vert_shader = shader_pack.handle->get(BuiltinShaderID::VertMvp),
            .frag_shader =
                shader_pack.handle->get(BuiltinShaderID::FragColored)})
        .unwrap();
  }

  //   void clip_rect();
  //   void clip_rrect();
  //   void clip_slanted_rect();

  // TEXT API
  void text(stx::StringView text, OffsetF position);

  // IMAGE API
  void draw_image(stx::Rc<Image *> image, OffsetF position);
  void draw_image(stx::Rc<Image *> image, RectF target);
  void draw_image(stx::Rc<Image *> image, RectF portion, RectF target);

  void move_to(OffsetF point) {
    position = vec3{.x = point.x, .y = point.y, .z = position.z};
  }

  void line_to(OffsetF point) {
    position;
    point;

    u64 start = draw_list.vertices.size();
    u64 line_width = brush.path_style.line_width;
    vec3 p1 = position;
    vec3 p2 = vec3{point.x, point.y, position.z};

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
  }
  //   void quadratic_curve_to(f32 cpx, f32 cpy, f32 x, f32 y);
  //   void bezier_curve_to(f32 cp1x, f32 cp1y, f32 cp2x, f32 cp2y, f32 x, f32
  //   y);

  // PRIMITIVES
  void rect(RectF area) {
    // TODO(lamarrr): this will only work for filled rects
    u64 start = __reserve_rect();

    mat4x4 transforms =
        transform *
        (transforms::translate(vec3{area.offset.x, area.offset.y, 0.0f}) *
         transforms::scale(vec3{area.extent.w, area.extent.h, 1.0f}));

    brush.pattern;  // TODO(lamarrr): what about textured backgrounds?
    draw_list.commands
        .push(DrawCommand{
            .indices_offset = start,
            .num_triangles = 2,
            .transform = transforms,
            .color = brush.color,
            .texture = stx::None,
            .vert_shader = shader_pack.handle->get(BuiltinShaderID::VertMvp),
            .frag_shader =
                shader_pack.handle->get(BuiltinShaderID::FragColored)})
        .unwrap();
  }

  void round_rect(RectF area, vec4 radii);
  void slanted_rect(RectF area);
  void arc(OffsetF p1, OffsetF p2,
           f32 radius);  // within circle and within a rect that comtains that
                         // circle (for filled arc)
  void circle(OffsetF center, f32 radius) { u64 start = __reserve_rect(); }
  void ellipse(OffsetF center, ExtentF radius, f32 rotation, f32 start_angle,
               f32 end_angle) {
    __reserve_rect();
  }
};

void record(DrawList const &draw_list, stx::Rc<vkh::Device *> const &device,
            vkh::CommandQueueFamilyInfo const &graphics_command_queue,
            VkPhysicalDeviceMemoryProperties const &memory_properties) {
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

  for (DrawCommand const &draw_command : draw_list.commands) {
    vkCmdBindPipeline();
    vkCmdBindDescriptorSets();
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer.handle->buffer,
                           0);
    vkCmdBindIndexBuffer(command_buffer, index_buffer.handle->buffer, 0,
                         VK_INDEX_TYPE_UINT32);

    //
    vkCmdDrawIndexed(command_buffer, 0, 0, 0, 0, 0);
  }
}

};  // namespace asr
