/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/pass_bundle.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/engine/systems.h"
#include "ashura/std/allocators.h"
#include "ashura/std/color.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

using BezierRegions = shader::BezierRegions;

using FillRule = shader::FillRule;

struct PassInfo
{
  FrameGraph & frame_graph;

  PassBundle & passes;
};

struct BlurEncoder
{
  Framebuffer framebuffer_;

  Option<PassStencil> stencil_;

  bool upsample_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::DescriptorSet samplers_;

  gpu::DescriptorSet textures_;

  Vec<shader::blur::Blur> blurs_;

  explicit BlurEncoder(AllocatorRef allocator) :
    framebuffer_{},
    stencil_{},
    upsample_{},
    scissor_{},
    viewport_{},
    samplers_{},
    textures_{},
    blurs_{allocator}
  {
  }

  bool push(Framebuffer framebuffer, Option<PassStencil> stencil, bool upsample,
            RectU scissor, gpu::Viewport viewport, gpu::DescriptorSet samplers,
            gpu::DescriptorSet textures, shader::blur::Blur const & blur)
  {
    framebuffer_ = framebuffer;
    stencil_     = stencil;
    upsample_    = upsample;
    scissor_     = scissor;
    viewport_    = viewport;
    samplers_    = samplers;
    textures_    = textures;
    blurs_.push(blur).unwrap();
    return true;
  }

  void pass(PassInfo const & info)
  {
    auto blurs   = blurs_.view();
    auto i_blurs = info.frame_graph.push_ssbo(blurs);

    info.frame_graph.add_pass(
      upsample_ ? "Blur.Upsample"_str : "Blur.Downsample"_str,
      [info, framebuffer = framebuffer_, stencil = this->stencil_,
       upsample = this->upsample_, scissor = this->scissor_,
       viewport = this->viewport_, samplers = this->samplers_,
       textures = this->textures_, num_instances = size32(blurs),
       i_blurs](FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
        auto blurs = frame_graph.get_struct_buffer(i_blurs);

        BlurPassParams params{
          .framebuffer = framebuffer,
          .stencil     = stencil,
          .scissor     = scissor,
          .viewport    = viewport,
          .samplers    = samplers,
          .textures    = textures,
          .blurs       = blurs,
          .instances   = Slice32{0, num_instances}
        };

        if (upsample)
        {
          info.passes.blur->upsample(enc, params);
        }
        else
        {
          info.passes.blur->downsample(enc, params);
        }
      });
  }

  void clear()
  {
    framebuffer_ = {};
    stencil_     = none;
    upsample_    = false;
    scissor_     = {};
    viewport_    = {};
    samplers_    = {};
    textures_    = {};
    blurs_.clear();
  }
};

template <typename Material>
struct NgonEncoder
{
  Framebuffer framebuffer_;

  Option<PassStencil> stencil_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::DescriptorSet samplers_;

  gpu::DescriptorSet textures_;

  f32x4x4 world_to_ndc_;

  Vec<u32> index_counts_;

  Vec<f32x4x4> transforms_;

  Vec<f32x2> vertices_;

  Vec<u32> indices_;

  Vec<Material> materials_;

  Str shader_variant_;

  explicit NgonEncoder(AllocatorRef allocator, Str shader_variant = ""_str) :
    framebuffer_{},
    stencil_{},
    scissor_{},
    viewport_{},
    samplers_{},
    textures_{},
    world_to_ndc_{},
    index_counts_{allocator},
    transforms_{allocator},
    vertices_{allocator},
    indices_{allocator},
    materials_{allocator},
    shader_variant_{shader_variant}
  {
  }

  static bool pass_cmp(Framebuffer a_framebuffer, Option<PassStencil> a_stencil,
                       RectU a_scissor, gpu::Viewport a_viewport,
                       gpu::DescriptorSet a_samplers,
                       gpu::DescriptorSet a_textures, f32x4x4 a_world_to_ndc,
                       Framebuffer b_framebuffer, Option<PassStencil> b_stencil,
                       RectU b_scissor, gpu::Viewport b_viewport,
                       gpu::DescriptorSet b_samplers,
                       gpu::DescriptorSet b_textures, f32x4x4 b_world_to_ndc

  )
  {
    return mem::eq(span({a_framebuffer}), span({b_framebuffer})) &&
           mem::eq(span({a_stencil}), span({b_stencil})) &&
           a_scissor == b_scissor &&
           mem::eq(span({a_viewport}), span({b_viewport})) &&
           a_world_to_ndc == b_world_to_ndc && a_samplers == b_samplers &&
           a_textures == b_textures;
  }

  bool push(Framebuffer framebuffer, Option<PassStencil> stencil,
            gpu::DescriptorSet samplers, gpu::DescriptorSet textures,
            RectU scissor, gpu::Viewport viewport, f32x4x4 world_to_ndc,
            f32x4x4 transform, Span<f32x2 const> vertices,
            Span<u32 const> indices, Material const & material)
  {
    auto is_new_pass =
      pass_cmp(framebuffer_, stencil_, scissor_, viewport_, samplers_,
               textures_, world_to_ndc_, framebuffer, stencil, scissor,
               viewport, samplers, textures, world_to_ndc);

    if (is_new_pass)
    {
      clear();
    }

    framebuffer_  = framebuffer;
    stencil_      = stencil;
    scissor_      = scissor;
    viewport_     = viewport;
    samplers_     = samplers;
    textures_     = textures;
    world_to_ndc_ = world_to_ndc;

    index_counts_.push(size32(indices)).unwrap();
    transforms_.push(transform).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices).unwrap();
    materials_.push(material).unwrap();

    return is_new_pass;
  }

  void pass(PassInfo const & info)
  {
    auto index_counts = index_counts_.view();
    auto transforms   = transforms_.view();
    auto vertices     = vertices_.view();
    auto indices      = indices_.view();
    auto materials    = materials_.view();

    auto i_world_to_ndc = info.frame_graph.push_ssbo(span({world_to_ndc_}));
    auto i_transforms   = info.frame_graph.push_ssbo(transforms);
    auto i_vertices     = info.frame_graph.push_ssbo(vertices);
    auto i_indices      = info.frame_graph.push_ssbo(indices);
    auto i_materials    = info.frame_graph.push_ssbo(materials);
    auto i_index_counts = info.frame_graph.push_buffer(index_counts);

    // [ ] implement deferring of begin_rendewring and end_rendering?

    // [ ] lifetime of PassInfo refs

    info.frame_graph.add_pass(
      "Ngon"_str,
      [info, framebuffer = framebuffer_, stencil = this->stencil_,
       scissor = this->scissor_, viewport = this->viewport_,
       samplers = this->samplers_, textures = this->textures_, i_world_to_ndc,
       i_transforms, i_vertices, i_indices, i_materials, i_index_counts,
       shader_variant = this->shader_variant_](FrameGraph & frame_graph,
                                               gpu::CommandEncoder & enc) {
        auto world_to_ndc = frame_graph.get_struct_buffer(i_world_to_ndc);
        auto transforms   = frame_graph.get_struct_buffer(i_transforms);
        auto vertices     = frame_graph.get_struct_buffer(i_vertices);
        auto indices      = frame_graph.get_struct_buffer(i_indices);
        auto materials    = frame_graph.get_struct_buffer(i_materials);
        auto index_counts = frame_graph.get_buffer<u32>(i_index_counts);

        NgonPassParams params{.framebuffer    = framebuffer,
                              .stencil        = stencil,
                              .scissor        = scissor,
                              .viewport       = viewport,
                              .samplers       = samplers,
                              .textures       = textures,
                              .world_to_ndc   = world_to_ndc,
                              .transforms     = transforms,
                              .vertices       = vertices,
                              .indices        = indices,
                              .materials      = materials,
                              .first_instance = 0,
                              .index_counts   = index_counts};

        info.passes.ngon->encode(enc, params, shader_variant);
      });
  }

  void clear()
  {
    framebuffer_ = {};
    stencil_     = none;
    scissor_     = {};
    viewport_    = {};
    samplers_    = {};
    textures_    = {};
    index_counts_.clear();
    transforms_.clear();
    vertices_.clear();
    indices_.clear();
    materials_.clear();
  }
};

template <typename Shape, typename Material>
struct SdfEncoder
{
  Framebuffer framebuffer_;

  Option<PassStencil> stencil_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::DescriptorSet samplers_;

  gpu::DescriptorSet textures_;

  f32x4x4 world_to_ndc_;

  Vec<Shape> shapes_;

  Vec<f32x4x4> transforms_;

  Vec<Material> materials_;

  Str shader_variant_;

  explicit SdfEncoder(AllocatorRef allocator, Str shader_variant = ""_str) :
    framebuffer_{},
    stencil_{},
    scissor_{},
    viewport_{},
    samplers_{},
    textures_{},
    world_to_ndc_{},
    shapes_{allocator},
    transforms_{allocator},
    materials_{allocator},
    shader_variant_{shader_variant}
  {
  }

  static bool pass_cmp(Framebuffer a_framebuffer, Option<PassStencil> a_stencil,
                       RectU a_scissor, gpu::Viewport a_viewport,
                       gpu::DescriptorSet a_samplers,
                       gpu::DescriptorSet a_textures, f32x4x4 a_world_to_ndc,
                       Framebuffer b_framebuffer, Option<PassStencil> b_stencil,
                       RectU b_scissor, gpu::Viewport b_viewport,
                       gpu::DescriptorSet b_samplers,
                       gpu::DescriptorSet b_textures, f32x4x4 b_world_to_ndc)
  {
    // [ ] use mem::eq of a struct
    return mem::eq(span({a_framebuffer}), span({b_framebuffer})) &&
           mem::eq(span({a_stencil}), span({b_stencil})) &&
           a_scissor == b_scissor &&
           mem::eq(span({a_viewport}), span({b_viewport})) &&
           a_world_to_ndc == b_world_to_ndc && a_samplers == b_samplers &&
           a_textures == b_textures;
  }

  bool push(Framebuffer framebuffer, Option<PassStencil> stencil, RectU scissor,
            gpu::Viewport viewport, gpu::DescriptorSet samplers,
            gpu::DescriptorSet textures, f32x4x4 world_to_ndc, Shape shape,
            f32x4x4 transform, Material const & material)
  {
    auto is_new_pass =
      pass_cmp(framebuffer_, stencil_, scissor_, viewport_, samplers_,
               textures_, world_to_ndc_, framebuffer, stencil, scissor,
               viewport, samplers_, textures_, world_to_ndc);

    // [ ] flush instead? how to structurere
    if (is_new_pass)
    {
      clear();
    }

    framebuffer_  = framebuffer;
    stencil_      = stencil;
    scissor_      = scissor;
    viewport_     = viewport;
    samplers_     = samplers;
    textures_     = textures;
    world_to_ndc_ = world_to_ndc;

    shapes_.push(shape).unwrap();
    transforms_.push(transform).unwrap();
    materials_.push(material).unwrap();

    return is_new_pass;
  }

  void pass(PassInfo const & info)
  {
    auto shapes     = shapes_.view();
    auto transforms = transforms_.view();
    auto materials  = materials_.view();

    auto i_world_to_ndc = info.frame_graph.push_ssbo(span({world_to_ndc_}));
    auto i_shapes       = info.frame_graph.push_ssbo(shapes);
    auto i_transforms   = info.frame_graph.push_ssbo(transforms);
    auto i_materials    = info.frame_graph.push_ssbo(materials);

    auto num_instances = size32(shapes);

    info.frame_graph.add_pass(
      "SDF"_str,
      [info, framebuffer = this->framebuffer_, stencil = this->stencil_,
       scissor = this->scissor_, viewport = this->viewport_,
       samplers = this->samplers_, textures = this->textures_, i_world_to_ndc,
       i_shapes, i_transforms, i_materials,
       shader_variant = this->shader_variant_,
       num_instances](FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
        auto world_to_ndc = frame_graph.get_struct_buffer(i_world_to_ndc);
        auto shapes       = frame_graph.get_struct_buffer(i_transforms);
        auto transforms   = frame_graph.get_struct_buffer(i_transforms);
        auto materials    = frame_graph.get_struct_buffer(i_materials);

        SdfPassParams params{
          .framebuffer  = framebuffer,
          .stencil      = stencil,
          .scissor      = scissor,
          .viewport     = viewport,
          .samplers     = samplers,
          .textures     = textures,
          .world_to_ndc = world_to_ndc,
          .shapes       = shapes,
          .transforms   = transforms,
          .materials    = materials,
          .instances{0, num_instances}
        };

        info.passes.sdf->encode(enc, params, shader_variant);
      });
  }

  void clear()
  {
    framebuffer_  = {};
    stencil_      = none;
    scissor_      = {};
    viewport_     = {};
    samplers_     = {};
    textures_     = {};
    world_to_ndc_ = {};
    shapes_.clear();
    transforms_.clear();
    materials_.clear();
  }
};

struct ContourStencilEncoder
{
  Framebuffer framebuffer_;

  u32 write_mask_;

  RectU scissor_;

  gpu::Viewport viewport_;

  shader::FillRule fill_rule_;

  bool invert_;

  f32x4x4 world_to_ndc_;

  Vec<u32> triangle_offsets_;

  Vec<f32x4x4> transforms_;

  Vec<f32x2> vertices_;

  Vec<BezierRegions> regions_;

  Vec<u32> triangle_counts_;

  explicit ContourStencilEncoder(AllocatorRef allocator) :
    framebuffer_{},
    write_mask_{},
    scissor_{},
    viewport_{},
    fill_rule_{shader::FillRule::EvenOdd},
    invert_{},
    world_to_ndc_{},
    triangle_offsets_{allocator},
    transforms_{allocator},
    vertices_{allocator},
    regions_{allocator},
    triangle_counts_{allocator}
  {
  }

  static bool pass_cmp(Framebuffer a_framebuffer, u32 a_write_mask,
                       RectU a_scissor, gpu::Viewport a_viewport,
                       shader::FillRule a_fill_rule, bool a_invert,
                       f32x4x4 a_world_to_ndc, Framebuffer b_framebuffer,
                       u32 b_write_mask, RectU b_scissor,
                       gpu::Viewport b_viewport, shader::FillRule b_fill_rule,
                       bool b_invert, f32x4x4 b_world_to_ndc)
  {
    return mem::eq(span({a_framebuffer}), span({b_framebuffer})) &&
           mem::eq(span({a_write_mask}), span({b_write_mask})) &&
           a_scissor == b_scissor &&
           mem::eq(span({a_viewport}), span({b_viewport})) &&
           a_world_to_ndc == b_world_to_ndc && a_fill_rule == b_fill_rule &&
           a_invert == b_invert;
  }

  bool push(Framebuffer framebuffer, u32 write_mask, RectU scissor,
            gpu::Viewport viewport, Span<f32x2 const> vertices,
            f32x4x4 world_to_ndc, f32x4x4 transform,
            Span<BezierRegions const> regions, shader::FillRule fill_rule,
            bool invert)
  {
    auto is_new_pass =
      pass_cmp(framebuffer_, write_mask_, scissor_, viewport_, fill_rule_,
               invert_, world_to_ndc_, framebuffer, write_mask, scissor,
               viewport, fill_rule, invert, world_to_ndc);

    if (is_new_pass)
    {
      clear();
    }

    framebuffer_  = framebuffer;
    write_mask_   = write_mask;
    scissor_      = scissor;
    viewport_     = viewport;
    world_to_ndc_ = world_to_ndc;
    fill_rule_    = fill_rule;
    invert        = invert_;

    auto triangle_offset = size32(vertices_) / 3;
    auto triangle_count  = size32(vertices) / 3;

    vertices_.extend(vertices).unwrap();
    transforms_.push(transform).unwrap();
    regions_.extend(regions).unwrap();
    triangle_offsets_.push(triangle_offset).unwrap();
    triangle_counts_.push(triangle_count).unwrap();

    return is_new_pass;
  }

  void pass(PassInfo const & info)
  {
    auto vertices         = vertices_.view();
    auto transforms       = transforms_.view();
    auto regions          = regions_.view();
    auto triangle_offsets = triangle_offsets_.view();
    auto triangle_counts  = triangle_counts_.view();

    auto i_vertices         = info.frame_graph.push_ssbo(vertices);
    auto i_world_to_ndc     = info.frame_graph.push_ssbo(span({world_to_ndc_}));
    auto i_transforms       = info.frame_graph.push_ssbo(transforms);
    auto i_regions          = info.frame_graph.push_ssbo(regions);
    auto i_triangle_offsets = info.frame_graph.push_ssbo(triangle_offsets);
    auto i_triangle_counts  = info.frame_graph.push_buffer(triangle_counts);

    info.frame_graph.add_pass(
      "Contour Stencil"_str,
      [info, framebuffer = this->framebuffer_, write_mask = this->write_mask_,
       scissor = this->scissor_, viewport = this->viewport_,
       fill_rule = this->fill_rule_, invert = this->invert_, i_vertices,
       i_world_to_ndc, i_transforms, i_regions, i_triangle_offsets,
       i_triangle_counts](FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
        auto vertices     = frame_graph.get_struct_buffer(i_vertices);
        auto world_to_ndc = frame_graph.get_struct_buffer(i_world_to_ndc);
        auto transforms   = frame_graph.get_struct_buffer(i_transforms);
        auto regions      = frame_graph.get_struct_buffer(i_regions);
        auto triangle_offsets =
          frame_graph.get_struct_buffer(i_triangle_offsets);
        auto triangle_counts = frame_graph.get_buffer<u32>(i_triangle_counts);

        ContourStencilPassParams params{
          .stencil          = framebuffer.depth_stencil,
          .write_mask       = write_mask,
          .scissor          = scissor,
          .viewport         = viewport,
          .fill_rule        = fill_rule,
          .invert           = invert,
          .world_to_ndc     = world_to_ndc,
          .triangle_offsets = triangle_offsets,
          .transforms       = transforms,
          .vertices         = vertices,
          .regions          = regions,
          .triangle_counts  = triangle_counts,
        };

        info.passes.contour_stencil->encode(enc, params);
      });
  }

  void clear()
  {
    framebuffer_ = {};
    write_mask_  = 0;
    scissor_     = {};
    viewport_    = {};
    vertices_.clear();
    world_to_ndc_ = {};
    transforms_.clear();
    regions_.clear();
    triangle_offsets_.clear();
    triangle_counts_.clear();
    fill_rule_ = shader::FillRule::EvenOdd;
    invert_    = false;
  }
};

template <typename Material>
struct QuadEncoder
{
  Framebuffer framebuffer_;

  Option<PassStencil> stencil_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::DescriptorSet samplers_;

  gpu::DescriptorSet textures_;

  f32x4x4 world_to_ndc_;

  Vec<f32x4x4> quads_;

  Vec<f32x4x4> transforms_;

  Vec<Material> materials_;

  Str shader_variant_;

  explicit QuadEncoder(AllocatorRef allocator, Str shader_variant = ""_str) :
    framebuffer_{},
    stencil_{},
    scissor_{},
    viewport_{},
    samplers_{},
    textures_{},
    world_to_ndc_{},
    quads_{allocator},
    transforms_{allocator},
    materials_{allocator},
    shader_variant_{shader_variant}
  {
  }

  static bool pass_cmp(Framebuffer a_framebuffer, Option<PassStencil> a_stencil,
                       RectU a_scissor, gpu::Viewport a_viewport,
                       gpu::DescriptorSet a_samplers,
                       gpu::DescriptorSet a_textures, f32x4x4 a_world_to_ndc,
                       Framebuffer b_framebuffer, Option<PassStencil> b_stencil,
                       RectU b_scissor, gpu::Viewport b_viewport,
                       gpu::DescriptorSet b_samplers,
                       gpu::DescriptorSet b_textures, f32x4x4 b_world_to_ndc)
  {
    return mem::eq(span({a_framebuffer}), span({b_framebuffer})) &&
           mem::eq(span({a_stencil}), span({b_stencil})) &&
           a_scissor == b_scissor &&
           mem::eq(span({a_viewport}), span({b_viewport})) &&
           a_world_to_ndc == b_world_to_ndc && a_samplers == b_samplers &&
           a_textures == b_textures;
  }

  bool push(Framebuffer framebuffer, Option<PassStencil> stencil,
            gpu::DescriptorSet samplers, gpu::DescriptorSet textures,
            RectU scissor, gpu::Viewport viewport, f32x4x4 world_to_ndc,
            f32x4x4 quad, f32x4x4 transform, Material const & material)
  {
    auto is_new_pass =
      pass_cmp(framebuffer_, stencil_, scissor_, viewport_, samplers_,
               textures_, world_to_ndc_, framebuffer, stencil, scissor,
               viewport, samplers, textures, world_to_ndc);

    if (is_new_pass) [[unlikely]]    // [ ] unlikely
    {
      clear();
    }

    framebuffer_  = framebuffer;
    scissor_      = scissor;
    viewport_     = viewport;
    stencil_      = stencil;
    samplers_     = samplers;
    textures_     = textures;
    world_to_ndc_ = world_to_ndc;

    quads_.push(quad).unwrap();
    transforms_.push(transform).unwrap();
    materials_.push(material).unwrap();

    return is_new_pass;
  }

  void pass(PassInfo const & info)
  {
    auto quads      = quads_.view();
    auto transforms = transforms_.view();
    auto materials  = materials_.view();

    auto i_world_to_ndc = info.frame_graph.push_ssbo(span({world_to_ndc_}));
    auto i_quads        = info.frame_graph.push_ssbo(quads);
    auto i_transforms   = info.frame_graph.push_ssbo(transforms);
    auto i_materials    = info.frame_graph.push_ssbo(materials);

    info.frame_graph.add_pass(
      "Quad"_str,
      [info, framebuffer = this->framebuffer_, stencil = this->stencil_,
       scissor = this->scissor_, viewport = this->viewport_,
       samplers = this->samplers_, textures = this->textures_, i_world_to_ndc,
       i_quads, i_transforms, i_materials, num_instances = size32(quads),
       shader_variant = this->shader_variant_](FrameGraph & frame_graph,
                                               gpu::CommandEncoder & enc) {
        auto world_to_ndc = frame_graph.get_struct_buffer(i_world_to_ndc);
        auto quads        = frame_graph.get_struct_buffer(i_quads);
        auto transforms   = frame_graph.get_struct_buffer(i_transforms);
        auto materials    = frame_graph.get_struct_buffer(i_materials);

        QuadPassParams params{
          .framebuffer  = framebuffer,
          .stencil      = stencil,
          .scissor      = scissor,
          .viewport     = viewport,
          .samplers     = samplers,
          .textures     = textures,
          .world_to_ndc = world_to_ndc,
          .quads        = quads,
          .transforms   = transforms,
          .materials    = materials,
          .instances{0, num_instances}
        };

        info.passes.quad->encode(enc, params, shader_variant);
      });
  }

  void clear()
  {
    framebuffer_  = {};
    stencil_      = none;
    scissor_      = {};
    viewport_     = {};
    samplers_     = {};
    textures_     = {};
    world_to_ndc_ = {};
    quads_.clear();
    transforms_.clear();
    materials_.clear();
  }
};

template <typename World,
          typename Vertex,      // = shader::pbr::Vertex
          typename Index,       // = shader::pbr::Index
          typename Material,    // = shader::pbr::BaseMaterial,
          typename Light        // = shader::pbr::PunctualLight
          >
struct PbrEncoder
{
  Framebuffer framebuffer_;

  Option<PassStencil> stencil_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::DescriptorSet samplers_;

  gpu::DescriptorSet textures_;

  f32x4x4 world_to_ndc_;

  StructBufferSpan vertices_;

  StructBufferSpan indices_;

  World world_;

  Material material_;

  // [ ] lights

  Str shader_variant_;

  explicit PbrEncoder([[maybe_unused]] AllocatorRef allocator,
                      Str                           shader_variant = ""_str) :
    framebuffer_{},
    stencil_{},
    scissor_{},
    viewport_{},
    samplers_{},
    textures_{},
    world_to_ndc_{},
    vertices_{},
    indices_{},
    world_{},
    material_{},
    shader_variant_{shader_variant}
  {
  }

  hash64 push(Framebuffer framebuffer, RectU scissor,
              Span<shader::pbr::Vertex const> vertices,
              Span<shader::pbr::Index const>  indices,
              Material const &                material);

  void pass(PassInfo const & info);

  void clear()
  {
    /*
    layers.clear();
    scissors.clear();
    vertices.clear();
    indices.clear();
    index_counts.clear();
    materials.clear();*/
  }
};

}    // namespace ash
