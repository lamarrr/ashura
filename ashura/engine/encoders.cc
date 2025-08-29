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
#include "ashura/engine/systems.h"

namespace ash
{

void SdfEncoder::operator()(GpuFramePlan plan)
{
  auto shapes     = shapes_.view();
  auto transforms = transforms_.view();
  auto materials  = materials_.view();

  auto i_world_to_ndc = plan->push_gpu(span({world_to_ndc_}));
  auto i_shapes       = plan->push_gpu(shapes);
  auto i_transforms   = plan->push_gpu(transforms);
  auto i_materials    = plan->push_gpu(materials);

  auto num_instances = size32(shapes);

  plan->add_pass([color = this->color_, depth_stencil = this->depth_stencil_,
                  stencil_op = this->stencil_op_, scissor = this->scissor_,
                  viewport = this->viewport_, texture_set = this->texture_set_,
                  i_world_to_ndc, i_shapes, i_transforms, i_materials,
                  variant = this->variant_,
                  num_instances](GpuFrame frame, gpu::CommandEncoder enc) {
    auto world_to_ndc = frame->get(i_world_to_ndc);
    auto shapes       = frame->get(i_shapes);
    auto transforms   = frame->get(i_transforms);
    auto materials    = frame->get(i_materials);

    auto dst = frame->get_scratch_images();

    auto framebuffer = Framebuffer{
      .color      = dst[color].color,
      .color_msaa = none,
      .depth_stencil =
        depth_stencil.map([&](auto s) { return dst[s].depth_stencil; })
          .unwrap_or()};

    auto params = SdfPipelineParams{
      .framebuffer  = framebuffer,
      .stencil      = stencil_op,
      .scissor      = scissor,
      .viewport     = viewport,
      .samplers     = sys.gpu->descriptors_.samplers,
      .textures     = frame->get(texture_set),
      .world_to_ndc = world_to_ndc,
      .shapes       = shapes,
      .transforms   = transforms,
      .materials    = materials,
      .instances{0, num_instances}
    };

    sys.pipeline->sdf().encode(enc, params, variant);
  });
}

void QuadEncoder::operator()(GpuFramePlan plan)
{
  auto quads      = quads_.view();
  auto transforms = transforms_.view();
  auto materials  = materials_.view();

  auto i_world_to_ndc = plan->push_gpu(span({world_to_ndc_}));
  auto i_quads        = plan->push_gpu(quads);
  auto i_transforms   = plan->push_gpu(transforms);
  auto i_materials    = plan->push_gpu(materials);

  plan->add_pass([color = this->color_, depth_stencil = this->depth_stencil_,
                  stencil_op = this->stencil_op_, scissor = this->scissor_,
                  viewport = this->viewport_, texture_set = this->texture_set_,
                  i_world_to_ndc, i_quads, i_transforms, i_materials,
                  num_instances = size32(quads), variant = this->variant_](
                   GpuFrame frame, gpu::CommandEncoder enc) {
    auto world_to_ndc = frame->get(i_world_to_ndc);
    auto quads        = frame->get(i_quads);
    auto transforms   = frame->get(i_transforms);
    auto materials    = frame->get(i_materials);

    auto dst = frame->get_scratch_images();

    auto framebuffer = Framebuffer{
      .color      = dst[color].color,
      .color_msaa = none,
      .depth_stencil =
        depth_stencil.map([&](auto s) { return dst[s].depth_stencil; })
          .unwrap_or()};

    auto params = QuadPipelineParams{
      .framebuffer  = framebuffer,
      .stencil      = stencil_op,
      .scissor      = scissor,
      .viewport     = viewport,
      .samplers     = sys.gpu->samplers(),
      .textures     = frame->get(texture_set),
      .world_to_ndc = world_to_ndc,
      .quads        = quads,
      .transforms   = transforms,
      .materials    = materials,
      .instances{0, num_instances}
    };

    sys.pipeline->quad().encode(enc, params, variant);
  });
}

void TriangleFillEncoder::operator()(GpuFramePlan plan)
{
  auto index_counts = index_counts_.view();
  auto transforms   = transforms_.view();
  auto vertices     = vertices_.view();
  auto indices      = indices_.view();
  auto materials    = materials_.view();

  auto i_world_to_ndc = plan->push_gpu(span({world_to_ndc_}));
  auto i_transforms   = plan->push_gpu(transforms);
  auto i_vertices     = plan->push_gpu(vertices);
  auto i_indices      = plan->push_gpu(indices);
  auto i_materials    = plan->push_gpu(materials);
  auto i_index_counts = plan->push_cpu(index_counts);

  plan->add_pass([color = this->color_, depth_stencil = this->depth_stencil_,
                  stencil_op = stencil_op_, scissor = this->scissor_,
                  viewport = this->viewport_, texture_set = this->texture_set_,
                  i_world_to_ndc, i_transforms, i_vertices, i_indices,
                  i_materials, i_index_counts, variant = this->variant_](
                   GpuFrame frame, gpu::CommandEncoder enc) {
    auto world_to_ndc = frame->get(i_world_to_ndc);
    auto transforms   = frame->get(i_transforms);
    auto vertices     = frame->get(i_vertices);
    auto indices      = frame->get(i_indices);
    auto materials    = frame->get(i_materials);
    auto index_counts = frame->get<u32>(i_index_counts);

    auto dst = frame->get_scratch_images();

    auto framebuffer = Framebuffer{
      .color      = dst[color].color,
      .color_msaa = none,
      .depth_stencil =
        depth_stencil.map([&](auto s) { return dst[s].depth_stencil; })
          .unwrap_or()};

    auto params =
      TriangleFillPipelineParams{.framebuffer    = framebuffer,
                                 .stencil        = stencil_op,
                                 .scissor        = scissor,
                                 .viewport       = viewport,
                                 .samplers       = sys.gpu->samplers(),
                                 .textures       = frame->get(texture_set),
                                 .world_to_ndc   = world_to_ndc,
                                 .transforms     = transforms,
                                 .vertices       = vertices,
                                 .indices        = indices,
                                 .materials      = materials,
                                 .first_instance = 0,
                                 .index_counts   = index_counts};

    sys.pipeline->triangle_fill().encode(enc, params, variant);
  });
}

void FillStencilEncoder::operator()(GpuFramePlan plan)
{
  auto index_counts = index_counts_.view();
  auto transforms   = transforms_.view();
  auto vertices     = vertices_.view();
  auto indices      = indices_.view();

  auto i_world_to_ndc = plan->push_gpu(span({world_to_ndc_}));
  auto i_transforms   = plan->push_gpu(transforms);
  auto i_vertices     = plan->push_gpu(vertices);
  auto i_indices      = plan->push_gpu(indices);
  auto i_index_counts = plan->push_cpu(index_counts);

  plan->add_pass([depth_stencil = this->depth_stencil_,
                  write_mask = this->write_mask_, scissor = this->scissor_,
                  viewport = this->viewport_, fill_rule = this->fill_rule_,
                  invert = this->invert_, i_world_to_ndc, i_transforms,
                  i_vertices, i_indices,
                  i_index_counts](GpuFrame frame, gpu::CommandEncoder enc) {
    auto world_to_ndc = frame->get(i_world_to_ndc);
    auto transforms   = frame->get(i_transforms);
    auto vertices     = frame->get(i_vertices);
    auto indices      = frame->get(i_indices);
    auto index_counts = frame->get<u32>(i_index_counts);

    auto dst = frame->get_scratch_images();

    auto params =
      FillStencilPipelineParams{.stencil    = dst[depth_stencil].depth_stencil,
                                .write_mask = write_mask,
                                .scissor    = scissor,
                                .viewport   = viewport,
                                .fill_rule  = fill_rule,
                                .invert     = invert,
                                .world_to_ndc   = world_to_ndc,
                                .transforms     = transforms,
                                .vertices       = vertices,
                                .indices        = indices,
                                .first_instance = 0,
                                .index_counts   = index_counts};

    sys.pipeline->fill_stencil().encode(enc, params);
  });
}

void BezierStencilEncoder::operator()(GpuFramePlan plan)
{
  auto vertices            = vertices_.view();
  auto indices             = indices_.view();
  auto transforms          = transforms_.view();
  auto regions             = regions_.view();
  auto region_index_counts = region_index_counts_.view();

  auto i_vertices            = plan->push_gpu(vertices);
  auto i_indices             = plan->push_gpu(indices);
  auto i_world_to_ndc        = plan->push_gpu(span({world_to_ndc_}));
  auto i_transforms          = plan->push_gpu(transforms);
  auto i_regions             = plan->push_gpu(regions);
  auto i_region_index_counts = plan->push_cpu(region_index_counts);

  plan->add_pass(
    [depth_stencil = this->depth_stencil_, write_mask = this->write_mask_,
     scissor = this->scissor_, viewport = this->viewport_,
     fill_rule = this->fill_rule_, invert = this->invert_, i_vertices,
     i_indices, i_world_to_ndc, i_transforms, i_regions,
     i_region_index_counts](GpuFrame frame, gpu::CommandEncoder enc) {
      auto vertices            = frame->get(i_vertices);
      auto indices             = frame->get(i_indices);
      auto world_to_ndc        = frame->get(i_world_to_ndc);
      auto transforms          = frame->get(i_transforms);
      auto regions             = frame->get(i_regions);
      auto region_index_counts = frame->get<u32>(i_region_index_counts);

      auto dst = frame->get_scratch_images();

      auto params =
        BezierStencilPipelineParams{.stencil = dst[depth_stencil].depth_stencil,
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

      sys.pipeline->bezier_stencil().encode(enc, params);
    });
}

void PbrEncoder::operator()(GpuFramePlan plan)
{
  auto i_world    = plan->push_gpu(world_.view());
  auto i_material = plan->push_gpu(material_.view());
  auto i_lights   = plan->push_gpu(lights_.view());

  plan->add_pass(
    [color = color_, depth_stencil = this->depth_stencil_,
     stencil_op = stencil_op_, scissor = this->scissor_,
     polygon_mode = this->polygon_mode_, viewport = this->viewport_,
     texture_set = this->texture_set_, vertices = this->vertices_,
     indices = this->indices_, num_indices = this->num_indices_, i_world,
     i_material, i_lights,
     variant = this->variant_](GpuFrame frame, gpu::CommandEncoder enc) {
      auto world    = frame->get(i_world);
      auto material = frame->get(i_material);
      auto lights   = frame->get(i_lights);

      auto dst = frame->get_scratch_images();

      auto framebuffer = Framebuffer{
        .color      = dst[color].color,
        .color_msaa = none,
        .depth_stencil =
          depth_stencil.map([&](auto s) { return dst[s].depth_stencil; })
            .unwrap_or()};

      auto params = PBRPipelineParams{.framebuffer  = framebuffer,
                                      .stencil      = stencil_op,
                                      .scissor      = scissor,
                                      .viewport     = viewport,
                                      .polygon_mode = polygon_mode,
                                      .samplers     = sys.gpu->samplers(),
                                      .textures     = frame->get(texture_set),
                                      .vertices     = vertices,
                                      .indices      = indices,
                                      .world        = world,
                                      .material     = material,
                                      .lights       = lights,
                                      .num_indices  = num_indices};

      sys.pipeline->pbr().encode(enc, params, variant);
    });
}

}    // namespace ash
