#pragma once

#include <array>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <iostream>

#include "ashura/primitives.h"
#include "ashura/shaders.h"
#include "ashura/vulkan.h"
#include "stx/string.h"
#include "stx/vec.h"
#include "vulkan/vulkan.h"

namespace asr {
using namespace stx::literals;

struct vertex {
  vec2 position;
  vec2 st;

  static constexpr std::array<VkVertexInputAttributeDescription, 2>
  attribute_descriptions() {
    return {
        VkVertexInputAttributeDescription{.location = 0,
                                          .binding = 0,
                                          .format = VK_FORMAT_R32G32_SFLOAT,
                                          .offset = offsetof(vertex, position)},
        VkVertexInputAttributeDescription{.location = 1,
                                          .binding = 0,
                                          .format = VK_FORMAT_R32G32_SFLOAT,
                                          .offset = offsetof(vertex, st)}};
  }

  static constexpr VkVertexInputBindingDescription binding_description() {
    return {.binding = 0,
            .stride = sizeof(vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
  }
};

namespace gfx {

inline void triangulate_convex_polygon(stx::Vec<u32>& indices, u32 first_index,
                                       stx::Span<vec2 const> polygon) {
  ASR_CHECK(polygon.size() >= 3, "polygon must have 3 or more points");

  for (u32 i = 2; i < polygon.size(); i++) {
    indices.push_inplace(first_index).unwrap();
    indices.push_inplace(first_index + (i - 1)).unwrap();
    indices.push_inplace(first_index + i).unwrap();
  }
}

namespace polygons {

inline void rect(stx::Span<vec2> polygon, vec2 offset, vec2 extent) {
  polygon[0] = offset;
  polygon[1] = {offset.x + extent.x, offset.y};
  polygon[2] = offset + extent;
  polygon[3] = {offset.x, offset.y + extent.y};
}

inline void circle(stx::Span<vec2> polygon, vec2 center, f32 radius,
                   usize nsegments) {
  if (nsegments == 0 || radius <= 0.0f) return;

  f32 step = AS_F32((2 * M_PI) / nsegments);

  for (usize i = 0; i < nsegments; i++) {
    polygon[i] = vec2{center.x + radius - radius * std::cos(i * step),
                      center.y + radius - radius * std::sin(i * step)};
  }
}

inline void ellipse(stx::Span<vec2> polygon, vec2 center, vec2 radius,
                    usize nsegments) {
  if (nsegments == 0 || radius.x <= 0.0f || radius.y <= 0.0f) return;

  f32 step = AS_F32((2 * M_PI) / nsegments);

  for (usize i = 0; i < nsegments; i++) {
    polygon[i] = vec2{center.x + radius.x - radius.x * std::cos(i * step),
                      center.y + radius.y - radius.y * std::sin(i * step)};
  }
}

/// {polygon.size() == nsegments * 4}
inline void round_rect(stx::Span<vec2> polygon, vec2 offset, vec2 extent,
                       vec4 radii, usize nsegments) {
  if (nsegments == 0) return;

  radii.x = std::min(radii.x, std::min(extent.x, extent.y));
  radii.y = std::min(radii.y, std::min(extent.x, extent.y));
  radii.z = std::min(radii.z, std::min(extent.x, extent.y));
  radii.w = std::min(radii.w, std::min(extent.x, extent.y));

  f32 step = AS_F32((M_PI / 2) / nsegments);

  usize i = 0;

  vec2 xoffset{offset.x + radii.x, offset.y + radii.x};

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    polygon[i] = vec2{xoffset.y - radii.x * std::cos(segment * step),
                      xoffset.x - radii.x * std::sin(segment * step)};
  }

  vec2 yoffset{offset.x + (extent.x - radii.y), offset.y + radii.y};

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    polygon[i] = vec2{yoffset.y - radii.y * std::cos(90.0f + segment * step),
                      yoffset.x - radii.y * std::sin(90.0f + segment * step)};
  }

  vec2 zoffset{offset.x + (extent.x - radii.z),
               offset.y + (extent.y - radii.z)};

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    polygon[i] = vec2{zoffset.y - radii.z * std::cos(180.0f + segment * step),
                      zoffset.x - radii.z * std::sin(180.0f + segment * step)};
  }

  vec2 woffset{offset.x + radii.w, offset.y + (extent.y - radii.w)};

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    polygon[i] = vec2{woffset.y - radii.w * std::cos(270.0f + segment * step),
                      woffset.x - radii.w * std::sin(270.0f + segment * step)};
  }
}

};  // namespace polygons

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

struct Shadow {
  f32 offset_x = 0.0f;
  f32 offset_y = 0.0f;
  f32 blur_radius = 0.0f;
  Color color = colors::BLACK;
};

struct Filter {
  // None by default
};

enum class TextAlign : u8 {
  // detect locale and other crap
  Start,
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

enum class FontWeight : u32 {
  Thin = 100U,
  ExtraLight = 200U,
  Light = 300U,
  Normal = 400U,
  Medium = 500U,
  Semi = 600U,
  Bold = 700U,
  ExtraBold = 800U,
  Black = 900U,
  ExtraBlack = 950U
};

// requirements:
// - easy resource specification
// - caching so we don't have to reupload on every frame
// - GPU texture/image

// requirements:
// -
struct TypefaceId {
  u64 id = 0;
};

// stored in vulkan context
using Image = stx::Rc<vk::ImageSampler*>;

struct Typeface;

struct TypefaceRetainer;

// TODO(lamarrr): embed font into a cpp file
//
// on font loading
//
struct TextStyle {
  stx::String font_family = "SF Pro"_str;
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

namespace transforms {

inline mat4 translate(vec3 t) {
  return mat4{
      vec4{1.0f, 0.0f, 0.0f, t.x},
      vec4{0.0f, 1.0f, 0.0f, t.y},
      vec4{0.0f, 0.0f, 1.0f, t.z},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

inline mat4 scale(vec3 s) {
  return mat4{
      vec4{s.x, 0.0f, 0.0f, 0.0f},
      vec4{0.0f, s.y, 0.0f, 0.0f},
      vec4{0.0f, 0.0f, s.z, 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

inline mat4 rotate_x(f32 degree_radians) {
  return mat4{
      vec4{1.0f, 0.0f, 0.0f, 0.0f},
      vec4{0.0f, std::cos(degree_radians), -std::sin(degree_radians), 0.0f},
      vec4{0.0f, std::sin(degree_radians), std::cos(degree_radians), 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

inline mat4 rotate_y(f32 degree_radians) {
  return mat4{
      vec4{std::cos(degree_radians), 0.0f, std::sin(degree_radians), 0.0f},
      vec4{0.0f, 1.0f, 0.0f, 0.0f},
      vec4{-std::sin(degree_radians), 0, std::cos(degree_radians), 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

inline mat4 rotate_z(f32 degree_radians) {
  return mat4{
      vec4{std::cos(degree_radians), -std::sin(degree_radians), 0.0f, 0.0f},
      vec4{std::sin(degree_radians), std::cos(degree_radians), 0.0f, 0.0f},
      vec4{0.0f, 0.0f, 1.0f, 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

};  // namespace transforms

struct Brush {
  Color color = colors::BLACK;
  bool fill = true;
  f32 line_width = 1.0f;
  Image pattern;
  TextStyle text_style;
  Filter filter;
  Shadow shadow;
};

struct DrawCommand {
  u32 indices_offset = 0;
  u32 nindices = 0;
  rect clip_rect;
  mat4 transform = mat4::identity();
  Color color = colors::BLACK;
  Image texture;
};

struct DrawList {
  stx::Vec<vertex> vertices{stx::os_allocator};
  stx::Vec<u32> indices{stx::os_allocator};
  stx::Vec<DrawCommand> cmds{stx::os_allocator};

  void clear() {
    vertices.clear();
    indices.clear();
    cmds.clear();
  }
};

inline void transform_vertices_to_viewport(stx::Span<vertex> vertices,
                                           vec2 viewport_extent,
                                           rect polygon_area,
                                           rect texture_area) {
  for (usize i = 0; i < vertices.size(); i++) {
    vec2 position = vertices[i].position;

    // transform to -1 to +1 range with x pointing right and y pointing upwards
    vec2 normalized = (2 * position / viewport_extent) - 1;

    // positions are specified with x pointing right and y pointing downwards
    vec2 flipped = normalized * vec2{1, -1};

    // transform vertex position into texture coordinates (within the polygon's
    // extent)
    //
    //
    // TODO(lamarrr): texture coordinates have bottom-left origin
    //
    vec2 st = (position - polygon_area.offset) / polygon_area.extent;

    // map it into the portion of the texture we are interested in
    st = (texture_area.offset + st * texture_area.extent);

    vertices[i] = vertex{.position = flipped, .st = st};
  }
}

// TODO(lamarrr): properly handle the case of zero sized indices and clip
// indices
//
//
/// Coordinates are specified in top-left origin space with x pointing to the
/// right and y pointing downwards
///
struct Canvas {
  vec2 viewport_extent;
  Brush brush;

  mat4 transform = mat4::identity();
  stx::Vec<mat4> transform_state_stack{stx::os_allocator};

  rect clip_rect;
  stx::Vec<rect> clip_rect_stack{stx::os_allocator};

  Image transparent_image;

  DrawList draw_list;

  Canvas(vec2 viewport_extent, Image const& atransparent_image)
      : brush{.pattern = atransparent_image.share()},
        transparent_image{atransparent_image.share()} {
    restart(viewport_extent);
  }

  void restart(vec2 new_viewport_extent) {
    viewport_extent = new_viewport_extent;
    brush = Brush{.pattern = transparent_image.share()};
    transform = mat4::identity();
    transform_state_stack.clear();

    clip_rect = {{0, 0}, viewport_extent};

    clip_rect_stack.clear();
    draw_list.clear();
  }

  // push state (transform and clips) on state stack
  Canvas& save() {
    transform_state_stack.push_inplace(transform).unwrap();
    clip_rect_stack.push_inplace(clip_rect).unwrap();
    return *this;
  }

  // save current transform and clip state
  // pop state (transform and clips) stack and restore state
  Canvas& restore() {
    ASR_CHECK(!transform_state_stack.is_empty());
    ASR_CHECK(!clip_rect_stack.is_empty());

    transform = *(transform_state_stack.end() - 1);
    transform_state_stack.erase(transform_state_stack.span().slice(1));

    clip_rect = *(clip_rect_stack.end() - 1);
    clip_rect_stack.erase(clip_rect_stack.span().slice(1));

    return *this;
  }

  // reset the rendering context to its default state (transform
  // and clips)
  Canvas& reset() {
    transform = mat4::identity();
    transform_state_stack.clear();
    clip_rect = {{0, 0}, viewport_extent};
    clip_rect_stack.clear();
    return *this;
  }

  Canvas& translate(f32 x, f32 y, f32 z) {
    transform = transforms::translate(vec3{x, y, z}) * transform;
    return *this;
  }

  Canvas& translate(f32 x, f32 y) { return translate(x, y, 0.0f); }

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

  Canvas& clear() {
    restart(viewport_extent);

    vertex vertices[] = {{{0, 0}, {0, 1}},
                         {{viewport_extent.x, 0}, {1, 1}},
                         {viewport_extent, {1, 0}},
                         {{0, viewport_extent.y}, {0, 0}}};

    for (vertex& vertex : vertices) {
      vertex.position = (2 * vertex.position / viewport_extent) - 1;
      vertex.position = vertex.position * vec2{1, -1};
    }

    draw_list.vertices.extend(vertices).unwrap();

    u32 indices[] = {0, 1, 2, 0, 2, 3};

    draw_list.indices.extend(indices).unwrap();

    draw_list.cmds
        .push(DrawCommand{.indices_offset = 0,
                          .nindices = AS_U32(std::size(indices)),
                          .clip_rect = clip_rect,
                          .transform = mat4::identity(),
                          .color = brush.color,
                          .texture = brush.pattern.share()})
        .unwrap();

    return *this;
  }

  Canvas& draw_polygon_line(stx::Span<vec2 const> line) {
    /*
    ASR_CHECK(line.size() >= 2);

    u32 start = AS_U32(draw_list.indices.size());
    u32 nindices = 0;

    for (usize i = 0; i < line.size(); i++) {
      usize j = (i + 1 == line.size()) ? 0UL : (i + 1);

      vec2 p1 = line[i];
      vec2 p2 = line[j];

      vec2 d = p2 - p1;

      {
        f32 dot_product = dot(d, d);
        if (dot_product > 0.0f) {
          f32 inverse_length = 1 / std::sqrt(dot_product);
          d.x *= inverse_length;
          d.y *= inverse_length;
        }
      }

      d.x *= brush.line_width * 0.5f;
      d.y *= brush.line_width * 0.5f;

      vec2 vertices[] = {{p1.x + d.y, p1.y - d.x},
                         {p2.x + d.y, p2.y - d.x},
                         {p2.x - d.y, p2.y + d.x},
                         {p1.x - d.y, p1.y + d.x}};

      u32 indices[] = {start,     start + 1, start + 2,
                       start + 3, start + 4, start + 5};

      draw_list.vertices.extend(vertices).unwrap();
      draw_list.indices.extend(indices).unwrap();

      nindices += AS_U32(std::size(indices));
    }

    u32 nclip_polygon_vertices = AS_U32(draw_list.clip_vertices.size());

    // triangulate_polygon(draw_list.clip_vertices, clip);

    nclip_polygon_vertices =
        AS_U32(draw_list.clip_vertices.size()) - nclip_polygon_vertices;

    u32 clip_start = AS_U32(draw_list.clip_indices.size());

    for (u32 index = clip_start; index < (clip_start + nclip_polygon_vertices);
         index++) {
      draw_list.clip_indices.push_inplace(index).unwrap();
    }

    draw_list.cmds
        .push(DrawCommand{.indices_offset = start,
                          .nindices = nindices,
                          .clip_indices_offset = clip_start,
                          .nclip_indices = nclip_polygon_vertices,
                          .transform = transform,
                          .color = brush.color,
                          .texture = brush.pattern.share()})
        .unwrap();
        */
    return *this;
  }

  Canvas& draw_convex_polygon_filled(stx::Span<vec2 const> polygon, rect area,
                                     rect texture_area) {
    if (polygon.size() < 3 || area.extent.x == 0 || area.extent.y == 0 ||
        !clip_rect.overlaps(area))
      return *this;

    // TODO(lamarrr): somehow, the vertices are still transformed and the area
    // is cleared

    u32 start = AS_U32(draw_list.indices.size());

    u32 vertices_offset = AS_U32(draw_list.vertices.size());

    triangulate_convex_polygon(draw_list.indices, vertices_offset, polygon);

    u32 nindices = AS_U32(draw_list.indices.size() - start);

    for (usize i = 0; i < polygon.size(); i++) {
      draw_list.vertices.push(vertex{.position = polygon[i], .st = {0, 0}})
          .unwrap();
    }

    transform_vertices_to_viewport(
        draw_list.vertices.span().slice(vertices_offset), viewport_extent, area,
        texture_area);

    draw_list.cmds
        .push(DrawCommand{.indices_offset = start,
                          .nindices = nindices,
                          .clip_rect = clip_rect,
                          .transform = transform,
                          .color = brush.color,
                          .texture = brush.pattern.share()})
        .unwrap();

    return *this;
  }

  Canvas& draw_line(vec2 p1, vec2 p2) {
    /*
    vec2 d = p2 - p1;

  {
    f32 dot_product = dot(d, d);
    if (dot_product > 0.0f) {
      f32 inverse_length = 1 / std::sqrt(dot_product);
      d.x *= inverse_length;
      d.y *= inverse_length;
    }
  }

  d.x *= brush.line_width * 0.5f;
  d.y *= brush.line_width * 0.5f;

  vec2 vertices[] = {{p1.x + d.y, p1.y - d.x},
                     {p2.x + d.y, p2.y - d.x},
                     {p2.x - d.y, p2.y + d.x},
                     {p1.x - d.y, p1.y + d.x}};

  u32 start = AS_U32(draw_list.indices.size());

  u32 indices[] = {start,     start + 1, start + 2,
                   start + 3, start + 4, start + 5};

  draw_list.vertices.extend(vertices).unwrap();
  draw_list.indices.extend(indices).unwrap();

  u32 nclip_polygon_vertices = AS_U32(draw_list.clip_vertices.size());

  // triangulate_polygon(draw_list.clip_vertices, clip);

  nclip_polygon_vertices =
      AS_U32(draw_list.clip_vertices.size()) - nclip_polygon_vertices;

  u32 clip_start = AS_U32(draw_list.clip_indices.size());

  for (u32 index = clip_start; index < (clip_start + nclip_polygon_vertices);
       index++) {
    draw_list.clip_indices.push_inplace(index).unwrap();
  }

  draw_list.cmds
      .push(DrawCommand{.indices_offset = start,
                        .nindices = AS_U32(std::size(indices)),
                        .clip_indices_offset = clip_start,
                        .nclip_indices = nclip_polygon_vertices,
                        .transform = transform,
                        .color = brush.color,
                        .texture = brush.pattern.share()})
      .unwrap();
      */
    return *this;
  }

  Canvas& draw_rect(vec2 offset, vec2 extent) {
    vec2 points[4];

    polygons::rect(points, offset, extent);

    if (brush.fill) {
      return draw_convex_polygon_filled(points, {offset, extent},
                                        {{0, 0}, {1, 1}});
    } else {
      return draw_polygon_line(points);
    }
  }

  Canvas& draw_circle(vec2 center, f32 radius, usize nsegments) {
    stx::Vec<vec2> points{stx::os_allocator};
    points.resize(nsegments).unwrap();
    polygons::circle(points, center, radius, nsegments);

    if (brush.fill) {
      return draw_convex_polygon_filled(
          points, {center - radius, 2 * vec2{radius, radius}},
          {{0, 0}, {1, 1}});
    } else {
      return draw_polygon_line(points);
    }
  }

  Canvas& draw_ellipse(vec2 center, vec2 radius, usize nsegments) {
    stx::Vec<vec2> points{stx::os_allocator};
    points.resize(nsegments).unwrap();
    polygons::ellipse(points, center, radius, nsegments);

    if (brush.fill) {
      return draw_convex_polygon_filled(points, {center - radius, 2 * radius},
                                        {{0, 0}, {1, 1}});
    } else {
      return draw_polygon_line(points);
    }
  }

  Canvas& draw_round_rect(vec2 offset, vec2 extent, vec4 radii,
                          usize nsegments) {
    stx::Vec<vec2> points{stx::os_allocator};
    points.resize(nsegments * 4).unwrap();
    polygons::round_rect(points, offset, extent, radii, nsegments);

    if (brush.fill) {
      return draw_convex_polygon_filled(points, {offset, extent},
                                        {{0, 0}, {1, 1}});
    } else {
      return draw_polygon_line(points);
    }
  }

  // Text API
  Canvas& draw_text(stx::StringView text, vec2 position);

  // Image API
  Canvas& draw_image(Image const& image, vec2 offset);
  Canvas& draw_image(Image const& image, vec2 offset, vec2 extent);
  Canvas& draw_image(Image const& image, vec2 portion_offset, vec2 portion_size,
                     vec2 dest_offset, vec2 dest_extent);
};

inline void sample(Canvas& canvas, Image& image) {
  canvas.save()
      .rotate(45, 0, 0)
      .draw_circle({0, 0}, 20.0f, 20)
      .draw_image(image, {0.0, 0.0}, {20, 40})
      .restore()
      .scale(2.0f, 2.0f)
      .draw_line({0, 0}, {200, 200})
      .draw_text("Hello World, こんにちは世界", {10.0f, 10.0f})
      .draw_rect({0, 0}, {20, 20})
      .draw_round_rect({0.0f, 0.0f}, {20.0f, 20.0f},
                       {10.0f, 10.0f, 10.0f, 10.0f}, 20);
}

struct CanvasContext {
  stx::Vec<vk::SpanBuffer> vertex_buffers{stx::os_allocator};
  stx::Vec<vk::SpanBuffer> index_buffers{stx::os_allocator};

  // stx::Vec<stx::Vec<vk::DescriptorSets>> descriptor_sets{stx::os_allocator};

  vk::RecordingContext ctx;

  stx::Rc<vk::CommandQueue*> queue;

  // TODO(lamarrr): keep a mapping of [vksampler, vkimageview] to
  // vkdescriptorset, this won't work since it will end up retaining them for
  // longer than necessary

  CanvasContext(stx::Rc<vk::CommandQueue*> aqueue) : queue{std::move(aqueue)} {
    VkDevice dev = queue.handle->device.handle->device;

    auto vertex_input_attributes = vertex::attribute_descriptions();

    vk::DescriptorSetSpec descriptor_set_specs[] = {
        vk::DescriptorSetSpec{vk::DescriptorType::Sampler}};

    ctx.init(*queue.handle, vertex_shader_code, fragment_shader_code,
             vertex_input_attributes, sizeof(vertex), descriptor_set_specs);

    for (u32 i = 0; i < vk::SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
      vertex_buffers.push(vk::SpanBuffer{}).unwrap();
      index_buffers.push(vk::SpanBuffer{}).unwrap();
    }

    // for (u32 i = 0; i < vk::SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    //   // TODO(lamarrr): this won't work since we need to keep multiple of
    //   them
    //   // per frame. alternative: each texture already has a descriptor set,
    //   // which will be more efficient and reasonable
    //   vk::DescriptorSetSpec specs[] = {
    //       vk::DescriptorSetSpec{vk::DescriptorType::Sampler}};
    //   vk::DescriptorSets sets;
    //   sets.init(dev, specs);

    //   // descriptor_sets.push(   );
    // }
  }

  STX_MAKE_PINNED(CanvasContext)

  ~CanvasContext() {
    VkDevice dev = queue.handle->device.handle->device;

    for (vk::SpanBuffer& buff : vertex_buffers) buff.destroy(dev);

    for (vk::SpanBuffer& buff : index_buffers) buff.destroy(dev);

    ctx.destroy(dev);
  }

  void __write_vertices(stx::Span<vertex const> vertices,
                        stx::Span<u32 const> indices,
                        u32 next_frame_flight_index) {
    VkDevice dev = queue.handle->device.handle->device;
    VkPhysicalDeviceMemoryProperties const& memory_properties =
        queue.handle->device.handle->phy_device.handle->memory_properties;

    vk::CommandQueueFamilyInfo const& family_info = queue.handle->info.family;

    vertex_buffers[next_frame_flight_index].write(
        dev, memory_properties, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices);

    index_buffers[next_frame_flight_index].write(
        dev, memory_properties, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices);
  }

  void submit(vk::SwapChain const& swapchain, u32 swapchain_image_index,
              DrawList const& draw_list) {
    stx::Rc<vk::Device*> const& device = swapchain.queue.handle->device;

    VkDevice dev = device.handle->device;

    VkQueue queue = swapchain.queue.handle->info.queue;

    u32 frame = swapchain.next_frame_flight_index;

    VkCommandBuffer cmd_buffer = ctx.cmd_buffers[frame];

    ASR_VK_CHECK(vkWaitForFences(dev, 1, &swapchain.rendering_fences[frame],
                                 VK_TRUE, COMMAND_TIMEOUT));

    ASR_VK_CHECK(vkResetCommandBuffer(cmd_buffer, 0));

    __write_vertices(draw_list.vertices, draw_list.indices, frame);

    VkCommandBufferBeginInfo command_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    ASR_VK_CHECK(vkBeginCommandBuffer(cmd_buffer, &command_buffer_begin_info));

    VkClearValue clear_values[] = {
        {.color = VkClearColorValue{{0, 0, 0, 0}}},
        {.depthStencil = VkClearDepthStencilValue{.depth = 1, .stencil = 0}}};

    VkRenderPassBeginInfo render_pass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = swapchain.render_pass,
        .framebuffer = swapchain.framebuffers[swapchain_image_index],
        .renderArea =
            VkRect2D{.offset = {0, 0}, .extent = swapchain.image_extent},
        .clearValueCount = AS_U32(std::size(clear_values)),
        .pClearValues = clear_values};

    vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    // allocate as many descriptor sets as the number of draw commands to be
    // issued

    VkDeviceSize offset = 0;

    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &vertex_buffers[frame].buffer,
                           &offset);

    vkCmdBindIndexBuffer(cmd_buffer, index_buffers[frame].buffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      ctx.pipeline.pipeline);

    for (DrawCommand const& cmd : draw_list.cmds) {
      vk::PushConstants push_constant{
          .transform = cmd.transform,
          .overlay = {cmd.color.r / 255.0f, cmd.color.g / 255.0f,
                      cmd.color.b / 255.0f, cmd.color.a / 255.0f}};

      vkCmdPushConstants(
          cmd_buffer, ctx.pipeline.layout,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
          sizeof(vk::PushConstants), &push_constant);

      vk::DescriptorBinding set0[] = {vk::DescriptorBinding::make_sampler(
          cmd.texture.handle->image.handle->view, cmd.texture.handle->sampler)};

      stx::Span<vk::DescriptorBinding const> sets[] = {set0};

      // check list contained in frame to see if there's a descriptor set
      // pointing to the same resource

      ctx.descriptor_sets[frame].write(sets);

      VkViewport viewport{.x = 0.0f,
                          .y = 0.0f,
                          .width = AS_F32(swapchain.window_extent.width),
                          .height = AS_F32(swapchain.window_extent.height),
                          .minDepth = 0.0f,
                          .maxDepth = 1.0f};

      vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

      VkRect2D scissor{.offset = {AS_I32(cmd.clip_rect.offset.x),
                                  AS_I32(cmd.clip_rect.offset.y)},
                       .extent = {AS_U32(cmd.clip_rect.extent.x),
                                  AS_U32(cmd.clip_rect.extent.y)}};

      vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

      vkCmdBindDescriptorSets(
          cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipeline.layout, 0,
          AS_U32(ctx.descriptor_sets[frame].descriptor_sets.size()),
          ctx.descriptor_sets[frame].descriptor_sets.data(), 0, nullptr);

      vkCmdDrawIndexed(cmd_buffer, cmd.nindices, 1, cmd.indices_offset, 0, 0);
    }

    vkCmdEndRenderPass(cmd_buffer);

    ASR_VK_CHECK(vkEndCommandBuffer(cmd_buffer));

    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &swapchain.image_acquisition_semaphores[frame],
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &swapchain.rendering_semaphores[frame]};

    ASR_VK_CHECK(vkQueueSubmit(queue, 1, &submit_info,
                               swapchain.rendering_fences[frame]));
  }
};

}  // namespace gfx
}  // namespace asr
