#pragma once

#include <cinttypes>

#include "asura/primitives.h"
#include "stx/string.h"
#include "stx/vec.h"

// See: https://html.spec.whatwg.org/multipage/canvas.html

// TODO(lamarrr): we'll acrtually generate 3D vertices with them so they can
// play well with 3D animations

namespace asr {

// TODO(lamarrr): this should be a GPU texture/image
struct Image {};

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
  f32 em_height_ascent = 0.0f;
  f32 em_height_descent = 0.0f;
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
  bool image_smoothing_enabled = true;
  ImageSmoothingQuality image_smoothing_quality = ImageSmoothingQuality::Medium;
};

struct Shadow {
  f32 offset_x = 0.0f;
  f32 offset_y = 0.0f;
  f32 blur_radius = 0.0f;
  Color color = colors::BLACK;
};

struct Filter { /* None by default */
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

struct Typeface {};

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

struct Shader {};

namespace transforms {
constexpr mat4x4 translation(vec3 t) {
  return mat4x4{
      vec4{1.0f, 0.0f, 0.0f, t.x},
      vec4{0.0f, 1.0f, 0.0f, t.y},
      vec4{0.0f, 0.0f, 1.0f, t.z},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

constexpr mat4x4 scaling(vec3 s) {
  return mat4x4{
      vec4{s.x, 0.0f, 0.0f, 0.0f},
      vec4{0.0f, s.y, 0.0f, 0.0f},
      vec4{0.0f, 0.0f, s.z, 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

constexpr mat4x4 rotation(vec3 r) {
  return mat4x4{
      vec4{1.0f, 0.0f, 0.0f, 0.0f},
      vec4{1.0f, 0.0f, 0.0f, 0.0f},
      vec4{1.0f, 0.0f, 0.0f, 0.0f},
      vec4{1.0f, 0.0f, 0.0f, 0.0f},
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
  stx::Vec<u64> indices{stx::os_allocator};
  stx::Vec<DrawCommand> commands{stx::os_allocator};
};

// struct CanvasState {
//   Transform transform;
// clips
// };

enum class BuiltinShaderID : u32 { FRAG_DIRECT, VERT_COLORED };

struct BuiltinShaderPack {
  virtual stx::Rc<Shader *> get(BuiltinShaderID);
};

// TODO(lamarrr): Builtin ShaderManager and TextureManager that is preloaded at
// runtime and we just use enums to find which we need

// clipping, filling, stroking
struct Path {
  stx::Vec<vec3> points{stx::os_allocator};
};

// TODO(lamarrr): how do we handle selection of transformed widgets?

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
  void save() { saved_transform_states.push(mat4x4{transform}).unwrap(); }

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

  void translate(f32 x, f32 y, f32 z = 0.0f) {
    transform = transforms::rotation(vec3{x, y, z}) * transform;
  }

  void rotate(f32 degree);

  void scale(f32 x, f32 y, f32 z = 1.0f) {
    transform = transforms::scaling(vec3{x, y, z}) * transform;
  }

  void clear() {
    u64 start = draw_list.vertices.size();

    draw_list.vertices.push(vec3{0.0f, 0.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{extent.w, 0.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{extent.w, extent.h, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{0.0f, extent.h, 0.0f}).unwrap();

    draw_list.indices.push_inplace(start).unwrap();
    draw_list.indices.push_inplace(start + 1).unwrap();
    draw_list.indices.push_inplace(start + 2).unwrap();
    draw_list.indices.push_inplace(start + 2).unwrap();
    draw_list.indices.push_inplace(start).unwrap();
    draw_list.indices.push_inplace(start + 3).unwrap();

    draw_list.commands
        .push(DrawCommand{.indices_offset = start,
                          .num_triangles = 2,
                          .transform = mat4x4::identity(),
                          .color = brush.color,
                          .texture = stx::None,
                          .vert_shader = shader_pack.handle->get(
                              BuiltinShaderID::VERT_COLORED),
                          .frag_shader = shader_pack.handle->get(
                              BuiltinShaderID::FRAG_DIRECT)})
        .unwrap();
  }

  // HELPERS
  void clip_rect();
  void clip_rrect();
  void clip_slanted_rect();

  // TEXT API
  void text(stx::String text, OffsetF position);

  // IMAGE API
  void draw_image(stx::Rc<Image *> image, OffsetF position);
  void draw_image(stx::Rc<Image *> image, RectF target);
  void draw_image(stx::Rc<Image *> image, RectF portion, RectF target);

  // SHARED PATH API
  void close_path();
  void move_to(OffsetF point) {
    position = vec3{.x = point.x, .y = point.y, .z = position.z};
  }
  void line_to(OffsetF point);
  void quadratic_curve_to(f32 cpx, f32 cpy, f32 x, f32 y);
  void bezier_curve_to(f32 cp1x, f32 cp1y, f32 cp2x, f32 cp2y, f32 x, f32 y);
  void arc_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 radius);

  // PRIMITIVES
  void rect(RectF area) {
    u64 start = draw_list.vertices.size();

    // we only need to set these from 0 to 1 and then apply the transform to
    // scale it?


    draw_list.vertices.push(vec3{0.0f, 0.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{1.0f, 0.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{1.0f, 1.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{0.0f, 1.0f, 0.0f}).unwrap();

    draw_list.vertices.push(vec3{area.offset.x, area.offset.y, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{area.offset.x + extent.w, area.offset.y, 0.0f})
        .unwrap();
    draw_list.vertices
        .push(vec3{area.offset.x + extent.w, area.offset.y + extent.h, 0.0f})
        .unwrap();
    draw_list.vertices.push(vec3{area.offset.x, area.offset.y + extent.h, 0.0f})
        .unwrap();

    draw_list.indices.push_inplace(start).unwrap();
    draw_list.indices.push_inplace(start + 1).unwrap();
    draw_list.indices.push_inplace(start + 2).unwrap();
    draw_list.indices.push_inplace(start + 2).unwrap();
    draw_list.indices.push_inplace(start).unwrap();
    draw_list.indices.push_inplace(start + 3).unwrap();

    brush.pattern;  // TODO(lamarrr): what about textured backgrounds?
    draw_list.commands
        .push(DrawCommand{
            .indices_offset = start,
            .num_triangles = 2,
            .transform =  // transforms::translation(position) * transform,
                .color = brush.color,
            .texture = stx::None,
            .vert_shader =
                shader_pack.handle->get(BuiltinShaderID::VERT_COLORED),
            .frag_shader =
                shader_pack.handle->get(BuiltinShaderID::FRAG_DIRECT)})
        .unwrap();
  }

  void round_rect(RectF area, vec4 radii);
  void slanted_rect(RectF area);
  void arc(OffsetF center, f32 radius, f32 start_angle, f32 end_angle);
  void circle(OffsetF center, f32 radius);
  void ellipse(OffsetF center, f32 rx, f32 ry, f32 rotation, f32 start_angle,
               f32 end_angle);
};
};  // namespace asr
