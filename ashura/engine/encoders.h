/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/pass_bundle.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/std/allocator.h"
#include "ashura/std/math.h"
#include "ashura/std/obj.h"
#include "ashura/std/types.h"

namespace ash
{

struct BlurEncoder
{
  struct Item
  {
    Framebuffer         framebuffer;
    Option<PassStencil> stencil;
    bool                upsample;
    RectU               scissor;
    gpu::Viewport       viewport;
    gpu::DescriptorSet  samplers;
    gpu::DescriptorSet  textures;
    shader::blur::Blur  blur;
  };

  Framebuffer framebuffer_;

  Option<PassStencil> stencil_;

  bool upsample_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::DescriptorSet samplers_;

  gpu::DescriptorSet textures_;

  Vec<shader::blur::Blur> blurs_;

  explicit BlurEncoder(AllocatorRef allocator, Item const & item) :
    framebuffer_{item.framebuffer},
    stencil_{item.stencil},
    upsample_{item.upsample},
    scissor_{item.scissor},
    viewport_{item.viewport},
    samplers_{item.samplers},
    textures_{item.textures},
    blurs_{allocator}
  {
    blurs_.push(item.blur).unwrap();
  }

  BlurEncoder(BlurEncoder const &)             = delete;
  BlurEncoder(BlurEncoder &&)                  = default;
  BlurEncoder & operator=(BlurEncoder const &) = delete;
  BlurEncoder & operator=(BlurEncoder &&)      = default;
  ~BlurEncoder()                               = default;

  void pass(FrameGraph & frame_graph, PassBundle & passes) const
  {
    auto blurs   = blurs_.view();
    auto i_blurs = frame_graph.push_ssbo(blurs);

    frame_graph.add_pass(
      upsample_ ? "Blur.Upsample"_str : "Blur.Downsample"_str,
      [&passes, framebuffer = framebuffer_, stencil = this->stencil_,
       upsample = this->upsample_, scissor = this->scissor_,
       viewport = this->viewport_, samplers = this->samplers_,
       textures = this->textures_, num_instances = size32(blurs),
       i_blurs](FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
        auto blurs = frame_graph.get(i_blurs);

        BlurPassParams params{
          .framebuffer = framebuffer,
          .stencil     = stencil,
          .scissor     = scissor,
          .viewport    = viewport,
          .samplers    = samplers,
          .textures    = textures,
          .blurs       = blurs,
          .instances   = Slice32{0, num_instances},
          .upsample    = upsample
        };

        passes.blur->encode(enc, params);
      });
  }
};

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

  Vec<u8> materials_;

  ShaderVariantId shader_variant_;

  template <typename Material>
  explicit NgonEncoder(AllocatorRef allocator, Framebuffer const & framebuffer,
                       Option<PassStencil> const & stencil, RectU scissor,
                       gpu::Viewport const & viewport,
                       gpu::DescriptorSet samplers, gpu::DescriptorSet textures,
                       f32x4x4 const & world_to_ndc, f32x4x4 const & transform,
                       Span<f32x2 const> vertices, Span<u32 const> indices,
                       Material const & material,
                       ShaderVariantId  shader_variant) :
    framebuffer_{framebuffer},
    stencil_{stencil},
    scissor_{scissor},
    viewport_{viewport},
    samplers_{samplers},
    textures_{textures},
    world_to_ndc_{world_to_ndc},
    index_counts_{allocator},
    transforms_{allocator},
    vertices_{allocator},
    indices_{allocator},
    materials_{allocator},
    shader_variant_{shader_variant}
  {
    push_(transform, vertices, indices, material);
  }

  NgonEncoder(NgonEncoder const &)             = delete;
  NgonEncoder(NgonEncoder &&)                  = default;
  NgonEncoder & operator=(NgonEncoder const &) = delete;
  NgonEncoder & operator=(NgonEncoder &&)      = default;
  ~NgonEncoder()                               = default;

  template <typename Material>
  void push_(f32x4x4 const & transform, Span<f32x2 const> vertices,
             Span<u32 const> indices, Material const & material)
  {
    index_counts_.push(size32(indices)).unwrap();
    transforms_.push(transform).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices).unwrap();
    materials_.extend(Span{&material, 1}.as_u8()).unwrap();
  }

  template <typename Material>
  bool push(Framebuffer const &         framebuffer,
            Option<PassStencil> const & stencil, RectU scissor,
            gpu::Viewport const & viewport, gpu::DescriptorSet samplers,
            gpu::DescriptorSet textures, f32x4x4 const & world_to_ndc,
            f32x4x4 const & transform, Span<f32x2 const> vertices,
            Span<u32 const> indices, Material const & material,
            ShaderVariantId shader_variant)
  {
    auto mergeable =
      obj::byte_eq(Tuple{framebuffer_, stencil_, scissor_, viewport_, samplers_,
                         textures_, world_to_ndc_, shader_variant_},
                   Tuple{framebuffer, stencil, scissor, viewport, samplers,
                         textures, world_to_ndc, shader_variant});

    if (!mergeable)
    {
      return false;
    }

    push_(transform, vertices, indices, material);

    return true;
  }

  void pass(FrameGraph & frame_graph, PassInfo const & info) const
  {
    auto index_counts = index_counts_.view();
    auto transforms   = transforms_.view();
    auto vertices     = vertices_.view();
    auto indices      = indices_.view();
    auto materials    = materials_.view();

    auto i_world_to_ndc = frame_graph.push_ssbo(span({world_to_ndc_}));
    auto i_transforms   = frame_graph.push_ssbo(transforms);
    auto i_vertices     = frame_graph.push_ssbo(vertices);
    auto i_indices      = frame_graph.push_ssbo(indices);
    auto i_materials    = frame_graph.push_ssbo(materials);
    auto i_index_counts = frame_graph.push_buffer(index_counts);

    // [ ] implement deferring of begin_rendewring and end_rendering?

    frame_graph.add_pass(
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
};

struct SdfEncoder
{
  Framebuffer framebuffer_;

  Option<PassStencil> stencil_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::DescriptorSet samplers_;

  gpu::DescriptorSet textures_;

  f32x4x4 world_to_ndc_;

  Vec<u8> shapes_;

  Vec<f32x4x4> transforms_;

  Vec<u8> materials_;

  ShaderVariantId shader_variant_;

  template <typename Shape, typename Material>
  explicit SdfEncoder(AllocatorRef allocator, Framebuffer const & framebuffer,
                      Option<PassStencil> const & stencil, RectU scissor,
                      gpu::Viewport const & viewport,
                      gpu::DescriptorSet samplers, gpu::DescriptorSet textures,
                      f32x4x4 const & world_to_ndc, Shape const & shape,
                      f32x4x4 const & transform, Material const & material,
                      ShaderVariantId shader_variant) :
    framebuffer_{framebuffer},
    stencil_{stencil},
    scissor_{scissor},
    viewport_{viewport},
    samplers_{samplers},
    textures_{textures},
    world_to_ndc_{world_to_ndc},
    shapes_{allocator},
    transforms_{allocator},
    materials_{allocator},
    shader_variant_{shader_variant}
  {
    push_(shape, transform, material);
  }

  SdfEncoder(SdfEncoder const &)             = delete;
  SdfEncoder(SdfEncoder &&)                  = default;
  SdfEncoder & operator=(SdfEncoder const &) = delete;
  SdfEncoder & operator=(SdfEncoder &&)      = default;
  ~SdfEncoder()                              = default;

  template <typename Shape, typename Material>
  void push_(Shape const & shape, f32x4x4 const & transform,
             Material const & material)
  {
    shapes_.extend(Span{&shape, 1}.as_u8()).unwrap();
    transforms_.push(transform).unwrap();
    materials_.extend(Span{&material, 1}.as_u8()).unwrap();
  }

  template <typename Shape, typename Material>
  bool push(Framebuffer const &         framebuffer,
            Option<PassStencil> const & stencil, RectU scissor,
            gpu::Viewport const & viewport, gpu::DescriptorSet samplers,
            gpu::DescriptorSet textures, f32x4x4 const & world_to_ndc,
            Shape const & shape, f32x4x4 const & transform,
            Material const & material, ShaderVariantId shader_variant)
  {
    // [ ] separate into a different function? lifetime?
    auto mergeable =
      obj::byte_eq(Tuple{framebuffer_, stencil_, scissor_, viewport_, samplers_,
                         textures_, world_to_ndc_, shader_variant_},
                   Tuple{framebuffer, stencil, scissor, viewport, samplers,
                         textures, world_to_ndc, shader_variant});

    if (!mergeable)
    {
      return false;
    }

    push_(shape, transform, material);

    return true;
  }

  void pass(FrameGraph & frame_graph, PassInfo const & info) const
  {
    auto shapes     = shapes_.view();
    auto transforms = transforms_.view();
    auto materials  = materials_.view();

    auto i_world_to_ndc = frame_graph.push_ssbo(span({world_to_ndc_}));
    auto i_shapes       = frame_graph.push_ssbo(shapes);
    auto i_transforms   = frame_graph.push_ssbo(transforms);
    auto i_materials    = frame_graph.push_ssbo(materials);

    auto num_instances = size32(shapes);

    frame_graph.add_pass(
      "SDF"_str,
      [info, framebuffer = this->framebuffer_, stencil = this->stencil_,
       scissor = this->scissor_, viewport = this->viewport_,
       samplers = this->samplers_, textures = this->textures_, i_world_to_ndc,
       i_shapes, i_transforms, i_materials,
       shader_variant = this->shader_variant_,
       num_instances](FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
        auto world_to_ndc = frame_graph.get_struct_buffer(i_world_to_ndc);
        auto shapes       = frame_graph.get_struct_buffer(i_shapes);
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

  Vec<shader::BezierRegions> regions_;

  Vec<u32> triangle_counts_;

  explicit ContourStencilEncoder(
    AllocatorRef allocator, Framebuffer const & framebuffer, u32 write_mask,
    RectU scissor, gpu::Viewport const & viewport, shader::FillRule fill_rule,
    bool invert, f32x4x4 const & world_to_ndc, f32x4x4 const & transform,
    Span<f32x2 const> vertices, Span<shader::BezierRegions const> regions) :
    framebuffer_{framebuffer},
    write_mask_{write_mask},
    scissor_{scissor},
    viewport_{viewport},
    fill_rule_{fill_rule},
    invert_{invert},
    world_to_ndc_{world_to_ndc},
    triangle_offsets_{allocator},
    transforms_{allocator},
    vertices_{allocator},
    regions_{allocator},
    triangle_counts_{allocator}
  {
    push_(transform, vertices, regions);
  }

  ContourStencilEncoder(ContourStencilEncoder const &)             = delete;
  ContourStencilEncoder(ContourStencilEncoder &&)                  = default;
  ContourStencilEncoder & operator=(ContourStencilEncoder const &) = delete;
  ContourStencilEncoder & operator=(ContourStencilEncoder &&)      = default;
  ~ContourStencilEncoder()                                         = default;

  void push_(f32x4x4 const & transform, Span<f32x2 const> vertices,
             Span<shader::BezierRegions const> regions)
  {
    auto triangle_offset = size32(vertices_) / 3;
    auto triangle_count  = size32(vertices) / 3;

    vertices_.extend(vertices).unwrap();
    transforms_.push(transform).unwrap();
    regions_.extend(regions).unwrap();
    triangle_offsets_.push(triangle_offset).unwrap();
    triangle_counts_.push(triangle_count).unwrap();
  }

  bool push(Framebuffer const & framebuffer, u32 write_mask, RectU scissor,
            gpu::Viewport const & viewport, shader::FillRule fill_rule,
            bool invert, f32x4x4 const & world_to_ndc,
            f32x4x4 const & transform, Span<f32x2 const> vertices,
            Span<shader::BezierRegions const> regions)
  {
    auto mergeable =
      obj::byte_eq(Tuple{framebuffer_, write_mask_, scissor_, viewport_,
                         fill_rule_, invert_, world_to_ndc_},
                   Tuple{framebuffer, write_mask, scissor, viewport, fill_rule,
                         invert, world_to_ndc});

    if (!mergeable)
    {
      return false;
    }

    push_(transform, vertices, regions);

    return true;
  }

  void pass(FrameGraph & frame_graph, PassInfo const & info) const
  {
    auto vertices         = vertices_.view();
    auto transforms       = transforms_.view();
    auto regions          = regions_.view();
    auto triangle_offsets = triangle_offsets_.view();
    auto triangle_counts  = triangle_counts_.view();

    auto i_vertices         = frame_graph.push_ssbo(vertices);
    auto i_world_to_ndc     = frame_graph.push_ssbo(span({world_to_ndc_}));
    auto i_transforms       = frame_graph.push_ssbo(transforms);
    auto i_regions          = frame_graph.push_ssbo(regions);
    auto i_triangle_offsets = frame_graph.push_ssbo(triangle_offsets);
    auto i_triangle_counts  = frame_graph.push_buffer(triangle_counts);

    frame_graph.add_pass(
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
};

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

  Vec<u8> materials_;

  ShaderVariantId shader_variant_;

  template <typename Material>
  explicit QuadEncoder(AllocatorRef allocator, Framebuffer const & framebuffer,
                       Option<PassStencil> const & stencil, RectU scissor,
                       gpu::Viewport viewport, gpu::DescriptorSet samplers,
                       gpu::DescriptorSet textures,
                       f32x4x4 const & world_to_ndc, f32x4x4 const & quad,
                       f32x4x4 const & transform, Material const & material,
                       ShaderVariantId shader_variant) :
    framebuffer_{framebuffer},
    stencil_{stencil},
    scissor_{scissor},
    viewport_{viewport},
    samplers_{samplers},
    textures_{textures},
    world_to_ndc_{world_to_ndc},
    quads_{allocator},
    transforms_{allocator},
    materials_{allocator},
    shader_variant_{shader_variant}
  {
    push_(quad, transform, material);
  }

  QuadEncoder(QuadEncoder const &)             = delete;
  QuadEncoder(QuadEncoder &&)                  = default;
  QuadEncoder & operator=(QuadEncoder const &) = delete;
  QuadEncoder & operator=(QuadEncoder &&)      = default;
  ~QuadEncoder()                               = default;

  template <typename Material>
  void push_(f32x4x4 const & quad, f32x4x4 const & transform,
             Material const & material)
  {
    quads_.push(quad).unwrap();
    transforms_.push(transform).unwrap();
    materials_.extend(Span{&material, 1}.as_u8()).unwrap();
  }

  template <typename Material>
  bool push(Framebuffer const &         framebuffer,
            Option<PassStencil> const & stencil, RectU scissor,
            gpu::Viewport viewport, gpu::DescriptorSet samplers,
            gpu::DescriptorSet textures, f32x4x4 const & world_to_ndc,
            f32x4x4 const & quad, f32x4x4 const & transform,
            Material const & material, ShaderVariantId shader_variant)
  {
    auto mergeable =
      obj::byte_eq(Tuple{framebuffer_, stencil_, scissor_, viewport_, samplers_,
                         textures_, world_to_ndc_, shader_variant_},
                   Tuple{framebuffer, stencil, scissor, viewport, samplers,
                         textures, world_to_ndc, shader_variant});

    if (!mergeable)
    {
      return false;
    }

    quads_.push(quad).unwrap();
    transforms_.push(transform).unwrap();
    materials_.extend(Span{&material, 1}.as_u8()).unwrap();

    return true;
  }

  void pass(FrameGraph & frame_graph, PassInfo const & info)
  {
    auto quads      = quads_.view();
    auto transforms = transforms_.view();
    auto materials  = materials_.view();

    auto i_world_to_ndc = frame_graph.push_ssbo(span({world_to_ndc_}));
    auto i_quads        = frame_graph.push_ssbo(quads);
    auto i_transforms   = frame_graph.push_ssbo(transforms);
    auto i_materials    = frame_graph.push_ssbo(materials);

    frame_graph.add_pass(
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
};

struct PbrEncoder
{
  Framebuffer framebuffer_;

  Option<PassStencil> stencil_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::PolygonMode polygon_mode_;

  gpu::DescriptorSet samplers_;

  gpu::DescriptorSet textures_;

  StructBufferSpan vertices_;    // = shader::pbr::Vertex

  StructBufferSpan indices_;    // = shader::pbr::Index

  u32 num_indices_;

  Vec<u8> world_;    // = shader::pbr::World

  Vec<u8> material_;    // = shader::pbr::BaseMaterial,

  Vec<u8> lights_;    // = shader::pbr::PunctualLight

  ShaderVariantId shader_variant_;

  template <typename World, typename Material, typename Light>
  explicit PbrEncoder(AllocatorRef allocator, Framebuffer const & framebuffer,
                      Option<PassStencil> const & stencil, RectU scissor,
                      gpu::Viewport const & viewport,
                      gpu::PolygonMode      polygon_mode,
                      gpu::DescriptorSet samplers, gpu::DescriptorSet textures,
                      StructBufferSpan vertices, StructBufferSpan indices,
                      u32 num_indices, World const & world,
                      Material const & material, Span<Light> lights,
                      ShaderVariantId shader_variant) :
    framebuffer_{framebuffer},
    stencil_{stencil},
    scissor_{scissor},
    viewport_{viewport},
    polygon_mode_{polygon_mode},
    samplers_{samplers},
    textures_{textures},
    vertices_{vertices},
    indices_{indices},
    num_indices_{num_indices},
    world_{allocator},
    material_{allocator},
    lights_{allocator},
    shader_variant_{shader_variant}
  {
    world_.extend(Span{&world, 1}.as_u8()).unwrap();
    material_.extend(Span{&material, 1}.as_u8()).unwrap();
    lights_.extend(lights.as_u8()).unwrap();
  }

  PbrEncoder(PbrEncoder const &)             = delete;
  PbrEncoder(PbrEncoder &&)                  = default;
  PbrEncoder & operator=(PbrEncoder const &) = delete;
  PbrEncoder & operator=(PbrEncoder &&)      = default;
  ~PbrEncoder()                              = default;

  void pass(FrameGraph & frame_graph, PassInfo const & info)
  {
    auto i_world    = frame_graph.push_ssbo(world_.view());
    auto i_material = frame_graph.push_ssbo(material_.view());
    auto i_lights   = frame_graph.push_ssbo(lights_.view());

    frame_graph.add_pass(
      "PBR"_str,
      [info, framebuffer = framebuffer_, stencil = this->stencil_,
       scissor = this->scissor_, polygon_mode = this->polygon_mode_,
       viewport = this->viewport_, samplers = this->samplers_,
       textures = this->textures_, vertices = this->vertices_,
       indices = this->indices_, num_indices = this->num_indices_, i_world,
       i_material, i_lights, shader_variant = this->shader_variant_](
        FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
        auto world    = frame_graph.get_struct_buffer(i_world);
        auto material = frame_graph.get_struct_buffer(i_material);
        auto lights   = frame_graph.get_struct_buffer(i_lights);

        PBRPassParams params{.framebuffer  = framebuffer,
                             .stencil      = stencil,
                             .scissor      = scissor,
                             .viewport     = viewport,
                             .polygon_mode = polygon_mode,
                             .samplers     = samplers,
                             .textures     = textures,
                             .vertices     = vertices,
                             .indices      = indices,
                             .world        = world,
                             .material     = material,
                             .lights       = lights,
                             .num_indices  = num_indices};

        info.passes.pbr->encode(enc, params, shader_variant);
      });
  }
};

}    // namespace ash
