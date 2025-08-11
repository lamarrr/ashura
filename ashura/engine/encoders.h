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

struct SdfEncoder
{
  template <typename Shape, typename Material>
  struct Item
  {
    Framebuffer         framebuffer;
    Option<PassStencil> stencil;
    RectU               scissor;
    gpu::Viewport       viewport;
    gpu::DescriptorSet  samplers;
    gpu::DescriptorSet  textures;
    f32x4x4             world_to_ndc;
    Shape               shape;
    f32x4x4             transform;
    Material            material;
    ShaderVariantId     shader_variant;
  };

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
  explicit SdfEncoder(Allocator                  allocator,
                      Item<Shape, Material> const & item) :
    framebuffer_{item.framebuffer},
    stencil_{item.stencil},
    scissor_{item.scissor},
    viewport_{item.viewport},
    samplers_{item.samplers},
    textures_{item.textures},
    world_to_ndc_{item.world_to_ndc},
    shapes_{allocator},
    transforms_{allocator},
    materials_{allocator},
    shader_variant_{item.shader_variant}
  {
    push_(item.shape, item.transform, item.material);
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
  bool push(Item<Shape, Material> const & item)
  {
    auto mergeable =
      obj::byte_eq(Tuple{framebuffer_, stencil_, scissor_, viewport_, samplers_,
                         textures_, world_to_ndc_, shader_variant_},
                   Tuple{item.framebuffer, item.stencil, item.scissor,
                         item.viewport, item.samplers, item.textures,
                         item.world_to_ndc, item.shader_variant});

    if (!mergeable)
    {
      return false;
    }

    push_(item.shape, item.transform, item.material);

    return true;
  }

  void pass(FrameGraph & frame_graph, PassBundle & passes) const
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
      [&passes, framebuffer = this->framebuffer_, stencil = this->stencil_,
       scissor = this->scissor_, viewport = this->viewport_,
       samplers = this->samplers_, textures = this->textures_, i_world_to_ndc,
       i_shapes, i_transforms, i_materials,
       shader_variant = this->shader_variant_,
       num_instances](FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
        auto world_to_ndc = frame_graph.get(i_world_to_ndc);
        auto shapes       = frame_graph.get(i_shapes);
        auto transforms   = frame_graph.get(i_transforms);
        auto materials    = frame_graph.get(i_materials);

        auto params = SdfPassParams{
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

        passes.sdf->encode(enc, params, shader_variant);
      });
  }
};

struct QuadEncoder
{
  template <typename Material>
  struct Item
  {
    Framebuffer         framebuffer;
    Option<PassStencil> stencil;
    RectU               scissor;
    gpu::Viewport       viewport;
    gpu::DescriptorSet  samplers;
    gpu::DescriptorSet  textures;
    f32x4x4             world_to_ndc;
    f32x4x4             quad;
    f32x4x4             transform;
    Material            material;
    ShaderVariantId     shader_variant;
  };

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
  explicit QuadEncoder(Allocator allocator, Item<Material> const & item) :
    framebuffer_{item.framebuffer},
    stencil_{item.stencil},
    scissor_{item.scissor},
    viewport_{item.viewport},
    samplers_{item.samplers},
    textures_{item.textures},
    world_to_ndc_{item.world_to_ndc},
    quads_{allocator},
    transforms_{allocator},
    materials_{allocator},
    shader_variant_{item.shader_variant}
  {
    push_(item.quad, item.transform, item.material);
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
  bool push(Item<Material> const & item)
  {
    auto mergeable =
      obj::byte_eq(Tuple{framebuffer_, stencil_, scissor_, viewport_, samplers_,
                         textures_, world_to_ndc_, shader_variant_},
                   Tuple{item.framebuffer, item.stencil, item.scissor,
                         item.viewport, item.samplers, item.textures,
                         item.world_to_ndc, item.shader_variant});

    if (!mergeable)
    {
      return false;
    }

    quads_.push(item.quad).unwrap();
    transforms_.push(item.transform).unwrap();
    materials_.extend(Span{&item.material, 1}.as_u8()).unwrap();

    return true;
  }

  void pass(FrameGraph & frame_graph, PassBundle & passes)
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
      [&passes, framebuffer = this->framebuffer_, stencil = this->stencil_,
       scissor = this->scissor_, viewport = this->viewport_,
       samplers = this->samplers_, textures = this->textures_, i_world_to_ndc,
       i_quads, i_transforms, i_materials, num_instances = size32(quads),
       shader_variant = this->shader_variant_](FrameGraph & frame_graph,
                                               gpu::CommandEncoder & enc) {
        auto world_to_ndc = frame_graph.get(i_world_to_ndc);
        auto quads        = frame_graph.get(i_quads);
        auto transforms   = frame_graph.get(i_transforms);
        auto materials    = frame_graph.get(i_materials);

        auto params = QuadPassParams{
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

        passes.quad->encode(enc, params, shader_variant);
      });
  }
};

struct NgonEncoder
{
  template <typename Material>
  struct Item
  {
    Framebuffer         framebuffer;
    Option<PassStencil> stencil;
    RectU               scissor;
    gpu::Viewport       viewport;
    gpu::DescriptorSet  samplers;
    gpu::DescriptorSet  textures;
    f32x4x4             world_to_ndc;
    f32x4x4             transform;
    Span<f32x2 const>   vertices;
    Span<u32 const>     indices;
    Material            material;
    ShaderVariantId     shader_variant;
  };

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
  explicit NgonEncoder(Allocator allocator, Item<Material> const & item) :
    framebuffer_{item.framebuffer},
    stencil_{item.stencil},
    scissor_{item.scissor},
    viewport_{item.viewport},
    samplers_{item.samplers},
    textures_{item.textures},
    world_to_ndc_{item.world_to_ndc},
    index_counts_{allocator},
    transforms_{allocator},
    vertices_{allocator},
    indices_{allocator},
    materials_{allocator},
    shader_variant_{item.shader_variant}
  {
    push_(item.transform, item.vertices, item.indices, item.material);
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
  bool push(Item<Material> const & item)
  {
    auto mergeable =
      obj::byte_eq(Tuple{framebuffer_, stencil_, scissor_, viewport_, samplers_,
                         textures_, world_to_ndc_, shader_variant_},
                   Tuple{item.framebuffer, item.stencil, item.scissor,
                         item.viewport, item.samplers, item.textures,
                         item.world_to_ndc, item.shader_variant});

    if (!mergeable)
    {
      return false;
    }

    push_(item.transform, item.vertices, item.indices, item.material);

    return true;
  }

  void pass(FrameGraph & frame_graph, PassBundle & passes) const
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

    frame_graph.add_pass(
      "Ngon"_str,
      [&passes, framebuffer = framebuffer_, stencil = this->stencil_,
       scissor = this->scissor_, viewport = this->viewport_,
       samplers = this->samplers_, textures = this->textures_, i_world_to_ndc,
       i_transforms, i_vertices, i_indices, i_materials, i_index_counts,
       shader_variant = this->shader_variant_](FrameGraph & frame_graph,
                                               gpu::CommandEncoder & enc) {
        auto world_to_ndc = frame_graph.get(i_world_to_ndc);
        auto transforms   = frame_graph.get(i_transforms);
        auto vertices     = frame_graph.get(i_vertices);
        auto indices      = frame_graph.get(i_indices);
        auto materials    = frame_graph.get(i_materials);
        auto index_counts = frame_graph.get<u32>(i_index_counts);

        auto params = NgonPassParams{.framebuffer    = framebuffer,
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

        passes.ngon->encode(enc, params, shader_variant);
      });
  }
};

struct FillStencilEncoder
{
  struct Item
  {
    DepthStencilTexture stencil;
    u32                 write_mask;
    RectU               scissor;
    gpu::Viewport       viewport;
    FillRule            fill_rule;
    bool                invert;
    f32x4x4             world_to_ndc;
    f32x4x4             transform;
    Span<f32x2 const>   vertices;
    Span<u32 const>     indices;
    Span<u32 const>     index_counts;
  };

  DepthStencilTexture stencil_;

  u32 write_mask_;

  RectU scissor_;

  gpu::Viewport viewport_;

  FillRule fill_rule_;

  bool invert_;

  f32x4x4 world_to_ndc_;

  Vec<f32x4x4> transforms_;

  Vec<f32x2> vertices_;

  Vec<u32> indices_;

  Vec<u32> index_counts_;

  explicit FillStencilEncoder(Allocator allocator, Item const & item) :
    stencil_{item.stencil},
    write_mask_{item.write_mask},
    scissor_{item.scissor},
    viewport_{item.viewport},
    fill_rule_{item.fill_rule},
    invert_{item.invert},
    world_to_ndc_{item.world_to_ndc},
    transforms_{allocator},
    vertices_{allocator},
    indices_{allocator},
    index_counts_{allocator}
  {
    push_(item.transform, item.vertices, item.indices, item.index_counts);
  }

  FillStencilEncoder(FillStencilEncoder const &)             = delete;
  FillStencilEncoder(FillStencilEncoder &&)                  = default;
  FillStencilEncoder & operator=(FillStencilEncoder const &) = delete;
  FillStencilEncoder & operator=(FillStencilEncoder &&)      = default;
  ~FillStencilEncoder()                                      = default;

  void push_(f32x4x4 const & transform, Span<f32x2 const> vertices,
             Span<u32 const> indices, Span<u32 const> index_counts)
  {
    transforms_.push(transform).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices).unwrap();
    index_counts_.extend(index_counts).unwrap();
  }

  bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(
      Tuple{stencil_, write_mask_, scissor_, viewport_, fill_rule_, invert_,
            world_to_ndc_},
      Tuple{item.stencil, item.write_mask, item.scissor, item.viewport,
            item.fill_rule, item.invert, item.world_to_ndc});

    if (!mergeable)
    {
      return false;
    }

    push_(item.transform, item.vertices, item.indices, item.index_counts);

    return true;
  }

  void pass(FrameGraph & frame_graph, PassBundle & passes) const
  {
    auto index_counts = index_counts_.view();
    auto transforms   = transforms_.view();
    auto vertices     = vertices_.view();
    auto indices      = indices_.view();

    auto i_world_to_ndc = frame_graph.push_ssbo(span({world_to_ndc_}));
    auto i_transforms   = frame_graph.push_ssbo(transforms);
    auto i_vertices     = frame_graph.push_ssbo(vertices);
    auto i_indices      = frame_graph.push_ssbo(indices);
    auto i_index_counts = frame_graph.push_buffer(index_counts);

    frame_graph.add_pass(
      "FillStencil"_str,
      [&passes, stencil = stencil_, write_mask = this->write_mask_,
       scissor = this->scissor_, viewport = this->viewport_,
       fill_rule = this->fill_rule_, invert = this->invert_, i_world_to_ndc,
       i_transforms, i_vertices, i_indices,
       i_index_counts](FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
        auto world_to_ndc = frame_graph.get(i_world_to_ndc);
        auto transforms   = frame_graph.get(i_transforms);
        auto vertices     = frame_graph.get(i_vertices);
        auto indices      = frame_graph.get(i_indices);
        auto index_counts = frame_graph.get<u32>(i_index_counts);

        auto params = FillStencilPassParams{.stencil        = stencil,
                                            .write_mask     = write_mask,
                                            .scissor        = scissor,
                                            .viewport       = viewport,
                                            .fill_rule      = fill_rule,
                                            .invert         = invert,
                                            .world_to_ndc   = world_to_ndc,
                                            .transforms     = transforms,
                                            .vertices       = vertices,
                                            .indices        = indices,
                                            .first_instance = 0,
                                            .index_counts   = index_counts};

        passes.fill_stencil->encode(enc, params);
      });
  }
};

struct BezierStencilEncoder
{
  struct Item
  {
    DepthStencilTexture               stencil;
    u32                               write_mask;
    RectU                             scissor;
    gpu::Viewport                     viewport;
    FillRule                          fill_rule;
    bool                              invert;
    f32x4x4                           world_to_ndc;
    f32x4x4                           transform;
    Span<f32x2 const>                 vertices;
    Span<u32 const>                   indices;
    Span<shader::BezierRegions const> regions;
    Span<u32 const>                   region_index_counts;
  };

  DepthStencilTexture stencil_;

  u32 write_mask_;

  RectU scissor_;

  gpu::Viewport viewport_;

  FillRule fill_rule_;

  bool invert_;

  f32x4x4 world_to_ndc_;

  Vec<f32x4x4> transforms_;

  Vec<f32x2> vertices_;

  Vec<u32> indices_;

  Vec<shader::BezierRegions> regions_;

  Vec<u32> region_index_counts_;

  explicit BezierStencilEncoder(Allocator allocator, Item const & item) :
    stencil_{item.stencil},
    write_mask_{item.write_mask},
    scissor_{item.scissor},
    viewport_{item.viewport},
    fill_rule_{item.fill_rule},
    invert_{item.invert},
    world_to_ndc_{item.world_to_ndc},
    transforms_{allocator},
    vertices_{allocator},
    indices_{allocator},
    regions_{allocator},
    region_index_counts_{allocator}
  {
    push_(item.transform, item.vertices, item.indices, item.regions,
          item.region_index_counts);
  }

  BezierStencilEncoder(BezierStencilEncoder const &)             = delete;
  BezierStencilEncoder(BezierStencilEncoder &&)                  = default;
  BezierStencilEncoder & operator=(BezierStencilEncoder const &) = delete;
  BezierStencilEncoder & operator=(BezierStencilEncoder &&)      = default;
  ~BezierStencilEncoder()                                        = default;

  void push_(f32x4x4 const & transform, Span<f32x2 const> vertices,
             Span<u32 const> indices, Span<shader::BezierRegions const> regions,
             Span<u32 const> region_index_counts)
  {
    transforms_.push(transform).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices).unwrap();
    regions_.extend(regions).unwrap();
    region_index_counts_.extend(region_index_counts).unwrap();
  }

  bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(
      Tuple{stencil_, write_mask_, scissor_, viewport_, fill_rule_, invert_,
            world_to_ndc_},
      Tuple{item.stencil, item.write_mask, item.scissor, item.viewport,
            item.fill_rule, item.invert, item.world_to_ndc});

    if (!mergeable)
    {
      return false;
    }

    push_(item.transform, item.vertices, item.indices, item.regions,
          item.region_index_counts);

    return true;
  }

  void pass(FrameGraph & frame_graph, PassBundle & passes) const
  {
    auto vertices            = vertices_.view();
    auto indices             = indices_.view();
    auto transforms          = transforms_.view();
    auto regions             = regions_.view();
    auto region_index_counts = region_index_counts_.view();

    auto i_vertices            = frame_graph.push_ssbo(vertices);
    auto i_indices             = frame_graph.push_ssbo(indices);
    auto i_world_to_ndc        = frame_graph.push_ssbo(span({world_to_ndc_}));
    auto i_transforms          = frame_graph.push_ssbo(transforms);
    auto i_regions             = frame_graph.push_ssbo(regions);
    auto i_region_index_counts = frame_graph.push_buffer(region_index_counts);

    frame_graph.add_pass(
      "Bezier Stencil"_str,
      [&passes, stencil = this->stencil_, write_mask = this->write_mask_,
       scissor = this->scissor_, viewport = this->viewport_,
       fill_rule = this->fill_rule_, invert = this->invert_, i_vertices,
       i_indices, i_world_to_ndc, i_transforms, i_regions,
       i_region_index_counts](FrameGraph &          frame_graph,
                              gpu::CommandEncoder & enc) {
        auto vertices            = frame_graph.get(i_vertices);
        auto indices             = frame_graph.get(i_indices);
        auto world_to_ndc        = frame_graph.get(i_world_to_ndc);
        auto transforms          = frame_graph.get(i_transforms);
        auto regions             = frame_graph.get(i_regions);
        auto region_index_counts = frame_graph.get<u32>(i_region_index_counts);

        auto params =
          BezierStencilPassParams{.stencil             = stencil,
                                  .write_mask          = write_mask,
                                  .scissor             = scissor,
                                  .viewport            = viewport,
                                  .fill_rule           = fill_rule,
                                  .invert              = invert,
                                  .world_to_ndc        = world_to_ndc,
                                  .transforms          = transforms,
                                  .vertices            = vertices,
                                  .indices             = indices,
                                  .regions             = regions,
                                  .region_index_counts = region_index_counts};

        passes.bezier_stencil->encode(enc, params);
      });
  }
};

struct PbrEncoder
{
  template <typename World, typename Material, typename Light>
  struct Item
  {
    Framebuffer         framebuffer;
    Option<PassStencil> stencil;
    RectU               scissor;
    gpu::Viewport       viewport;
    gpu::PolygonMode    polygon_mode;
    gpu::DescriptorSet  samplers;
    gpu::DescriptorSet  textures;
    StructBufferSpan    vertices;
    StructBufferSpan    indices;
    u32                 num_indices;
    World               world;
    Material            material;
    Span<Light>         lights;
    ShaderVariantId     shader_variant;
  };

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
  explicit PbrEncoder(Allocator                         allocator,
                      Item<World, Material, Light> const & item) :
    framebuffer_{item.framebuffer},
    stencil_{item.stencil},
    scissor_{item.scissor},
    viewport_{item.viewport},
    polygon_mode_{item.polygon_mode},
    samplers_{item.samplers},
    textures_{item.textures},
    vertices_{item.vertices},
    indices_{item.indices},
    num_indices_{item.num_indices},
    world_{allocator},
    material_{allocator},
    lights_{allocator},
    shader_variant_{item.shader_variant}
  {
    world_.extend(Span{&item.world, 1}.as_u8()).unwrap();
    material_.extend(Span{&item.material, 1}.as_u8()).unwrap();
    lights_.extend(item.lights.as_u8()).unwrap();
  }

  PbrEncoder(PbrEncoder const &)             = delete;
  PbrEncoder(PbrEncoder &&)                  = default;
  PbrEncoder & operator=(PbrEncoder const &) = delete;
  PbrEncoder & operator=(PbrEncoder &&)      = default;
  ~PbrEncoder()                              = default;

  void pass(FrameGraph & frame_graph, PassBundle & passes)
  {
    auto i_world    = frame_graph.push_ssbo(world_.view());
    auto i_material = frame_graph.push_ssbo(material_.view());
    auto i_lights   = frame_graph.push_ssbo(lights_.view());

    frame_graph.add_pass(
      "PBR"_str,
      [&passes, framebuffer = framebuffer_, stencil = this->stencil_,
       scissor = this->scissor_, polygon_mode = this->polygon_mode_,
       viewport = this->viewport_, samplers = this->samplers_,
       textures = this->textures_, vertices = this->vertices_,
       indices = this->indices_, num_indices = this->num_indices_, i_world,
       i_material, i_lights, shader_variant = this->shader_variant_](
        FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
        auto world    = frame_graph.get(i_world);
        auto material = frame_graph.get(i_material);
        auto lights   = frame_graph.get(i_lights);

        auto params = PBRPassParams{.framebuffer  = framebuffer,
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

        passes.pbr->encode(enc, params, shader_variant);
      });
  }
};

}    // namespace ash
