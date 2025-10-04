/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/encoders.h"
#include "ashura/engine/pipeline_system.h"
#include "ashura/engine/pipelines/bezier_stencil.h"
#include "ashura/engine/pipelines/bloom.h"
#include "ashura/engine/pipelines/blur.h"
#include "ashura/engine/pipelines/fill_stencil.h"
#include "ashura/engine/pipelines/pbr.h"
#include "ashura/engine/pipelines/quad.h"
#include "ashura/engine/pipelines/sdf.h"
#include "ashura/engine/pipelines/triangle_fill.h"
#include "ashura/engine/pipelines/vector_path.h"
#include "ashura/engine/systems.h"

namespace ash
{

void SdfEncoder::submit(GpuFramePlan plan)
{
  auto states     = states_.view();
  auto state_runs = state_runs_.view();

  auto items = items_.view();

  auto i_states     = plan->push_cpu(states);
  auto i_state_runs = plan->push_cpu(state_runs);

  auto i_world_to_ndc = plan->push_gpu(span({world_to_ndc_}));
  auto i_items        = plan->push_gpu(items);

  plan->add_pass([attachments = this->attachments_,
                  texture_set = this->texture_set_, i_states, i_state_runs,
                  i_world_to_ndc, i_items, variant = this->variant_](
                   GpuFrame frame, gpu::CommandEncoder enc) {
    auto states       = frame->get<State>(i_states);
    auto state_runs   = frame->get<u32>(i_state_runs);
    auto world_to_ndc = frame->get(i_world_to_ndc);
    auto items        = frame->get(i_items);
    auto images       = frame->get_scratch_images();

    auto framebuffer =
      Framebuffer{.color         = images[attachments.color].color,
                  .color_msaa    = none,
                  .depth_stencil = attachments.depth_stencil.map(
                    [&](auto s) { return images[s].depth_stencil; })};

    auto params = SdfPipelineParams{.framebuffer = framebuffer,
                                    .samplers = sys.gpu->descriptors_.samplers,
                                    .textures = frame->get(texture_set),
                                    .world_to_ndc = world_to_ndc,
                                    .items        = items,
                                    .states       = states,
                                    .state_runs   = state_runs,
                                    .variant      = variant};

    sys.pipeline->sdf().encode(enc, params);
  });
}

void QuadEncoder::submit(GpuFramePlan plan)
{
  auto states     = states_.view();
  auto state_runs = state_runs_.view();

  auto quads = quads_.view();

  auto i_states     = plan->push_cpu(states);
  auto i_state_runs = plan->push_cpu(state_runs);

  auto i_world_to_ndc = plan->push_gpu(span({world_to_ndc_}));
  auto i_quads        = plan->push_gpu(quads);

  plan->add_pass(
    [attachments = this->attachments_, texture_set = this->texture_set_,
     i_states, i_state_runs, i_world_to_ndc, i_quads,
     variant = this->variant_](GpuFrame frame, gpu::CommandEncoder enc) {
      auto states       = frame->get<State>(i_states);
      auto state_runs   = frame->get<u32>(i_state_runs);
      auto world_to_ndc = frame->get(i_world_to_ndc);
      auto quads        = frame->get(i_quads);
      auto images       = frame->get_scratch_images();

      auto framebuffer =
        Framebuffer{.color         = images[attachments.color].color,
                    .color_msaa    = none,
                    .depth_stencil = attachments.depth_stencil.map(
                      [&](auto s) { return images[s].depth_stencil; })};

      auto params = QuadPipelineParams{.framebuffer  = framebuffer,
                                       .samplers     = sys.gpu->samplers(),
                                       .textures     = frame->get(texture_set),
                                       .world_to_ndc = world_to_ndc,
                                       .quads        = quads,
                                       .states       = states,
                                       .state_runs   = state_runs,
                                       .variant      = variant};

      sys.pipeline->quad().encode(enc, params);
    });
}

void TriangleFillEncoder::submit(GpuFramePlan plan)
{
  auto index_runs = index_runs_.view();
  auto states     = states_.view();
  auto state_runs = state_runs_.view();

  auto sets     = sets_.view();
  auto vertices = vertices_.view();
  auto indices  = indices_.view();

  auto i_index_runs = plan->push_cpu(index_runs);
  auto i_states     = plan->push_cpu(states);
  auto i_state_runs = plan->push_cpu(state_runs);

  auto i_world_to_ndc = plan->push_gpu(span({world_to_ndc_}));
  auto i_sets         = plan->push_gpu(sets);
  auto i_vertices     = plan->push_gpu(vertices);
  auto i_indices      = plan->push_gpu(indices);

  plan->add_pass(
    [attachments = this->attachments_, stencil_op = stencil_op_,
     texture_set = this->texture_set_, i_index_runs, i_states, i_state_runs,
     i_world_to_ndc, i_sets, i_vertices, i_indices,
     variant = this->variant_](GpuFrame frame, gpu::CommandEncoder enc) {
      auto world_to_ndc = frame->get(i_world_to_ndc);
      auto sets         = frame->get(i_sets);
      auto vertices     = frame->get(i_vertices);
      auto indices      = frame->get(i_indices);
      auto index_runs   = frame->get<u32>(i_index_runs);
      auto states       = frame->get<State>(i_states);
      auto state_runs   = frame->get<u32>(i_state_runs);
      auto images       = frame->get_scratch_images();

      auto framebuffer =
        Framebuffer{.color         = images[attachments.color].color,
                    .color_msaa    = none,
                    .depth_stencil = attachments.depth_stencil.map(
                      [&](auto s) { return images[s].depth_stencil; })};

      auto params =
        TriangleFillPipelineParams{.framebuffer  = framebuffer,
                                   .samplers     = sys.gpu->samplers(),
                                   .textures     = frame->get(texture_set),
                                   .world_to_ndc = world_to_ndc,
                                   .sets         = sets,
                                   .vertices     = vertices,
                                   .indices      = indices,
                                   .index_runs   = index_runs,
                                   .states       = states,
                                   .state_runs   = state_runs,
                                   .variant      = variant};

      sys.pipeline->triangle_fill().encode(enc, params);
    });
}

void FillStencilEncoder::submit(GpuFramePlan plan)
{
  auto index_runs = index_runs_.view();
  auto states     = states_.view();
  auto state_runs = state_runs_.view();

  auto world_transforms = world_transforms_.view();
  auto vertices         = vertices_.view();
  auto indices          = indices_.view();

  auto i_world_to_ndc     = plan->push_gpu(span({world_to_ndc_}));
  auto i_world_transforms = plan->push_gpu(world_transforms);
  auto i_vertices         = plan->push_gpu(vertices);
  auto i_indices          = plan->push_gpu(indices);

  auto i_index_runs = plan->push_cpu(index_runs);
  auto i_states     = plan->push_cpu(states);
  auto i_state_runs = plan->push_cpu(state_runs);

  plan->add_pass([attachments = this->attachments_, i_world_to_ndc,
                  i_world_transforms, i_vertices, i_indices, i_index_runs,
                  i_states,
                  i_state_runs](GpuFrame frame, gpu::CommandEncoder enc) {
    auto world_to_ndc     = frame->get(i_world_to_ndc);
    auto world_transforms = frame->get(i_world_transforms);
    auto vertices         = frame->get(i_vertices);
    auto indices          = frame->get(i_indices);
    auto index_runs       = frame->get<u32>(i_index_runs);
    auto states           = frame->get<State>(i_states);
    auto state_runs       = frame->get<u32>(i_state_runs);
    auto images           = frame->get_scratch_images();

    auto image = images[attachments.depth_stencil].depth_stencil;
    auto stencil =
      gpu::RenderingAttachment{.view         = image.stencil_view,
                               .resolve      = nullptr,
                               .resolve_mode = gpu::ResolveModes::None,
                               .load_op      = gpu::LoadOp::Clear,
                               .store_op     = gpu::StoreOp::Store,
                               .clear        = {}};

    auto params = FillStencilPipelineParams{
      .stencil_attachment = stencil,
      .render_area        = {.offset = {}, .extent = image.extent().xy()},
      .world_to_ndc       = world_to_ndc,
      .world_transforms   = world_transforms,
      .vertices           = vertices,
      .indices            = indices,
      .index_runs         = index_runs,
      .states             = states,
      .state_runs         = state_runs
    };

    sys.pipeline->fill_stencil().encode(enc, params);
  });
}

void BezierStencilEncoder::submit(GpuFramePlan plan)
{
  auto index_runs = index_runs_.view();
  auto states     = states_.view();
  auto state_runs = state_runs_.view();
  auto items      = items_.view();
  auto vertices   = vertices_.view();
  auto indices    = indices_.view();

  auto i_index_runs = plan->push_cpu(index_runs);
  auto i_states     = plan->push_cpu(states);
  auto i_state_runs = plan->push_cpu(state_runs);

  auto i_world_to_ndc = plan->push_gpu(span({world_to_ndc_}));
  auto i_items        = plan->push_gpu(items);
  auto i_vertices     = plan->push_gpu(vertices);
  auto i_indices      = plan->push_gpu(indices);

  plan->add_pass([attachments = this->attachments_, i_index_runs, i_states,
                  i_state_runs, i_world_to_ndc, i_items, i_vertices,
                  i_indices](GpuFrame frame, gpu::CommandEncoder enc) {
    auto index_runs   = frame->get<u32>(i_index_runs);
    auto states       = frame->get<State>(i_states);
    auto state_runs   = frame->get<u32>(i_state_runs);
    auto world_to_ndc = frame->get(i_world_to_ndc);
    auto items        = frame->get(i_items);
    auto vertices     = frame->get(i_vertices);
    auto indices      = frame->get(i_indices);
    auto images       = frame->get_scratch_images();

    auto image = images[attachments.depth_stencil].depth_stencil;
    auto stencil =
      gpu::RenderingAttachment{.view         = image.stencil_view,
                               .resolve      = nullptr,
                               .resolve_mode = gpu::ResolveModes::None,
                               .load_op      = gpu::LoadOp::Clear,
                               .store_op     = gpu::StoreOp::Store,
                               .clear        = {}};

    auto params = BezierStencilPipelineParams{
      .stencil_attachment = stencil,
      .render_area        = {.offset = {}, .extent = image.extent().xy()},
      .world_to_ndc       = world_to_ndc,
      .items              = items,
      .vertices           = vertices,
      .indices            = indices,
      .index_runs         = index_runs,
      .states             = states,
      .state_runs         = state_runs
    };

    sys.pipeline->bezier_stencil().encode(enc, params);
  });
}

void FillPathEncoder::submit(GpuFramePlan plan)
{
  stencil_.submit(plan);
  fill_.submit(plan);
}

void BezierPathEncoder::submit(GpuFramePlan plan)
{
  stencil_.submit(plan);
  fill_.submit(plan);
}

void VectorPathEncoder::submit(GpuFramePlan plan)
{
  auto index_runs          = index_runs_.view();
  auto coverage_states     = coverage_states_.view();
  auto coverage_state_runs = coverage_state_runs_.view();
  auto fill_states         = fill_states_.view();
  auto fill_state_runs     = fill_state_runs_.view();

  auto vertices       = vertices_.view();
  auto indices        = indices_.view();
  auto coverage_items = coverage_items_.view();
  auto fill_items     = fill_items_.view();

  auto i_world_to_ndc   = plan->push_gpu(span({world_to_ndc_}));
  auto i_vertices       = plan->push_gpu(vertices);
  auto i_indices        = plan->push_gpu(indices);
  auto i_coverage_items = plan->push_gpu(coverage_items);
  auto i_fill_items     = plan->push_gpu(fill_items);

  auto i_index_runs          = plan->push_cpu(index_runs);
  auto i_coverage_states     = plan->push_cpu(coverage_states);
  auto i_coverage_state_runs = plan->push_cpu(coverage_state_runs);
  auto i_fill_states         = plan->push_cpu(fill_states);
  auto i_fill_state_runs     = plan->push_cpu(fill_state_runs);

  plan->add_pass(
    [attachments = this->attachments_, texture_set = this->texture_set_,
     i_index_runs, i_coverage_states, i_coverage_state_runs, i_fill_states,
     i_fill_state_runs, i_world_to_ndc, i_vertices, i_indices, i_coverage_items,
     i_fill_items](GpuFrame frame, gpu::CommandEncoder enc) {
      auto world_to_ndc        = frame->get(i_world_to_ndc);
      auto vertices            = frame->get(i_vertices);
      auto indices             = frame->get(i_indices);
      auto coverage_items      = frame->get(i_coverage_items);
      auto fill_items          = frame->get(i_fill_items);
      auto index_runs          = frame->get<u32>(i_index_runs);
      auto coverage_states     = frame->get<State>(i_coverage_states);
      auto coverage_state_runs = frame->get<u32>(i_coverage_state_runs);
      auto fill_states         = frame->get<State>(i_fill_states);
      auto fill_state_runs     = frame->get<u32>(i_fill_state_runs);
      auto images              = frame->get_scratch_images();

      {
        auto coverage_params = VectorPathCoveragePipelineParams{
          .stencil = images[attachments.scratch_depth_stencil].depth_stencil,
          .write_alpha_masks = images[attachments.scratch_alpha_mask]
                                 .texel.interpret(gpu::Format::R32_SFLOAT)
                                 .storage_texel_buffers,
          .write_fill_ids = images[attachments.scratch_fill_id]
                              .texel.interpret(gpu::Format::R32_UINT)
                              .storage_texel_buffers,
          .world_to_ndc   = world_to_ndc,
          .vertices       = vertices,
          .indices        = indices,
          .coverage_items = coverage_items,
          .index_runs     = index_runs,
          .states         = coverage_states,
          .state_runs     = coverage_state_runs};

        sys.pipeline->vector_path().encode(enc, coverage_params,
                                           PipelineVariantId::Base);
      }

      {
        auto framebuffer =
          Framebuffer{.color         = images[attachments.color].color,
                      .color_msaa    = none,
                      .depth_stencil = attachments.depth_stencil.map(
                        [&](auto s) { return images[s].depth_stencil; })};

        auto fill_params = VectorPathFillPipelineParams{
          .framebuffer      = framebuffer,
          .samplers         = sys.gpu->samplers(),
          .textures         = frame->get(texture_set),
          .read_alpha_masks = images[attachments.scratch_alpha_mask]
                                .texel.interpret(gpu::Format::R32_SFLOAT)
                                .uniform_texel_buffers,
          .read_fill_ids = images[attachments.scratch_fill_id]
                             .texel.interpret(gpu::Format::R32_UINT)
                             .uniform_texel_buffers,
          .world_to_ndc = world_to_ndc,
          .fill_items   = fill_items,
          .states       = fill_states,
          .state_runs   = fill_state_runs};

        sys.pipeline->vector_path().encode(enc, fill_params,
                                           PipelineVariantId::Base);
      }
    });
}

void PbrEncoder::submit(GpuFramePlan plan)
{
  auto i_item   = plan->push_gpu(item_.view());
  auto i_lights = plan->push_gpu(lights_.view());

  plan->add_pass(
    [attachments = this->attachments_, stencil_op = stencil_op_,
     scissor = this->scissor_, polygon_mode = this->polygon_mode_,
     viewport = this->viewport_, texture_set = this->texture_set_,
     vertices = this->vertices_, indices = this->indices_,
     num_indices = this->num_indices_, i_item, i_lights,
     cull_mode = this->cull_mode_, front_face = this->front_face_,
     variant = this->variant_](GpuFrame frame, gpu::CommandEncoder enc) {
      auto items  = frame->get(i_item);
      auto lights = frame->get(i_lights);
      auto images = frame->get_scratch_images();

      auto framebuffer =
        Framebuffer{.color         = images[attachments.color].color,
                    .color_msaa    = none,
                    .depth_stencil = attachments.depth_stencil.map(
                      [&](auto s) { return images[s].depth_stencil; })};

      auto params = PBRPipelineParams{.framebuffer  = framebuffer,
                                      .stencil      = stencil_op,
                                      .scissor      = scissor,
                                      .viewport     = viewport,
                                      .polygon_mode = polygon_mode,
                                      .samplers     = sys.gpu->samplers(),
                                      .textures     = frame->get(texture_set),
                                      .vertices     = vertices,
                                      .indices      = indices,
                                      .items        = items,
                                      .lights       = lights,
                                      .num_indices  = num_indices,
                                      .cull_mode    = cull_mode,
                                      .front_face   = front_face,
                                      .variant      = variant};

      sys.pipeline->pbr().encode(enc, params);
    });
}

}    // namespace ash
