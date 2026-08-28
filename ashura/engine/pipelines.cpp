/// SPDX-License-Identifier: MIT
#include "ashura/engine/pipelines.hpp"
#include "ashura/engine/shader_system.hpp"
#include "ashura/engine/systems.hpp"
#include "ashura/std/math.hpp"
#include "ashura/std/range.hpp"
#include "ashura/std/sformat.hpp"

namespace ash
{

BezierStencilPipeline::BezierStencilPipeline(Allocator)
{
}

Str BezierStencilPipeline::label()
{
    return "BezierStencil"_s;
}

void BezierStencilPipeline::acquire(GpuFramePlan plan, Allocator)
{
    auto & gpu = *plan->sys();

    auto shader = sys.shader->get("defaults/bezier_stencil"_s).unwrap().shader;

    auto raster_state =
      gpu::RasterizationState{.depth_clamp_enable = false,
                              .polygon_mode       = gpu::PolygonMode::Fill,
                              .cull_mode          = gpu::CullMode::None,
                              .front_face         = gpu::FrontFace::CounterClockWise,
                              .depth_bias_enable  = false,
                              .depth_bias_constant_factor = 0,
                              .depth_bias_clamp           = 0,
                              .depth_bias_slope_factor    = 0,
                              .sample_count               = gpu.sample_count()};

    auto depth_stencil_state =
      gpu::DepthStencilState{.depth_test_enable        = false,
                             .depth_write_enable       = false,
                             .depth_compare_op         = gpu::CompareOp::Never,
                             .depth_bounds_test_enable = false,
                             .stencil_test_enable      = false,
                             .front_stencil            = {},
                             .back_stencil             = {},
                             .min_depth_bounds         = 0,
                             .max_depth_bounds         = 0};

    auto pipeline_info = gpu::GraphicsPipelineInfo{
      .label           = "Bezier Stencil Graphics Pipeline"_s,
      .vertex_shader   = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "vert"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .fragment_shader = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "frag"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .color_formats   = {},
      .depth_format    = {},
      .stencil_format  = gpu.depth_stencil_format(),
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(shader::BezierStencilShaderParams),
      .descriptor_set_layouts = {},
      .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = {},
      .cache                  = gpu.pipeline_cache()
    };

    pipeline_ = gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

void BezierStencilPipeline::encode(gpu::CommandEncoder                 e,
                                   BezierStencilPipelineParams const & params)
{
    auto info = gpu::RenderingInfo{.render_area        = params.render_area,
                                   .num_layers         = 1,
                                   .color_attachments  = {},
                                   .depth_attachment   = {},
                                   .stencil_attachment = params.stencil_attachment};

    // TODO: deferring of begin rendering or allowing store and load and clear value
    // spec
    e->begin_rendering(info);
    e->bind_graphics_pipeline(pipeline_);
    e->push_constants(as_u8_span(params.params));

    ASH_CHECK(size32(params.states) > 0, "");
    ASH_CHECK(size32(params.state_runs) == (size32(params.states) + 1), "");
    ASH_CHECK(size32(params.index_runs) > 1, "");
    auto num_states = size32(params.states);

    for (auto s : range(num_states))
    {
        auto & state = params.states[s];

        auto [front_stencil, back_stencil] =
          fill_rule_stencil(state.fill_rule, state.invert, state.write_mask);

        e->set_graphics_state(gpu::GraphicsState{.scissor             = state.scissor,
                                                 .viewport            = state.viewport,
                                                 .stencil_test_enable = false,
                                                 .front_face_stencil  = front_stencil,
                                                 .back_face_stencil   = back_stencil,
                                                 .front_face = state.front_face});

        for (auto i :
             range(Slice32::offsets(params.state_runs[s], params.state_runs[s + 1])))
        {
            e->draw(Slice32::offsets(params.index_runs[i], params.index_runs[i + 1]),
                    {i, 1});
        }
    }

    e->end_rendering();
}

void BezierStencilPipeline::release(GpuFramePlan plan, Allocator)
{
    plan->add_preframe_task(
      [p = pipeline_, d = plan->device()](GpuFrame) { d->uninit(p); });
}

BloomPipeline::BloomPipeline(Allocator)
{
}

Str BloomPipeline::label()
{
    return "Bloom"_s;
}

void BloomPipeline::acquire(GpuFramePlan, Allocator)
{
}

void BloomPipeline::release(GpuFramePlan, Allocator)
{
}

void BloomPipeline::encode(gpu::CommandEncoder, BloomPipelineParams const &)
{
    /// E' = Blur(E)
    /// D' = Blur(D) + E'
    /// C' = Blur(C) + D'
    /// B' = Blur(B) + C'
    /// A' = Blur(A) + B'
}

// https://www.youtube.com/watch?v=ml-5OGZC7vE
//
// An investigation of fast real-time GPU-based image blur algorithms -
// https://www.intel.cn/content/www/cn/zh/developer/articles/technical/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms.html

//
// Algorithm described here:
// https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_slides.pdf
//

BlurPipeline::BlurPipeline(Allocator)
{
}

Str BlurPipeline::label()
{
    return "Blur"_s;
}

static gpu::GraphicsPipeline create_blur_pipeline(GpuFramePlan plan, Str label,
                                                  gpu::Shader shader, Allocator,
                                                  Allocator   scratch)
{
    auto & gpu = *plan->sys();

    auto tagged_label =
      sformat(scratch, "Blur Graphics Pipeline: {}"_s, label).unwrap();

    auto raster_state =
      gpu::RasterizationState{.depth_clamp_enable = false,
                              .polygon_mode       = gpu::PolygonMode::Fill,
                              .cull_mode          = gpu::CullMode::None,
                              .front_face         = gpu::FrontFace::CounterClockWise,
                              .depth_bias_enable  = false,
                              .depth_bias_constant_factor = 0,
                              .depth_bias_clamp           = 0,
                              .depth_bias_slope_factor    = 0,
                              .sample_count               = gpu::SampleCount::C1};

    auto depth_stencil_state =
      gpu::DepthStencilState{.depth_test_enable        = false,
                             .depth_write_enable       = false,
                             .depth_compare_op         = gpu::CompareOp::Never,
                             .depth_bounds_test_enable = false,
                             .stencil_test_enable      = false,
                             .front_stencil            = {},
                             .back_stencil             = {},
                             .min_depth_bounds         = 0,
                             .max_depth_bounds         = 0};

    gpu::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = false,
       .src_color_blend_factor = gpu::BlendFactor::Zero,
       .dst_color_blend_factor = gpu::BlendFactor::Zero,
       .color_blend_op         = gpu::BlendOp::Add,
       .src_alpha_blend_factor = gpu::BlendFactor::Zero,
       .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
       .alpha_blend_op         = gpu::BlendOp::Add,
       .color_write_mask       = gpu::ColorComponents::All}
    };

    auto color_blend_state =
      gpu::ColorBlendState{.attachments = attachment_states, .blend_constant = {}};

    auto & layout = gpu.descriptors_layout();

    gpu::DescriptorSetLayout set_layouts[] = {
      layout.samplers,           // 0: samplers
      layout.sampled_textures    // 1: textures
    };

    auto pipeline_info = gpu::GraphicsPipelineInfo{
      .label           = tagged_label,
      .vertex_shader   = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "vert"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .fragment_shader = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "frag"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .color_formats   = span({gpu.color_format()}
          ),
      .depth_format    = {},
      .stencil_format  = gpu.depth_stencil_format(),
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(shader::BlurShaderParams),
      .descriptor_set_layouts = set_layouts,
      .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = gpu.pipeline_cache()
    };

    return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

void BlurPipeline::acquire(GpuFramePlan plan, Allocator allocator)
{
    ThreadScratchScope scratch;

    downsample_pipeline_ = create_blur_pipeline(
      plan, "Downsample"_s,
      sys.shader->get("defaults/blur_downsample"_s).unwrap().shader, allocator,
      scratch);
    upsample_pipeline_ = create_blur_pipeline(
      plan, "Upsample"_s, sys.shader->get("defaults/blur_upsample"_s).unwrap().shader,
      allocator, scratch);
}

void BlurPipeline::release(GpuFramePlan plan, Allocator)
{
    plan->add_preframe_task([p0 = downsample_pipeline_, p1 = upsample_pipeline_,
                             d = plan->device()](GpuFrame) {
        d->uninit(p0);
        d->uninit(p1);
    });
}

void BlurPipeline::encode(gpu::CommandEncoder e, BlurPipelineParams const & params)
{
    InplaceVec<gpu::RenderingAttachment, 1, 0> color;

    color
      .push(gpu::RenderingAttachment{.view         = params.framebuffer.color.view,
                                     .resolve      = nullptr,
                                     .resolve_mode = gpu::ResolveModes::None,
                                     .load_op      = gpu::LoadOp::Load,
                                     .store_op     = gpu::StoreOp::Store,
                                     .clear        = {}})
      .unwrap();

    auto stencil = params.stencil.map([&](PipelineStencil const &) {
        return gpu::RenderingAttachment{
          .view         = params.framebuffer.depth_stencil.v().stencil_view,
          .resolve      = nullptr,
          .resolve_mode = gpu::ResolveModes::None,
          .load_op      = gpu::LoadOp::Load,
          .store_op     = gpu::StoreOp::None,
          .clear        = {}};
    });

    e->begin_rendering(gpu::RenderingInfo{
      .render_area{.offset = {}, .extent = params.framebuffer.extent().xy()},
      .num_layers         = 1,
      .color_attachments  = color,
      .depth_attachment   = {},
      .stencil_attachment = stencil
    });

    e->bind_graphics_pipeline(params.upsample ? upsample_pipeline_ :
                                                downsample_pipeline_);
    e->set_graphics_state(gpu::GraphicsState{
      .scissor             = params.scissor,
      .viewport            = params.viewport,
      .stencil_test_enable = params.stencil.is_some(),
      .front_face_stencil =
        params.stencil.map([](auto s) { return s.front; }).unwrap_or(),
      .back_face_stencil =
        params.stencil.map([](auto s) { return s.back; }).unwrap_or()});
    e->push_constants(as_u8_span(params.params));
    e->bind_descriptor_sets(span({
                              params.samplers,    // 0: samplers
                              params.textures     // 1: textures
                            }),
                            {});
    e->draw({0, 4}, params.instances);
    e->end_rendering();
}

Str FillStencilPipeline::label()
{
    return "FillStencil"_s;
}

FillStencilPipeline::FillStencilPipeline(Allocator)
{
}

void FillStencilPipeline::acquire(GpuFramePlan plan, Allocator allocator)
{
    ThreadScratchScope scratch;

    auto & gpu = *plan->sys();

    auto tagged_label = sformat(scratch, "Fill Stencil Graphics Pipeline"_s).unwrap();

    auto raster_state =
      gpu::RasterizationState{.depth_clamp_enable = false,
                              .polygon_mode       = gpu::PolygonMode::Fill,
                              .cull_mode          = gpu::CullMode::None,
                              .front_face         = gpu::FrontFace::CounterClockWise,
                              .depth_bias_enable  = false,
                              .depth_bias_constant_factor = 0,
                              .depth_bias_clamp           = 0,
                              .depth_bias_slope_factor    = 0,
                              .sample_count               = gpu.sample_count()};

    auto depth_stencil_state =
      gpu::DepthStencilState{.depth_test_enable        = false,
                             .depth_write_enable       = false,
                             .depth_compare_op         = gpu::CompareOp::Never,
                             .depth_bounds_test_enable = false,
                             .stencil_test_enable      = false,
                             .front_stencil            = {},
                             .back_stencil             = {},
                             .min_depth_bounds         = 0,
                             .max_depth_bounds         = 0};

    auto color_blend_state =
      gpu::ColorBlendState{.attachments = {}, .blend_constant = {}};

    auto & layout = gpu.descriptors_layout();

    gpu::DescriptorSetLayout set_layouts[] = {
      layout.read_storage_buffer,    // 0: world_to_ndc
      layout.read_storage_buffer,    // 1: world_transforms
      layout.read_storage_buffer,    // 2: vertices
      layout.read_storage_buffer,    // 3: indices
    };

    auto shader = sys.shader->get("defaults/fill_stencil"_s).unwrap().shader;

    auto pipeline_info = gpu::GraphicsPipelineInfo{
      .label           = tagged_label,
      .vertex_shader   = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "vert"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .fragment_shader = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "frag"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .color_formats   = {},
      .depth_format    = {},
      .stencil_format  = gpu.depth_stencil_format(),
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(shader::FillStencilShaderParams),
      .descriptor_set_layouts = set_layouts,
      .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = gpu.pipeline_cache()
    };

    pipeline_ = gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

void FillStencilPipeline::encode(gpu::CommandEncoder               e,
                                 FillStencilPipelineParams const & params)
{
    auto info = gpu::RenderingInfo{.render_area        = params.render_area,
                                   .num_layers         = 1,
                                   .color_attachments  = {},
                                   .depth_attachment   = {},
                                   .stencil_attachment = params.stencil_attachment};

    e->begin_rendering(info);

    e->bind_graphics_pipeline(pipeline_);
    e->push_constants(as_u8_span(params.params));

    ASH_CHECK(size32(params.states) > 0, "");
    ASH_CHECK(size32(params.state_runs) == (size32(params.states) + 1), "");
    ASH_CHECK(size32(params.index_runs) > 1, "");
    auto num_states = size32(params.states);

    for (auto s : range(num_states))
    {
        auto & state = params.states[s];

        auto [front_stencil, back_stencil] =
          fill_rule_stencil(state.fill_rule, state.invert, state.write_mask);

        e->set_graphics_state(gpu::GraphicsState{.scissor             = state.scissor,
                                                 .viewport            = state.viewport,
                                                 .stencil_test_enable = false,
                                                 .front_face_stencil  = front_stencil,
                                                 .back_face_stencil   = back_stencil,
                                                 .front_face = state.front_face});

        for (auto i :
             range(Slice32::offsets(params.state_runs[s], params.state_runs[s + 1])))
        {
            e->draw(Slice32::offsets(params.index_runs[i], params.index_runs[i + 1]),
                    {i, 1});
        }
    }

    e->end_rendering();
}

void FillStencilPipeline::release(GpuFramePlan plan, Allocator)
{
    plan->add_preframe_task(
      [d = plan->device(), p = pipeline_](GpuFrame) { d->uninit(p); });
}

Str PBRPipeline::label()
{
    return "PBR"_s;
}

static gpu::GraphicsPipeline create_pbr_pipeline(GpuFramePlan plan, Str label,
                                                 gpu::Shader      shader,
                                                 gpu::PolygonMode polygon_mode)
{
    ThreadScratchScope scratch;

    auto & gpu = *plan->sys();

    auto tagged_label = sformat(scratch, "PBR Graphics Pipeline: {}"_s, label).unwrap();

    auto raster_state =
      gpu::RasterizationState{.depth_clamp_enable = false,
                              .polygon_mode       = polygon_mode,
                              .cull_mode          = gpu::CullMode::None,
                              .front_face         = gpu::FrontFace::CounterClockWise,
                              .depth_bias_enable  = false,
                              .depth_bias_constant_factor = 0,
                              .depth_bias_clamp           = 0,
                              .depth_bias_slope_factor    = 0,
                              .sample_count               = gpu.sample_count()};

    auto depth_stencil_state =
      gpu::DepthStencilState{.depth_test_enable        = false,
                             .depth_write_enable       = false,
                             .depth_compare_op         = gpu::CompareOp::Greater,
                             .depth_bounds_test_enable = false,
                             .stencil_test_enable      = false,
                             .front_stencil            = {},
                             .back_stencil             = {},
                             .min_depth_bounds         = 0,
                             .max_depth_bounds         = 1};

    gpu::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = false,
       .src_color_blend_factor = gpu::BlendFactor::Zero,
       .dst_color_blend_factor = gpu::BlendFactor::Zero,
       .color_blend_op         = gpu::BlendOp::Add,
       .src_alpha_blend_factor = gpu::BlendFactor::Zero,
       .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
       .alpha_blend_op         = gpu::BlendOp::Add,
       .color_write_mask       = gpu::ColorComponents::All}
    };

    auto color_blend_state = gpu::ColorBlendState{
      .attachments = attachment_states, .blend_constant = {1, 1, 1, 1}
    };

    auto & layout = gpu.descriptors_layout();

    gpu::DescriptorSetLayout const set_layouts[] = {
      layout.samplers,           // 0: samplers
      layout.sampled_textures    // 1: textures
    };

    auto pipeline_info = gpu::GraphicsPipelineInfo{
      .label           = tagged_label,
      .vertex_shader   = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "vert"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .fragment_shader = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "frag"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .color_formats   = span({gpu.color_format()}
          ),
      .depth_format    = gpu.depth_stencil_format(),
      .stencil_format  = gpu.depth_stencil_format(),
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(shader::PbrShaderParams),
      .descriptor_set_layouts = set_layouts,
      .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = gpu.pipeline_cache()
    };

    return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

PBRPipeline::Pipeline create_pbr_pipeline(GpuFramePlan plan, Str label,
                                          gpu::Shader shader, Allocator allocator)
{
    return {
      .fill  = create_pbr_pipeline(plan, label, shader, gpu::PolygonMode::Fill),
      .line  = create_pbr_pipeline(plan, label, shader, gpu::PolygonMode::Line),
      .point = create_pbr_pipeline(plan, label, shader, gpu::PolygonMode::Point),
    };
}

PBRPipeline::PBRPipeline(Allocator allocator) : variants_{allocator}
{
}

void PBRPipeline::acquire(GpuFramePlan plan, Allocator allocator)
{
    auto id =
      add_variant(plan, "base"_s,
                  sys.shader->get("defaults/pbr_base"_s).unwrap().shader, allocator);
    ASH_CHECK(id == PipelineVariantId::Base, "");
}

PipelineVariantId PBRPipeline::add_variant(GpuFramePlan plan, Str label,
                                           gpu::Shader shader, Allocator allocator)
{
    auto pipeline = create_pbr_pipeline(plan, label, shader, allocator);
    auto id       = (PipelineVariantId) variants_.push(Tuple{label, pipeline}).unwrap();
    ASH_CHECK(id == PipelineVariantId::Base, "");
    return id;
}

void PBRPipeline::remove_variant(GpuFramePlan plan, PipelineVariantId id)
{
    auto pipeline = variants_[id].v0.v1;

    variants_.erase(id);

    plan->add_preframe_task([p = pipeline, d = plan->device()](GpuFrame) {
        d->uninit(p.fill);
        d->uninit(p.line);
        d->uninit(p.point);
    });
}

void PBRPipeline::encode(gpu::CommandEncoder e, PBRPipelineParams const & params)
{
    InplaceVec<gpu::RenderingAttachment, 1, 0> color;

    params.framebuffer.color_msaa.match(
      [&](ColorMsaaImage const & tex) {
          color
            .push(gpu::RenderingAttachment{.view    = tex.view,
                                           .resolve = params.framebuffer.color.view,
                                           .resolve_mode = gpu::ResolveModes::Average,
                                           .load_op      = gpu::LoadOp::Load,
                                           .store_op     = gpu::StoreOp::Store,
                                           .clear        = {}})
            .unwrap();
      },
      [&]() {
          color
            .push(gpu::RenderingAttachment{.view    = params.framebuffer.color.view,
                                           .resolve = nullptr,
                                           .resolve_mode = gpu::ResolveModes::None,
                                           .load_op      = gpu::LoadOp::Load,
                                           .store_op     = gpu::StoreOp::Store,
                                           .clear        = {}})
            .unwrap();
      });

    auto depth = params.framebuffer.depth_stencil.map([](auto & s) {
        return gpu::RenderingAttachment{.view         = s.depth_view,
                                        .resolve      = nullptr,
                                        .resolve_mode = gpu::ResolveModes::None,
                                        .load_op      = gpu::LoadOp::Load,
                                        .store_op     = gpu::StoreOp::Store,
                                        .clear        = {}};
    });

    auto stencil = params.framebuffer.depth_stencil.map([&](auto & s) {
        return gpu::RenderingAttachment{.view         = s.stencil_view,
                                        .resolve      = nullptr,
                                        .resolve_mode = gpu::ResolveModes::None,
                                        .load_op      = gpu::LoadOp::Load,
                                        .store_op     = gpu::StoreOp::None,
                                        .clear        = {}};
    });

    auto info =
      gpu::RenderingInfo{.render_area{.extent = params.framebuffer.extent().xy()},
                         .num_layers         = 1,
                         .color_attachments  = color,
                         .depth_attachment   = depth,
                         .stencil_attachment = stencil};

    e->begin_rendering(info);

    auto pipelines = variants_[params.variant].v0.v1;

    auto pipeline = pipelines.fill;

    switch (params.polygon_mode)
    {
        case gpu::PolygonMode::Fill:
            pipeline = pipelines.fill;
            break;
        case gpu::PolygonMode::Line:
            pipeline = pipelines.line;
            break;
        case gpu::PolygonMode::Point:
            pipeline = pipelines.point;
            break;
    }

    e->bind_graphics_pipeline(pipeline);

    e->set_graphics_state(gpu::GraphicsState{
      .scissor             = params.scissor,
      .viewport            = params.viewport,
      .blend_constant      = {1, 1, 1, 1},
      .stencil_test_enable = params.stencil.is_some(),
      .front_face_stencil =
        params.stencil.map([](auto s) { return s.front; }
             ).unwrap_or(),
      .back_face_stencil =
        params.stencil.map([](auto s) { return s.back; }
             ).unwrap_or(),
      .cull_mode                = params.cull_mode,
      .front_face               = params.front_face,
      .depth_test_enable        = true,
      .depth_compare_op         = gpu::CompareOp::Less,
      .depth_write_enable       = true,
      .depth_bounds_test_enable = false
    });
    e->push_constants(as_u8_span(params.params));
    e->bind_descriptor_sets(span({
                              params.samplers,    // 0: samplers
                              params.textures     // 1: textures
                            }),
                            {});
    e->draw({0, params.num_indices}, {0, 1});
    e->end_rendering();
}

void PBRPipeline::release(GpuFramePlan plan, Allocator)
{
    for (auto [v] : variants_)
    {
        plan->add_preframe_task([p = v.v1, d = plan->device()](GpuFrame) {
            d->uninit(p.fill);
            d->uninit(p.line);
            d->uninit(p.point);
        });
    }
}

QuadPipeline::QuadPipeline(Allocator allocator) : variants_{allocator}
{
}

Str QuadPipeline::label()
{
    return "Quad"_s;
}

static gpu::GraphicsPipeline create_quad_pipeline(GpuFramePlan plan, Str label,
                                                  gpu::Shader shader)
{
    ThreadScratchScope scratch;

    auto & gpu = *plan->sys();

    auto raster_state =
      gpu::RasterizationState{.depth_clamp_enable = false,
                              .polygon_mode       = gpu::PolygonMode::Fill,
                              .cull_mode          = gpu::CullMode::None,
                              .front_face         = gpu::FrontFace::CounterClockWise,
                              .depth_bias_enable  = false,
                              .depth_bias_constant_factor = 0,
                              .depth_bias_clamp           = 0,
                              .depth_bias_slope_factor    = 0,
                              .sample_count               = gpu.sample_count()};

    auto depth_stencil_state =
      gpu::DepthStencilState{.depth_test_enable        = false,
                             .depth_write_enable       = false,
                             .depth_compare_op         = gpu::CompareOp::Never,
                             .depth_bounds_test_enable = false,
                             .stencil_test_enable      = false,
                             .front_stencil            = {},
                             .back_stencil             = {},
                             .min_depth_bounds         = 0,
                             .max_depth_bounds         = 0};

    gpu::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = true,
       .src_color_blend_factor = gpu::BlendFactor::SrcAlpha,
       .dst_color_blend_factor = gpu::BlendFactor::OneMinusSrcAlpha,
       .color_blend_op         = gpu::BlendOp::Add,
       .src_alpha_blend_factor = gpu::BlendFactor::One,
       .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
       .alpha_blend_op         = gpu::BlendOp::Add,
       .color_write_mask       = gpu::ColorComponents::All}
    };

    auto color_blend_state = gpu::ColorBlendState{
      .attachments = attachment_states, .blend_constant = {1, 1, 1, 1}
    };

    auto & layout = gpu.descriptors_layout();

    gpu::DescriptorSetLayout set_layouts[] = {
      layout.samplers,           // 0: samplers
      layout.sampled_textures    // 1: textures
    };

    auto tagged_label =
      sformat(scratch, "Quad Graphics Pipeline: {}"_s, label).unwrap();

    auto pipeline_info = gpu::GraphicsPipelineInfo{
      .label           = tagged_label,
      .vertex_shader   = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "vert"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .fragment_shader = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "frag"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .color_formats   = span({gpu.color_format()}
          ),
      .depth_format    = {},
      .stencil_format  = gpu.depth_stencil_format(),
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(shader::QuadShaderParams),
      .descriptor_set_layouts = set_layouts,
      .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = gpu.pipeline_cache()
    };

    return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

void QuadPipeline::acquire(GpuFramePlan plan, Allocator allocator)
{
    auto id =
      add_variant(plan, "base"_s,
                  sys.shader->get("defaults/quad_base"_s).unwrap().shader, allocator);
    ASH_CHECK(id == PipelineVariantId::Base, "");
}

PipelineVariantId QuadPipeline::add_variant(GpuFramePlan plan, Str label,
                                            gpu::Shader shader, Allocator allocator)
{
    auto pipeline = create_quad_pipeline(plan, label, shader);
    auto id       = variants_.push(Tuple{label, pipeline}).unwrap();
    return (PipelineVariantId) id;
}

void QuadPipeline::remove_variant(GpuFramePlan plan, PipelineVariantId id)
{
    auto pipeline = variants_[id].v0.v1;
    variants_.erase(id);
    plan->add_preframe_task(
      [p = pipeline, d = plan->device()](GpuFrame) { d->uninit(p); });
}

void QuadPipeline::encode(gpu::CommandEncoder e, QuadPipelineParams const & params)
{
    InplaceVec<gpu::RenderingAttachment, 1, 0> color;

    params.framebuffer.color_msaa.match(
      [&](ColorMsaaImage const & tex) {
          color
            .push(gpu::RenderingAttachment{.view    = tex.view,
                                           .resolve = params.framebuffer.color.view,
                                           .resolve_mode = gpu::ResolveModes::Average,
                                           .load_op      = gpu::LoadOp::Load,
                                           .store_op     = gpu::StoreOp::Store,
                                           .clear        = {}})
            .unwrap();
      },
      [&]() {
          color
            .push(gpu::RenderingAttachment{.view    = params.framebuffer.color.view,
                                           .resolve = nullptr,
                                           .resolve_mode = gpu::ResolveModes::None,
                                           .load_op      = gpu::LoadOp::Load,
                                           .store_op     = gpu::StoreOp::Store,
                                           .clear        = {}})
            .unwrap();
      });

    auto stencil = params.framebuffer.depth_stencil.map([&](auto & s) {
        return gpu::RenderingAttachment{.view         = s.stencil_view,
                                        .resolve      = nullptr,
                                        .resolve_mode = gpu::ResolveModes::None,
                                        .load_op      = gpu::LoadOp::Load,
                                        .store_op     = gpu::StoreOp::None,
                                        .clear        = {}};
    });

    auto info =
      gpu::RenderingInfo{.render_area{.extent = params.framebuffer.extent().xy()},
                         .num_layers         = 1,
                         .color_attachments  = color,
                         .depth_attachment   = {},
                         .stencil_attachment = stencil};

    auto pipeline = variants_[params.variant].v0.v1;

    e->begin_rendering(info);
    e->bind_graphics_pipeline(pipeline);
    e->push_constants(as_u8_span(params.params));
    e->bind_descriptor_sets(span({
                              params.samplers,    // 0: samplers
                              params.textures     // 1: textures
                            }),
                            {});

    ASH_CHECK(size32(params.states) > 0, "");
    ASH_CHECK(size32(params.state_runs) == (size32(params.states) + 1), "");
    auto num_states = size32(params.states);

    for (auto s : range(num_states))
    {
        auto & state = params.states[s];

        e->set_graphics_state(gpu::GraphicsState{
          .scissor             = state.scissor,
          .viewport            = state.viewport,
          .stencil_test_enable = state.stencil.is_some(),
          .front_face_stencil =
            state.stencil.map([](auto s) { return s.front; }).unwrap_or(),
          .back_face_stencil =
            state.stencil.map([](auto s) { return s.back; }).unwrap_or()});

        e->draw({0, 4},
                Slice32::offsets(params.state_runs[s], params.state_runs[s + 1]));
    }

    e->end_rendering();
}

void QuadPipeline::release(GpuFramePlan plan, Allocator)
{
    for (auto [v] : variants_)
    {
        plan->add_preframe_task(
          [d = plan->device(), p = v.v1](GpuFrame) { d->uninit(p); });
    }
}

SdfPipeline::SdfPipeline(Allocator allocator) : variants_{allocator}
{
}

Str SdfPipeline::label()
{
    return "SDF"_s;
}

static gpu::GraphicsPipeline create_sdf_pipeline(GpuFramePlan plan, Str label,
                                                 gpu::Shader shader)
{
    ThreadScratchScope scratch;

    auto & gpu = *plan->sys();

    auto raster_state =
      gpu::RasterizationState{.depth_clamp_enable = false,
                              .polygon_mode       = gpu::PolygonMode::Fill,
                              .cull_mode          = gpu::CullMode::None,
                              .front_face         = gpu::FrontFace::CounterClockWise,
                              .depth_bias_enable  = false,
                              .depth_bias_constant_factor = 0,
                              .depth_bias_clamp           = 0,
                              .depth_bias_slope_factor    = 0,
                              .sample_count               = gpu.sample_count()};

    auto depth_stencil_state =
      gpu::DepthStencilState{.depth_test_enable        = false,
                             .depth_write_enable       = false,
                             .depth_compare_op         = gpu::CompareOp::Never,
                             .depth_bounds_test_enable = false,
                             .stencil_test_enable      = false,
                             .front_stencil            = {},
                             .back_stencil             = {},
                             .min_depth_bounds         = 0,
                             .max_depth_bounds         = 0};

    gpu::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = true,
       .src_color_blend_factor = gpu::BlendFactor::SrcAlpha,
       .dst_color_blend_factor = gpu::BlendFactor::OneMinusSrcAlpha,
       .color_blend_op         = gpu::BlendOp::Add,
       .src_alpha_blend_factor = gpu::BlendFactor::One,
       .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
       .alpha_blend_op         = gpu::BlendOp::Add,
       .color_write_mask       = gpu::ColorComponents::All}
    };

    auto color_blend_state = gpu::ColorBlendState{
      .attachments = attachment_states, .blend_constant = {1, 1, 1, 1}
    };

    auto & layout = gpu.descriptors_layout();

    gpu::DescriptorSetLayout set_layouts[] = {
      layout.samplers,           // 0: samplers
      layout.sampled_textures    // 1: textures
    };

    auto tagged_label = sformat(scratch, "SDF Graphics Pipeline: {}"_s, label).unwrap();

    auto pipeline_info = gpu::GraphicsPipelineInfo{
      .label           = tagged_label,
      .vertex_shader   = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "vert"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .fragment_shader = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "frag"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .color_formats   = span({gpu.color_format()}
          ),
      .depth_format    = {},
      .stencil_format  = gpu.depth_stencil_format(),
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(shader::SdfShaderParams),
      .descriptor_set_layouts = set_layouts,
      .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = gpu.pipeline_cache()
    };

    return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

void SdfPipeline::acquire(GpuFramePlan plan, Allocator allocator)
{
    auto gradient_id = add_variant(
      plan, "gradient"_s, sys.shader->get("defaults/sdf_gradient"_s).unwrap().shader,
      allocator);
    ASH_CHECK(gradient_id == GRADIENT, "");
    auto noise_id =
      add_variant(plan, "noise"_s,
                  sys.shader->get("defaults/sdf_noise"_s).unwrap().shader, allocator);
    ASH_CHECK(noise_id == NOISE, "");
    auto mesh_gradient_id = add_variant(
      plan, "mesh_gradient"_s,
      sys.shader->get("defaults/sdf_mesh_gradient"_s).unwrap().shader, allocator);
    ASH_CHECK(mesh_gradient_id == MESH_GRADIENT, "");
}

PipelineVariantId SdfPipeline::add_variant(GpuFramePlan plan, Str label,
                                           gpu::Shader shader, Allocator allocator)
{
    auto pipeline = create_sdf_pipeline(plan, label, shader);
    return variants_.push(Tuple{label, pipeline}).unwrap();
}

void SdfPipeline::remove_variant(GpuFramePlan plan, PipelineVariantId id)
{
    auto pipeline = variants_[id].v0;
    variants_.erase(id);
    plan->add_preframe_task(
      [d = plan->device(), p = pipeline.v1](GpuFrame) { d->uninit(p); });
}

void SdfPipeline::encode(gpu::CommandEncoder e, SdfPipelineParams const & params)
{
    InplaceVec<gpu::RenderingAttachment, 1, 0> color;

    params.framebuffer.color_msaa.match(
      [&](ColorMsaaImage const & tex) {
          color
            .push(gpu::RenderingAttachment{.view    = tex.view,
                                           .resolve = params.framebuffer.color.view,
                                           .resolve_mode = gpu::ResolveModes::Average,
                                           .load_op      = gpu::LoadOp::Load,
                                           .store_op     = gpu::StoreOp::Store,
                                           .clear        = {}})
            .unwrap();
      },
      [&]() {
          color
            .push(gpu::RenderingAttachment{.view    = params.framebuffer.color.view,
                                           .resolve = nullptr,
                                           .resolve_mode = gpu::ResolveModes::None,
                                           .load_op      = gpu::LoadOp::Load,
                                           .store_op     = gpu::StoreOp::Store,
                                           .clear        = {}})
            .unwrap();
      });

    auto stencil = params.framebuffer.depth_stencil.map([&](auto & s) {
        return gpu::RenderingAttachment{.view         = s.stencil_view,
                                        .resolve      = nullptr,
                                        .resolve_mode = gpu::ResolveModes::None,
                                        .load_op      = gpu::LoadOp::Load,
                                        .store_op     = gpu::StoreOp::None,
                                        .clear        = {}};
    });

    auto info =
      gpu::RenderingInfo{.render_area{.extent = params.framebuffer.extent().xy()},
                         .num_layers         = 1,
                         .color_attachments  = color,
                         .depth_attachment   = {},
                         .stencil_attachment = stencil};

    auto pipeline = variants_[params.variant].v0.v1;

    e->begin_rendering(info);
    e->bind_graphics_pipeline(pipeline);
    e->push_constants(as_u8_span(params.params));
    e->bind_descriptor_sets(span({
                              params.samplers,    // 0: samplers
                              params.textures     // 1: textures
                            }),
                            {});

    ASH_CHECK(size32(params.states) > 0, "");
    ASH_CHECK(size32(params.state_runs) == (size32(params.states) + 1), "");
    auto num_states = size32(params.states);

    for (auto s : range(num_states))
    {
        auto & state = params.states[s];

        e->set_graphics_state(gpu::GraphicsState{
          .scissor             = state.scissor,
          .viewport            = state.viewport,
          .stencil_test_enable = state.stencil.is_some(),
          .front_face_stencil =
            state.stencil.map([](auto s) { return s.front; }).unwrap_or(),
          .back_face_stencil =
            state.stencil.map([](auto s) { return s.back; }).unwrap_or()});

        e->draw({0, 4},
                Slice32::offsets(params.state_runs[s], params.state_runs[s + 1]));
    }
    e->end_rendering();
}

void SdfPipeline::release(GpuFramePlan plan, Allocator)
{
    for (auto [v] : variants_)
    {
        plan->add_preframe_task(
          [d = plan->device(), p = v.v1](GpuFrame) { d->uninit(p); });
    }
}

Str TriangleFillPipeline::label()
{
    return "TriangleFill"_s;
}

static gpu::GraphicsPipeline create_triangle_fill_pipeline(GpuFramePlan plan, Str label,
                                                           gpu::Shader shader)
{
    ThreadScratchScope scratch;

    auto & gpu = *plan->sys();

    auto tagged_label =
      sformat(scratch, "TriangleFill Graphics Pipeline: {}"_s, label).unwrap();

    auto raster_state =
      gpu::RasterizationState{.depth_clamp_enable = false,
                              .polygon_mode       = gpu::PolygonMode::Fill,
                              .cull_mode          = gpu::CullMode::None,
                              .front_face         = gpu::FrontFace::CounterClockWise,
                              .depth_bias_enable  = false,
                              .depth_bias_constant_factor = 0,
                              .depth_bias_clamp           = 0,
                              .depth_bias_slope_factor    = 0,
                              .sample_count               = gpu.sample_count()};

    auto depth_stencil_state =
      gpu::DepthStencilState{.depth_test_enable        = false,
                             .depth_write_enable       = false,
                             .depth_compare_op         = gpu::CompareOp::Never,
                             .depth_bounds_test_enable = false,
                             .stencil_test_enable      = false,
                             .front_stencil            = {},
                             .back_stencil             = {},
                             .min_depth_bounds         = 0,
                             .max_depth_bounds         = 0};

    gpu::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = true,
       .src_color_blend_factor = gpu::BlendFactor::SrcAlpha,
       .dst_color_blend_factor = gpu::BlendFactor::OneMinusSrcAlpha,
       .color_blend_op         = gpu::BlendOp::Add,
       .src_alpha_blend_factor = gpu::BlendFactor::One,
       .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
       .alpha_blend_op         = gpu::BlendOp::Add,
       .color_write_mask       = gpu::ColorComponents::All}
    };

    auto color_blend_state = gpu::ColorBlendState{
      .attachments = attachment_states, .blend_constant = {1, 1, 1, 1}
    };

    auto & layout = gpu.descriptors_layout();

    gpu::DescriptorSetLayout set_layouts[] = {
      layout.samplers,           // 0: samplers
      layout.sampled_textures    // 1: textures
    };

    auto pipeline_info = gpu::GraphicsPipelineInfo{
      .label           = tagged_label,
      .vertex_shader   = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "vert"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .fragment_shader = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "frag"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .color_formats   = span({gpu.color_format()}
          ),
      .depth_format    = {},
      .stencil_format  = gpu.depth_stencil_format(),
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(shader::TriangleFillShaderParams),
      .descriptor_set_layouts = set_layouts,
      .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = gpu.pipeline_cache()
    };

    return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

TriangleFillPipeline::TriangleFillPipeline(Allocator allocator) : pipelines_{allocator}
{
}

void TriangleFillPipeline::acquire(GpuFramePlan plan, Allocator allocator)
{
    ThreadScratchScope scratch;

    auto id = add_variant(plan, "base"_s,
                          sys.shader->get("defaults/triangle_fill"_s).unwrap().shader,
                          allocator);
    ASH_CHECK(id == PipelineVariantId::Base, "");
}

PipelineVariantId TriangleFillPipeline::add_variant(GpuFramePlan plan, Str label,
                                                    gpu::Shader shader,
                                                    Allocator   allocator)
{
    auto pipeline = create_triangle_fill_pipeline(plan, label, shader);
    auto id       = pipelines_.push(Tuple{label, pipeline}).unwrap();
    return (PipelineVariantId) id;
}

void TriangleFillPipeline::remove_variant(GpuFramePlan plan, PipelineVariantId id)
{
    auto pipeline = pipelines_[id];
    pipelines_.erase(id);
    plan->add_preframe_task(
      [p = pipeline.v0, d = plan->device()](GpuFrame) { d->uninit(p.v1); });
}

void TriangleFillPipeline::encode(gpu::CommandEncoder                e,
                                  TriangleFillPipelineParams const & params)
{
    InplaceVec<gpu::RenderingAttachment, 1, 0> color;

    params.framebuffer.color_msaa.match(
      [&](ColorMsaaImage const & tex) {
          color
            .push(gpu::RenderingAttachment{.view    = tex.view,
                                           .resolve = params.framebuffer.color.view,
                                           .resolve_mode = gpu::ResolveModes::Average,
                                           .load_op      = gpu::LoadOp::Load,
                                           .store_op     = gpu::StoreOp::Store,
                                           .clear        = {}})
            .unwrap();
      },
      [&]() {
          color
            .push(gpu::RenderingAttachment{.view    = params.framebuffer.color.view,
                                           .resolve = nullptr,
                                           .resolve_mode = gpu::ResolveModes::None,
                                           .load_op      = gpu::LoadOp::Load,
                                           .store_op     = gpu::StoreOp::Store,
                                           .clear        = {}})
            .unwrap();
      });

    auto stencil =
      params.framebuffer.depth_stencil.map([&](DepthStencilImage const & img) {
          return gpu::RenderingAttachment{.view         = img.stencil_view,
                                          .resolve      = nullptr,
                                          .resolve_mode = gpu::ResolveModes::None,
                                          .load_op      = gpu::LoadOp::Load,
                                          .store_op     = gpu::StoreOp::None,
                                          .clear        = {}};
      });

    auto info =
      gpu::RenderingInfo{.render_area{.extent = params.framebuffer.extent().xy()},
                         .num_layers         = 1,
                         .color_attachments  = color,
                         .depth_attachment   = {},
                         .stencil_attachment = stencil};

    e->begin_rendering(info);

    auto pipeline = pipelines_[params.variant].v0.v1;

    e->bind_graphics_pipeline(pipeline);
    e->push_constants(as_u8_span(params.params));
    e->bind_descriptor_sets(span({
                              params.samplers,    // 0: samplers
                              params.textures     // 1: textures
                            }),
                            {});

    ASH_CHECK(size32(params.states) > 0, "");
    ASH_CHECK(size32(params.state_runs) == (size32(params.states) + 1), "");
    ASH_CHECK(size32(params.index_runs) > 1, "");
    auto num_states = size32(params.states);

    for (auto s : range(num_states))
    {
        auto & state = params.states[s];
        e->set_graphics_state(gpu::GraphicsState{
          .scissor             = state.scissor,
          .viewport            = state.viewport,
          .stencil_test_enable = state.stencil.is_some(),
          .front_face_stencil =
            state.stencil.map([](auto s) { return s.front; }).unwrap_or(),
          .back_face_stencil =
            state.stencil.map([](auto s) { return s.back; }).unwrap_or(),
          .cull_mode  = state.cull_mode,
          .front_face = state.front_face});

        for (auto i :
             range(Slice32::offsets(params.state_runs[s], params.state_runs[s + 1])))
        {
            e->draw(Slice32::offsets(params.index_runs[i], params.index_runs[i + 1]),
                    {i, 1});
        }
    }

    e->end_rendering();
}

void TriangleFillPipeline::release(GpuFramePlan plan, Allocator)
{
    for (auto [v] : pipelines_)
    {
        plan->add_preframe_task(
          [p = v.v1, d = plan->device()](GpuFrame) { d->uninit(p); });
    }
}

Str VectorPathPipeline::label()
{
    return "VectorPath"_s;
}

static gpu::GraphicsPipeline create_coverage_pipeline(GpuFramePlan plan, Str label,
                                                      gpu::Shader shader)
{
    ThreadScratchScope scratch;
    auto &             gpu = *plan->sys();

    auto tagged_label =
      sformat(scratch, "VectorPath Coverage Graphics Pipeline: {}"_s, label).unwrap();

    auto raster_state =
      gpu::RasterizationState{.depth_clamp_enable = false,
                              .polygon_mode       = gpu::PolygonMode::Fill,
                              .cull_mode          = gpu::CullMode::None,
                              .front_face         = gpu::FrontFace::CounterClockWise,
                              .depth_bias_enable  = false,
                              .depth_bias_constant_factor = 0,
                              .depth_bias_clamp           = 0,
                              .depth_bias_slope_factor    = 0,
                              .sample_count               = gpu.sample_count()};

    auto depth_stencil_state =
      gpu::DepthStencilState{.depth_test_enable        = false,
                             .depth_write_enable       = false,
                             .depth_compare_op         = gpu::CompareOp::Never,
                             .depth_bounds_test_enable = false,
                             .stencil_test_enable      = false,
                             .front_stencil            = {},
                             .back_stencil             = {},
                             .min_depth_bounds         = 0,
                             .max_depth_bounds         = 0};

    auto color_blend_state = gpu::ColorBlendState{
      .attachments = {},
        .blend_constant = {1, 1, 1, 1}
    };

    auto & layout = gpu.descriptors_layout();

    gpu::DescriptorSetLayout set_layouts[] = {
      layout.storage_texel_buffers,    // 0: alpha_masks
      layout.storage_texel_buffers     // 1: fill_ids
    };

    auto pipeline_info = gpu::GraphicsPipelineInfo{
      .label           = tagged_label,
      .vertex_shader   = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "vert"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .fragment_shader = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "frag"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .color_formats   = span({gpu.color_format()}
          ),
      .depth_format    = {},
      .stencil_format  = gpu.depth_stencil_format(),
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(shader::VectorPathFillShaderParams),
      .descriptor_set_layouts = set_layouts,
      .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = gpu.pipeline_cache()
    };

    return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

static gpu::GraphicsPipeline create_fill_pipeline(GpuFramePlan plan, Str label,
                                                  gpu::Shader shader)
{
    ThreadScratchScope scratch;

    auto & gpu = *plan->sys();

    auto tagged_label =
      sformat(scratch, "VectorPath Fill Graphics Pipeline: {}"_s, label).unwrap();

    auto raster_state =
      gpu::RasterizationState{.depth_clamp_enable = false,
                              .polygon_mode       = gpu::PolygonMode::Fill,
                              .cull_mode          = gpu::CullMode::None,
                              .front_face         = gpu::FrontFace::CounterClockWise,
                              .depth_bias_enable  = false,
                              .depth_bias_constant_factor = 0,
                              .depth_bias_clamp           = 0,
                              .depth_bias_slope_factor    = 0,
                              .sample_count               = gpu.sample_count()};

    auto depth_stencil_state =
      gpu::DepthStencilState{.depth_test_enable        = false,
                             .depth_write_enable       = false,
                             .depth_compare_op         = gpu::CompareOp::Never,
                             .depth_bounds_test_enable = false,
                             .stencil_test_enable      = false,
                             .front_stencil            = {},
                             .back_stencil             = {},
                             .min_depth_bounds         = 0,
                             .max_depth_bounds         = 0};

    gpu::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = true,
       .src_color_blend_factor = gpu::BlendFactor::SrcAlpha,
       .dst_color_blend_factor = gpu::BlendFactor::OneMinusSrcAlpha,
       .color_blend_op         = gpu::BlendOp::Add,
       .src_alpha_blend_factor = gpu::BlendFactor::One,
       .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
       .alpha_blend_op         = gpu::BlendOp::Add,
       .color_write_mask       = gpu::ColorComponents::All}
    };

    auto color_blend_state = gpu::ColorBlendState{
      .attachments = attachment_states, .blend_constant = {1, 1, 1, 1}
    };

    auto & layout = gpu.descriptors_layout();

    gpu::DescriptorSetLayout set_layouts[] = {
      layout.samplers,               // 0: samplers
      layout.sampled_textures,       // 1: textures
      layout.read_storage_buffer,    // 2: alpha_masks
      layout.read_storage_buffer     // 3: fill_ids
    };

    auto pipeline_info = gpu::GraphicsPipelineInfo{
      .label           = tagged_label,
      .vertex_shader   = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "vert"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .fragment_shader = gpu::ShaderStageInfo{.shader                        = shader,
                                              .entry_point                   = "frag"_s,
                                              .specialization_constants      = {},
                                              .specialization_constants_data = {}},
      .color_formats   = span({gpu.color_format()}
          ),
      .depth_format    = {},
      .stencil_format  = gpu.depth_stencil_format(),
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(shader::VectorPathFillShaderParams),
      .descriptor_set_layouts = set_layouts,
      .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = gpu.pipeline_cache()
    };

    return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

VectorPathPipeline::VectorPathPipeline(Allocator allocator) :
  coverage_pipeline_{nullptr},
  fill_pipelines_{allocator}
{
}

void VectorPathPipeline::acquire(GpuFramePlan plan, Allocator allocator)
{
    coverage_pipeline_ = create_coverage_pipeline(
      plan, "coverage"_s,
      sys.shader->get("defaults/vector_path_coverage"_s).unwrap().shader);

    {
        auto id = add_fill_variant(
          plan, "base"_s,
          sys.shader->get("defaults/vector_path_base"_s).unwrap().shader, allocator);
        ASH_CHECK(id == PipelineVariantId::Base, "");
    }
}

PipelineVariantId VectorPathPipeline::add_fill_variant(GpuFramePlan plan, Str label,
                                                       gpu::Shader shader,
                                                       Allocator   allocator)
{
    auto pipeline = create_fill_pipeline(plan, label, shader);
    auto id       = fill_pipelines_.push(Tuple{label, pipeline}).unwrap();
    return (PipelineVariantId) id;
}

void VectorPathPipeline::remove_fill_variant(GpuFramePlan plan, PipelineVariantId id)
{
    auto pipeline = fill_pipelines_[id];
    fill_pipelines_.erase(id);
    plan->add_preframe_task(
      [p = pipeline.v0, d = plan->device()](GpuFrame) { d->uninit(p.v1); });
}

void VectorPathPipeline::encode(gpu::CommandEncoder                      e,
                                VectorPathCoveragePipelineParams const & params)
{
    auto stencil = gpu::RenderingAttachment{.view         = params.stencil.stencil_view,
                                            .resolve      = nullptr,
                                            .resolve_mode = gpu::ResolveModes::None,
                                            .load_op      = gpu::LoadOp::Load,
                                            .store_op     = gpu::StoreOp::None,
                                            .clear        = {}};

    auto info = gpu::RenderingInfo{.render_area{.extent = params.stencil.extent().xy()},
                                   .num_layers         = 1,
                                   .color_attachments  = {},
                                   .depth_attachment   = {},
                                   .stencil_attachment = stencil};

    e->begin_rendering(info);

    e->bind_graphics_pipeline(coverage_pipeline_);
    e->push_constants(as_u8_span(params.params));
    e->bind_descriptor_sets(span({
                              params.write_alpha_masks,    //
                              params.write_fill_ids        //
                            }),
                            {});

    ASH_CHECK(size32(params.states) > 0, "");
    ASH_CHECK(size32(params.state_runs) == (size32(params.states) + 1), "");
    ASH_CHECK(size32(params.index_runs) > 1, "");
    auto num_states = size32(params.states);

    for (auto s : range(num_states))
    {
        auto & state       = params.states[s];
        auto [front, back] = fill_rule_stencil(FillRule::NonZero, false, U32_MAX);

        e->set_graphics_state(gpu::GraphicsState{.scissor             = state.scissor,
                                                 .viewport            = state.viewport,
                                                 .stencil_test_enable = false,
                                                 .front_face_stencil  = front,
                                                 .back_face_stencil   = back,
                                                 .cull_mode = gpu::CullMode::None});

        for (auto i :
             range(Slice32::offsets(params.state_runs[s], params.state_runs[s + 1])))
        {
            e->draw(Slice32::offsets(params.index_runs[i], params.index_runs[i + 1]),
                    {i, 1});
        }
    }

    e->end_rendering();
}

void VectorPathPipeline::encode(gpu::CommandEncoder                  e,
                                VectorPathFillPipelineParams const & params)
{
    InplaceVec<gpu::RenderingAttachment, 1, 0> color;

    params.framebuffer.color_msaa.match(
      [&](ColorMsaaImage const & tex) {
          color
            .push(gpu::RenderingAttachment{.view    = tex.view,
                                           .resolve = params.framebuffer.color.view,
                                           .resolve_mode = gpu::ResolveModes::Average,
                                           .load_op      = gpu::LoadOp::Load,
                                           .store_op     = gpu::StoreOp::Store,
                                           .clear        = {}})
            .unwrap();
      },
      [&]() {
          color
            .push(gpu::RenderingAttachment{.view    = params.framebuffer.color.view,
                                           .resolve = nullptr,
                                           .resolve_mode = gpu::ResolveModes::None,
                                           .load_op      = gpu::LoadOp::Load,
                                           .store_op     = gpu::StoreOp::Store,
                                           .clear        = {}})
            .unwrap();
      });

    auto stencil = params.framebuffer.depth_stencil.map([&](auto & s) {
        return gpu::RenderingAttachment{.view         = s.stencil_view,
                                        .resolve      = nullptr,
                                        .resolve_mode = gpu::ResolveModes::None,
                                        .load_op      = gpu::LoadOp::Load,
                                        .store_op     = gpu::StoreOp::None,
                                        .clear        = {}};
    });

    auto info =
      gpu::RenderingInfo{.render_area{.extent = params.framebuffer.extent().xy()},
                         .num_layers         = 1,
                         .color_attachments  = color,
                         .depth_attachment   = {},
                         .stencil_attachment = stencil};

    auto pipeline = fill_pipelines_[params.variant].v0.v1;

    e->begin_rendering(info);
    e->bind_graphics_pipeline(pipeline);
    e->push_constants(as_u8_span(params.params));
    e->bind_descriptor_sets(span({
                              params.samplers,            // 0: samplers
                              params.textures,            // 1: textures
                              params.read_alpha_masks,    // 2: read_alpha_masks
                              params.read_fill_ids        // 3: read_fill_ids
                            }),
                            {});

    ASH_CHECK(size32(params.states) > 0, "");
    ASH_CHECK(size32(params.state_runs) == (size32(params.states) + 1), "");
    auto num_states = size32(params.states);

    for (auto s : range(num_states))
    {
        auto & state          = params.states[s];
        auto non_zero_stencil = gpu::StencilState{.fail_op       = gpu::StencilOp::Keep,
                                                  .pass_op       = gpu::StencilOp::Keep,
                                                  .depth_fail_op = gpu::StencilOp::Keep,
                                                  .compare_op = gpu::CompareOp::Greater,
                                                  .compare_mask = 0xFF,
                                                  .write_mask   = 0x00,
                                                  .reference    = 0x00};

        e->set_graphics_state(
          gpu::GraphicsState{.scissor             = state.scissor,
                             .viewport            = state.viewport,
                             .stencil_test_enable = true,
                             .front_face_stencil  = non_zero_stencil,
                             .back_face_stencil   = non_zero_stencil});

        e->draw({0, 4},
                Slice32::offsets(params.state_runs[s], params.state_runs[s + 1]));
    }
    e->end_rendering();
}

void VectorPathPipeline::release(GpuFramePlan plan, Allocator)
{
    plan->add_preframe_task(
      [p = coverage_pipeline_, d = plan->device()](GpuFrame) { d->uninit(p); });
    for (auto [v] : fill_pipelines_)
    {
        plan->add_preframe_task(
          [p = v.v1, d = plan->device()](GpuFrame) { d->uninit(p); });
    }
}

}    // namespace ash
