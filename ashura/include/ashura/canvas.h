#pragma once

#include <cinttypes>
#include <cmath>

#include "ashura/font.h"
#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/shaders.h"
#include "ashura/vulkan.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace asr {

namespace gfx {

struct vertex {
  vec2 position;
  vec2 st;
};

struct Brush {
  color color = colors::BLACK;
  bool fill = true;
  f32 line_thickness = 1;
  Image pattern;
  TextStyle text_style;
};

struct DrawCommand {
  u32 indices_offset = 0;
  u32 nindices = 0;
  rect clip_rect;
  mat4 transform = mat4::identity();
  color color = colors::BLACK;
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

namespace polygons {

inline void rect(stx::Span<vec2> polygon, asr::rect area) {
  polygon[0] = area.offset;
  polygon[1] = {area.offset.x + area.extent.x, area.offset.y};
  polygon[2] = area.offset + area.extent;
  polygon[3] = {area.offset.x, area.offset.y + area.extent.y};
}

inline void circle(stx::Span<vec2> polygon, vec2 center, f32 radius,
                   usize nsegments) {
  if (nsegments == 0 || radius <= 0) return;

  f32 step = AS_F32((2 * M_PI) / nsegments);

  for (usize i = 0; i < nsegments; i++) {
    polygon[i] = vec2{center.x + radius - radius * std::cos(i * step),
                      center.y + radius - radius * std::sin(i * step)};
  }
}

inline void ellipse(stx::Span<vec2> polygon, vec2 center, vec2 radii,
                    usize nsegments) {
  if (nsegments == 0 || radii.x <= 0 || radii.y <= 0) return;

  f32 step = AS_F32((2 * M_PI) / nsegments);

  for (usize i = 0; i < nsegments; i++) {
    polygon[i] = vec2{center.x + radii.x - radii.x * std::cos(i * step),
                      center.y + radii.y - radii.y * std::sin(i * step)};
  }
}

/// {polygon.size() == nsegments * 4}
inline void round_rect(stx::Span<vec2> polygon, asr::rect area, vec4 radii,
                       usize nsegments) {
  if (nsegments == 0) return;

  radii.x = std::min(radii.x, std::min(area.extent.x, area.extent.y));
  radii.y = std::min(radii.y, std::min(area.extent.x, area.extent.y));
  radii.z = std::min(radii.z, std::min(area.extent.x, area.extent.y));
  radii.w = std::min(radii.w, std::min(area.extent.x, area.extent.y));

  f32 step = AS_F32((M_PI / 2) / nsegments);

  usize i = 0;

  vec2 xoffset{area.offset.x + radii.x, area.offset.y + radii.x};

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    polygon[i] = vec2{xoffset.x - radii.x * std::cos(segment * step),
                      xoffset.y - radii.x * std::sin(segment * step)};
  }

  vec2 yoffset{area.offset.x + area.extent.x - radii.y,
               area.offset.y + radii.y};

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    polygon[i] =
        vec2{yoffset.x - radii.y * std::cos(AS_F32(M_PI / 2) + segment * step),
             yoffset.y - radii.y * std::sin(AS_F32(M_PI / 2) + segment * step)};
  }

  vec2 zoffset{area.offset.x + area.extent.x - radii.z,
               area.offset.y + area.extent.y - radii.z};

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    polygon[i] =
        vec2{zoffset.x - radii.z * std::cos(AS_F32(M_PI) + segment * step),
             zoffset.y - radii.z * std::sin(AS_F32(M_PI) + segment * step)};
  }

  vec2 woffset{area.offset.x + radii.w,
               area.offset.y + area.extent.y - radii.w};

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    polygon[i] = vec2{
        woffset.x - radii.w * std::cos(AS_F32(M_PI * 3 / 2) + segment * step),
        woffset.y - radii.w * std::sin(AS_F32(M_PI * 3 / 2) + segment * step)};
  }
}

}  // namespace polygons

constexpr vec2 normalize_for_viewport(vec2 position, vec2 viewport_extent) {
  // transform to -1 to +1 range from x pointing right and y pointing upwards
  return (2 * position / viewport_extent) - 1;
}

inline void transform_vertices_to_viewport_and_generate_texture_coordinates(
    stx::Span<vertex> vertices, vec2 viewport_extent, rect polygon_area,
    rect texture_area) {
  for (usize i = 0; i < vertices.size(); i++) {
    vec2 position = vertices[i].position;

    vec2 normalized = normalize_for_viewport(position, viewport_extent);

    // transform vertex position into texture coordinates (within the polygon's
    // extent)
    vec2 st = (position - polygon_area.offset) / polygon_area.extent;

    // map it into the portion of the texture we are interested in
    st = (texture_area.offset + st * texture_area.extent);

    vertices[i] = vertex{.position = normalized, .st = st};
  }
}

inline void triangulate_convex_polygon(stx::Vec<u32>& indices,
                                       u32 first_vertex_index,
                                       stx::Span<vec2 const> polygon) {
  ASR_CHECK(polygon.size() >= 3, "polygon must have 3 or more points");

  for (u32 i = 2; i < polygon.size(); i++) {
    indices.push_inplace(first_vertex_index).unwrap();
    indices.push_inplace(first_vertex_index + (i - 1)).unwrap();
    indices.push_inplace(first_vertex_index + i).unwrap();
  }
}

inline void triangulate_line(stx::Vec<vertex>& ivertices,
                             stx::Vec<u32>& iindices, u32 first_vertex_index,
                             stx::Span<vec2 const> points, f32 line_thickness) {
  if (points.size() < 2) return;

  bool has_previous_line = false;

  u32 vertex_index = first_vertex_index;

  for (usize i = 1; i < points.size(); i++) {
    vec2 p1 = points[i - 1];
    vec2 p2 = points[i];

    // the angles are specified in clockwise direction to be compatible with the
    // vulkan coordinate system
    //
    // get the angle of inclination of p2 to p1
    vec2 d = p2 - p1;
    f32 m = std::abs(d.y / std::max(stx::f32_epsilon, d.x));
    f32 alpha = std::atan(m);

    // use direction of the points to get the actual overall angle of
    // inclination of p2 to p1
    if (d.x < 0 && d.y > 0) {
      alpha = AS_F32(M_PI - alpha);
    } else if (d.x < 0 && d.y < 0) {
      alpha = AS_F32(M_PI + alpha);
    } else if (d.x > 0 && d.y < 0) {
      alpha = AS_F32(2 * M_PI - alpha);
    } else {
      // d.x >=0 && d.y >= 0
    }

    // line will be at a parallel angle
    alpha = AS_F32(alpha + M_PI / 2);

    f32 hw = line_thickness / 2;

    vec2 f = vec2{hw * std::cos(alpha), hw * std::sin(alpha)};
    vec2 g = vec2{hw * std::cos(AS_F32(M_PI + alpha)),
                  hw * std::sin(AS_F32(M_PI + alpha))};

    vec2 s1 = p1 + f;
    vec2 s2 = p1 + g;

    vec2 t1 = p2 + f;
    vec2 t2 = p2 + g;

    vertex vertices[] = {{.position = s1, .st = {}},
                         {.position = s2, .st = {}},
                         {.position = t1, .st = {}},
                         {.position = t2, .st = {}}};

    u32 indices[] = {vertex_index, vertex_index + 1, vertex_index + 3,
                     vertex_index, vertex_index + 2, vertex_index + 3};

    ivertices.extend(vertices).unwrap();
    iindices.extend(indices).unwrap();

    if (has_previous_line) {
      u32 prev_line_vertex_index = vertex_index - 4;

      u32 indices[] = {prev_line_vertex_index + 2,
                       prev_line_vertex_index + 3,
                       vertex_index,
                       prev_line_vertex_index + 2,
                       prev_line_vertex_index + 3,
                       vertex_index + 1};

      iindices.extend(indices).unwrap();
    }

    has_previous_line = true;

    vertex_index += 4;
  }
}

/// coordinates are specified in top-left origin space with x pointing to the
/// right and y pointing downwards.
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
    if (!transform_state_stack.is_empty()) {
      transform = *(transform_state_stack.end() - 1);
      transform_state_stack.erase(transform_state_stack.span().slice(1));
    }

    if (!clip_rect_stack.is_empty()) {
      clip_rect = *(clip_rect_stack.end() - 1);
      clip_rect_stack.erase(clip_rect_stack.span().slice(1));
    }

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
    vec3 translation =
        2 * vec3{x, y, z} / vec3{viewport_extent.x, viewport_extent.y, 1};
    transform = transforms::translate(translation) * transform;
    return *this;
  }

  Canvas& translate(f32 x, f32 y) { return translate(x, y, 0); }

  Canvas& rotate(f32 x, f32 y, f32 z) {
    transform = transforms::rotate_z(z) * transforms::rotate_y(y) *
                transforms::rotate_x(x) * transform;
    return *this;
  }

  Canvas& scale(f32 x, f32 y, f32 z) {
    transform = transforms::scale(vec3{x, y, z}) * transform;
    return *this;
  }

  Canvas& scale(f32 x, f32 y) { return scale(x, y, 1); }

  Canvas& clear() {
    draw_list.clear();

    vertex vertices[] = {{{0, 0}, {0, 1}},
                         {{viewport_extent.x, 0}, {1, 1}},
                         {viewport_extent, {1, 0}},
                         {{0, viewport_extent.y}, {0, 0}}};

    for (vertex& vertex : vertices) {
      vertex.position =
          normalize_for_viewport(vertex.position, viewport_extent);
    }

    draw_list.vertices.extend(vertices).unwrap();

    u32 indices[] = {0, 1, 2, 0, 2, 3};

    draw_list.indices.extend(indices).unwrap();

    draw_list.cmds
        .push(DrawCommand{.indices_offset = 0,
                          .nindices = AS_U32(std::size(indices)),
                          .clip_rect = rect{{0, 0}, viewport_extent},
                          .transform = mat4::identity(),
                          .color = brush.color,
                          .texture = brush.pattern.share()})
        .unwrap();

    return *this;
  }

  Canvas& draw_lines(stx::Span<vec2 const> points, rect area, rect texture_area,
                     Image const& pattern) {
    if (points.size() < 2 || !area.is_visible() || !clip_rect.overlaps(area)) {
      return *this;
    }

    u32 indices_offset = AS_U32(draw_list.indices.size());

    u32 vertices_offset = AS_U32(draw_list.vertices.size());

    triangulate_line(draw_list.vertices, draw_list.indices, vertices_offset,
                     points, brush.line_thickness);

    u32 nindices = AS_U32(draw_list.indices.size() - indices_offset);

    transform_vertices_to_viewport_and_generate_texture_coordinates(
        draw_list.vertices.span().slice(vertices_offset), viewport_extent, area,
        texture_area);

    draw_list.cmds
        .push(DrawCommand{.indices_offset = indices_offset,
                          .nindices = nindices,
                          .clip_rect = clip_rect,
                          .transform = transform,
                          .color = brush.color,
                          .texture = pattern.share()})
        .unwrap();

    return *this;
  }

  Canvas& draw_convex_polygon_filled(stx::Span<vec2 const> polygon, rect area,
                                     rect texture_area, Image const& pattern) {
    if (polygon.size() < 3 || !area.is_visible() || !clip_rect.overlaps(area)) {
      return *this;
    }

    u32 indices_offset = AS_U32(draw_list.indices.size());

    u32 vertices_offset = AS_U32(draw_list.vertices.size());

    triangulate_convex_polygon(draw_list.indices, vertices_offset, polygon);

    u32 nindices = AS_U32(draw_list.indices.size() - indices_offset);

    for (usize i = 0; i < polygon.size(); i++) {
      draw_list.vertices.push(vertex{.position = polygon[i], .st = {0, 0}})
          .unwrap();
    }

    transform_vertices_to_viewport_and_generate_texture_coordinates(
        draw_list.vertices.span().slice(vertices_offset), viewport_extent, area,
        texture_area);

    draw_list.cmds
        .push(DrawCommand{.indices_offset = indices_offset,
                          .nindices = nindices,
                          .clip_rect = clip_rect,
                          .transform = transform,
                          .color = brush.color,
                          .texture = pattern.share()})
        .unwrap();

    return *this;
  }

  Canvas& draw_line(vec2 p1, vec2 p2) {
    vec2 points[] = {p1, p2};

    rect area;

    return draw_lines(points, area, {{0, 0}, {1, 1}}, brush.pattern);
  }

  Canvas& draw_rect(rect area) {
    vec2 points[4];

    polygons::rect(points, area);

    rect texture_area{{0, 0}, {1, 1}};

    if (brush.fill) {
      return draw_convex_polygon_filled(points, area, texture_area,
                                        brush.pattern);
    } else {
      area.offset = area.offset -
                    vec2{brush.line_thickness / 2, brush.line_thickness / 2};
      area.extent =
          area.extent + vec2{brush.line_thickness, brush.line_thickness};
      vec2 opoints[] = {points[0], points[1], points[2],
                        points[3], points[0], points[1]};
      return draw_lines(opoints, area, texture_area, brush.pattern);
    }
  }

  Canvas& draw_circle(vec2 center, f32 radius, usize nsegments) {
    stx::Vec<vec2> points{stx::os_allocator};
    points.resize(nsegments).unwrap();
    polygons::circle(points, center, radius, nsegments);

    rect area{center - radius, 2 * vec2{radius, radius}};
    rect texture_area{{0, 0}, {1, 1}};

    if (brush.fill) {
      return draw_convex_polygon_filled(points, area, texture_area,
                                        brush.pattern);
    } else {
      if (points.size() > 0) {
        points.push_inplace(points[0]).unwrap();
      }
      if (points.size() > 1) {
        points.push_inplace(points[1]).unwrap();
      }
      area.offset = area.offset -
                    vec2{brush.line_thickness / 2, brush.line_thickness / 2};
      area.extent =
          area.extent + vec2{brush.line_thickness, brush.line_thickness};
      return draw_lines(points, area, texture_area, brush.pattern);
    }
  }

  Canvas& draw_ellipse(vec2 center, vec2 radius, usize nsegments) {
    stx::Vec<vec2> points{stx::os_allocator};
    points.resize(nsegments).unwrap();
    polygons::ellipse(points, center, radius, nsegments);

    rect area{center - radius, 2 * radius};
    rect texture_area{{0, 0}, {1, 1}};

    if (brush.fill) {
      return draw_convex_polygon_filled(points, area, texture_area,
                                        brush.pattern);
    } else {
      if (points.size() > 0) {
        points.push_inplace(points[0]).unwrap();
      }
      if (points.size() > 1) {
        points.push_inplace(points[1]).unwrap();
      }
      area.offset = area.offset -
                    vec2{brush.line_thickness / 2, brush.line_thickness / 2};
      area.extent =
          area.extent + vec2{brush.line_thickness, brush.line_thickness};
      return draw_lines(points, area, texture_area, brush.pattern);
    }
  }

  Canvas& draw_round_rect(rect area, vec4 radii, usize nsegments) {
    stx::Vec<vec2> points{stx::os_allocator};
    points.resize(nsegments * 4).unwrap();
    polygons::round_rect(points, area, radii, nsegments);

    rect texture_area{{0, 0}, {1, 1}};

    if (brush.fill) {
      return draw_convex_polygon_filled(points, area, texture_area,
                                        brush.pattern);
    } else {
      if (points.size() > 0) {
        points.push_inplace(points[0]).unwrap();
      }
      if (points.size() > 1) {
        points.push_inplace(points[1]).unwrap();
      }
      area.offset = area.offset -
                    vec2{brush.line_thickness / 2, brush.line_thickness / 2};
      area.extent =
          area.extent + vec2{brush.line_thickness, brush.line_thickness};
      return draw_lines(points, area, texture_area, brush.pattern);
    }
  }

  // Image API
  Canvas& draw_image(Image const& image, rect area, rect image_portion,
                     vec4 border_radii, usize nsegments) {
    stx::Vec<vec2> points{stx::os_allocator};
    points.resize(nsegments * 4).unwrap();
    polygons::round_rect(points, area, border_radii, nsegments);
    return draw_convex_polygon_filled(points, area, image_portion, image);
  }

  Canvas& draw_image(Image const& image, rect area, vec4 border_radii,
                     usize nsegments) {
    rect texture_area{{0, 0}, {1, 1}};
    return draw_image(image, area, texture_area, border_radii, nsegments);
  }

  // Text API
  // TODO(lamarrr): we need separate layout pass, use callbacks to perform
  // certain actions on layout calculation
  Canvas& draw_text(Font& font, FontCache& cache, std::string_view text,
                    TextStyle const& style = {}, f32 max_width = stx::f32_max,
                    hb_script_t script = HB_SCRIPT_LATIN,
                    hb_language_t language = hb_language_from_string("en", 2)) {
    ASR_CHECK(style.direction == HB_DIRECTION_LTR ||
              style.direction == HB_DIRECTION_RTL);

    f32 font_scale = AS_F32(style.font_height) / cache.font_height;

    hb_font_set_scale(font.hbfont, 64 * cache.font_height,
                      64 * cache.font_height);
    hb_buffer_reset(font.hbscratch_buffer);
    hb_buffer_set_script(font.hbscratch_buffer, script);
    hb_buffer_set_direction(font.hbscratch_buffer, style.direction);
    hb_buffer_set_language(font.hbscratch_buffer, language);
    hb_buffer_add_utf8(font.hbscratch_buffer, text.data(), text.size(), 0,
                       text.size());

    hb_feature_t features[] = {
        {Font::KERNING_FEATURE, style.use_kerning, 0,
         std::numeric_limits<unsigned int>::max()},
        {Font::LIGATURE_FEATURE, style.use_ligatures, 0,
         std::numeric_limits<unsigned int>::max()},
        {Font::CONTEXTUAL_LIGATURE_FEATURE, style.use_ligatures, 0,
         std::numeric_limits<unsigned int>::max()}};

    hb_shape(font.hbfont, font.hbscratch_buffer, features, std::size(features));

    unsigned int nglyphs;
    hb_glyph_info_t* glyph_info =
        hb_buffer_get_glyph_infos(font.hbscratch_buffer, &nglyphs);
    // hb_glyph_position_t* glyph_pos =
    //    hb_buffer_get_glyph_positions(font.hbscratch_buffer, &nglyphs);

    // TODO(lamarrr): CONSIDER: canvas.clip_rect do not render beyond clip rect
    // apply transform to coordinates to see if any of the coordinates fall
    // inside it, if not discard certain parts of the text

    // for each text (paragraph)
    //
    // if word wrap is enabled seek to the end of the word, calculate the extent
    // of the word, if its x position doesn't exceed the max_width layout on
    // that line, if not wrap to the next line.
    //
    // if word wrap is not enabled just
    // keep drawing all the text and advancing
    //
    // if character is not space advance by advance.x + letter_spacing
    //
    // if character is space behave like a word_spacing * line_height character
    // was there and draw nothing
    //
    // if character is tab, perform procedure for space style.num_tab_spaces
    // times
    //
    // once all this layout is done, using the text align, align the text to the
    // desired alignments
    //
    //
    //
    for (usize i = 0; i < nglyphs; ++i) {
      u32 codepoint = glyph_info[i].codepoint;

      switch (codepoint) {
        // space
        case 0: {
        } break;

          // tab
        case 4: {
        } break;

        default: {
          stx::Span glyph = cache.entries.span().which(
              [codepoint](FontCacheEntry const& entry) {
                return entry.codepoint == codepoint;
              });

          if (!glyph.is_empty()) {
            FontCacheEntry const& g = glyph[0];
            g.s0;
            g.s1;
            g.t0;
            g.t1;
            g.pos.y;

            vec2 pos = font_scale * g.pos;
            vec2 advance = font_scale * g.advance;

            // xAdvance
          } else {
            // draw rect
          }
        } break;
      }
    }
  }
};

struct CanvasContext {
  stx::Vec<vk::SpanBuffer> vertex_buffers{stx::os_allocator};
  stx::Vec<vk::SpanBuffer> index_buffers{stx::os_allocator};

  vk::RecordingContext ctx;

  stx::Rc<vk::CommandQueue*> queue;

  CanvasContext(stx::Rc<vk::CommandQueue*> aqueue) : queue{std::move(aqueue)} {
    VkVertexInputAttributeDescription vertex_input_attributes[] = {
        {.location = 0,
         .binding = 0,
         .format = VK_FORMAT_R32G32_SFLOAT,
         .offset = offsetof(vertex, position)},
        {.location = 1,
         .binding = 0,
         .format = VK_FORMAT_R32G32_SFLOAT,
         .offset = offsetof(vertex, st)}};

    vk::DescriptorSetSpec descriptor_set_specs[] = {
        vk::DescriptorSetSpec{vk::DescriptorType::CombinedImageSampler}};

    VkDescriptorPoolSize descriptor_pool_sizes[] = {
        {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1}};

    ctx.init(queue.share(), vertex_shader_code, fragment_shader_code,
             vertex_input_attributes, sizeof(vertex), descriptor_set_specs,
             descriptor_pool_sizes, 1);

    for (u32 i = 0; i < vk::SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
      vertex_buffers.push(vk::SpanBuffer{}).unwrap();
      index_buffers.push(vk::SpanBuffer{}).unwrap();
    }
  }

  STX_MAKE_PINNED(CanvasContext)

  ~CanvasContext() {
    VkDevice dev = queue.handle->device.handle->device;

    for (vk::SpanBuffer& buff : vertex_buffers) buff.destroy(dev);

    for (vk::SpanBuffer& buff : index_buffers) buff.destroy(dev);

    ctx.destroy();
  }

  void __write_vertices(stx::Span<vertex const> vertices,
                        stx::Span<u32 const> indices,
                        u32 next_frame_flight_index) {
    VkDevice dev = queue.handle->device.handle->device;
    VkPhysicalDeviceMemoryProperties const& memory_properties =
        queue.handle->device.handle->phy_device.handle->memory_properties;

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

    VkCommandBuffer cmd_buffer = ctx.draw_cmd_buffers[frame];

    __write_vertices(draw_list.vertices, draw_list.indices, frame);

    u32 nallocated_descriptor_sets = AS_U32(ctx.descriptor_sets[frame].size());

    u32 ndraw_calls = AS_U32(draw_list.cmds.size());

    u32 ndescriptor_sets_per_draw_call =
        AS_U32(ctx.descriptor_set_layouts.size());

    u32 nrequired_descriptor_sets =
        ndescriptor_sets_per_draw_call * ndraw_calls;

    u32 max_ndescriptor_sets = ctx.descriptor_pool_infos[frame].max_sets;

    if (ndescriptor_sets_per_draw_call > 0) {
      if (nrequired_descriptor_sets > nallocated_descriptor_sets) {
        u32 nallocatable_combined_image_samplers = 0;

        for (VkDescriptorPoolSize size :
             ctx.descriptor_pool_infos[frame].sizes) {
          if (size.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            nallocatable_combined_image_samplers = size.descriptorCount;
            break;
          }
        }

        if (nrequired_descriptor_sets > max_ndescriptor_sets ||
            nrequired_descriptor_sets > nallocatable_combined_image_samplers) {
          if (!ctx.descriptor_sets[frame].is_empty()) {
            ASR_VK_CHECK(
                vkFreeDescriptorSets(dev, ctx.descriptor_pools[frame],
                                     AS_U32(ctx.descriptor_sets[frame].size()),
                                     ctx.descriptor_sets[frame].data()));
          }

          vkDestroyDescriptorPool(dev, ctx.descriptor_pools[frame], nullptr);

          stx::Vec<VkDescriptorPoolSize> sizes{stx::os_allocator};

          sizes
              .push({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                     .descriptorCount = nrequired_descriptor_sets})
              .unwrap();

          VkDescriptorPoolCreateInfo descriptor_pool_create_info{
              .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
              .pNext = nullptr,
              .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
              .maxSets = nrequired_descriptor_sets,
              .poolSizeCount = AS_U32(sizes.size()),
              .pPoolSizes = sizes.data()};

          ASR_VK_CHECK(vkCreateDescriptorPool(dev, &descriptor_pool_create_info,
                                              nullptr,
                                              &ctx.descriptor_pools[frame]));

          ctx.descriptor_pool_infos[frame] = vk::DescriptorPoolInfo{
              .sizes = std::move(sizes), .max_sets = nrequired_descriptor_sets};

          ctx.descriptor_sets[frame].resize(nrequired_descriptor_sets).unwrap();

          for (u32 i = 0; i < ndraw_calls; i++) {
            VkDescriptorSetAllocateInfo descriptor_set_allocate_info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = ctx.descriptor_pools[frame],
                .descriptorSetCount = AS_U32(ctx.descriptor_set_layouts.size()),
                .pSetLayouts = ctx.descriptor_set_layouts.data()};

            ASR_VK_CHECK(vkAllocateDescriptorSets(
                dev, &descriptor_set_allocate_info,
                ctx.descriptor_sets[frame].data() +
                    i * ndescriptor_sets_per_draw_call));
          }
        } else {
          ctx.descriptor_sets[frame].resize(nrequired_descriptor_sets).unwrap();

          VkDescriptorSetAllocateInfo descriptor_set_allocate_info{
              .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
              .pNext = nullptr,
              .descriptorPool = ctx.descriptor_pools[frame],
              .descriptorSetCount = AS_U32(ctx.descriptor_set_layouts.size()),
              .pSetLayouts = ctx.descriptor_set_layouts.data()};

          for (u32 i =
                   nallocated_descriptor_sets / ndescriptor_sets_per_draw_call;
               i < nrequired_descriptor_sets / ndescriptor_sets_per_draw_call;
               i++) {
            ASR_VK_CHECK(vkAllocateDescriptorSets(
                dev, &descriptor_set_allocate_info,
                ctx.descriptor_sets[frame].data() +
                    i * ndescriptor_sets_per_draw_call));
          }
        }
      }
    }

    ASR_VK_CHECK(vkWaitForFences(dev, 1, &swapchain.rendering_fences[frame],
                                 VK_TRUE, COMMAND_TIMEOUT));

    ASR_VK_CHECK(vkResetFences(dev, 1, &swapchain.rendering_fences[frame]));

    ASR_VK_CHECK(vkResetCommandBuffer(cmd_buffer, 0));

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

    VkDeviceSize offset = 0;

    for (usize icmd = 0; icmd < draw_list.cmds.size(); icmd++) {
      VkDescriptorImageInfo image_info{
          .sampler = draw_list.cmds[icmd].texture.handle->sampler,
          .imageView = draw_list.cmds[icmd].texture.handle->image.handle->view,
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

      VkWriteDescriptorSet write{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .pNext = nullptr,
          .dstSet = ctx.descriptor_sets[frame][icmd],
          .dstBinding = 0,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo = &image_info,
          .pBufferInfo = nullptr,
          .pTexelBufferView = nullptr,
      };

      vkUpdateDescriptorSets(dev, 1, &write, 0, nullptr);
    }

    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      ctx.pipeline.pipeline);

    ASR_CHECK(vertex_buffers[frame].is_valid());
    ASR_CHECK(index_buffers[frame].is_valid());

    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &vertex_buffers[frame].buffer,
                           &offset);

    vkCmdBindIndexBuffer(cmd_buffer, index_buffers[frame].buffer, 0,
                         VK_INDEX_TYPE_UINT32);

    for (usize icmd = 0; icmd < draw_list.cmds.size(); icmd++) {
      DrawCommand const& cmd = draw_list.cmds[icmd];

      VkViewport viewport{.x = 0,
                          .y = 0,
                          .width = AS_F32(swapchain.window_extent.width),
                          .height = AS_F32(swapchain.window_extent.height),
                          .minDepth = 0,
                          .maxDepth = 1};

      vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

      VkRect2D scissor{.offset = {AS_I32(cmd.clip_rect.offset.x),
                                  AS_I32(cmd.clip_rect.offset.y)},
                       .extent = {AS_U32(cmd.clip_rect.extent.x),
                                  AS_U32(cmd.clip_rect.extent.y)}};

      vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

      vk::PushConstants push_constant{
          .transform = cmd.transform.transpose(),
          .color = {cmd.color.r / 255.0f, cmd.color.g / 255.0f,
                    cmd.color.b / 255.0f, cmd.color.a / 255.0f}};

      vkCmdPushConstants(
          cmd_buffer, ctx.pipeline.layout,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
          sizeof(vk::PushConstants), &push_constant);

      vkCmdBindDescriptorSets(
          cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipeline.layout, 0,
          ndescriptor_sets_per_draw_call,
          &ctx.descriptor_sets[frame][icmd * ndescriptor_sets_per_draw_call], 0,
          nullptr);

      vkCmdDrawIndexed(cmd_buffer, cmd.nindices, 1, cmd.indices_offset, 0, 0);
    }

    vkCmdEndRenderPass(cmd_buffer);

    ASR_VK_CHECK(vkEndCommandBuffer(cmd_buffer));

    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &swapchain.image_acquisition_semaphores[frame],
        .pWaitDstStageMask = &wait_stage,
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
