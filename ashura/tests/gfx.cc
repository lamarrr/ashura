
#include "ashura/gfx.h"
#include "gtest/gtest.h"

using namespace ash::gfx;

TEST(GFX, ReadAfterWrite)
{
  ImageMemoryBarrier barrier;
  ImageState              state;
  ImageAccess shader_access{.stages = PipelineStages::FragmentShader | PipelineStages::VertexShader,
                            .access = Access::ShaderRead,
                            .layout = ImageLayout::ShaderReadOnlyOptimal};

  EXPECT_TRUE(state.sync(shader_access, barrier));
  EXPECT_EQ(barrier.old_layout, ImageLayout::Undefined);
  EXPECT_EQ(barrier.new_layout, ImageLayout::ShaderReadOnlyOptimal);
  EXPECT_EQ(barrier.src_stages, PipelineStages::None);
  EXPECT_EQ(barrier.dst_stages, PipelineStages::FragmentShader | PipelineStages::VertexShader);
  EXPECT_EQ(barrier.src_access, Access::None);
  EXPECT_EQ(barrier.dst_access, Access::ShaderRead);
  EXPECT_EQ(barrier.first_array_layer, 0);
  EXPECT_EQ(barrier.first_mip_level, 0);
  EXPECT_EQ(barrier.num_array_layers, REMAINING_ARRAY_LAYERS);
  EXPECT_EQ(barrier.num_mip_levels, REMAINING_MIP_LEVELS);

  EXPECT_TRUE(state.sync(shader_access, barrier));
  EXPECT_EQ(barrier.old_layout, ImageLayout::ShaderReadOnlyOptimal);
  EXPECT_EQ(barrier.new_layout, ImageLayout::ShaderReadOnlyOptimal);
  EXPECT_EQ(barrier.src_stages, PipelineStages::FragmentShader | PipelineStages::VertexShader);
  EXPECT_EQ(barrier.dst_stages, PipelineStages::FragmentShader | PipelineStages::VertexShader);
  EXPECT_EQ(barrier.src_access, Access::ShaderRead);
  EXPECT_EQ(barrier.dst_access, Access::ShaderRead);
  EXPECT_EQ(barrier.first_array_layer, 0);
  EXPECT_EQ(barrier.first_mip_level, 0);
  EXPECT_EQ(barrier.num_array_layers, REMAINING_ARRAY_LAYERS);
  EXPECT_EQ(barrier.num_mip_levels, REMAINING_MIP_LEVELS);

  EXPECT_TRUE(state.sync(shader_access, barrier));
  EXPECT_EQ(barrier.old_layout, ImageLayout::ShaderReadOnlyOptimal);
  EXPECT_EQ(barrier.new_layout, ImageLayout::ShaderReadOnlyOptimal);
  EXPECT_EQ(barrier.src_stages, PipelineStages::FragmentShader | PipelineStages::VertexShader);
  EXPECT_EQ(barrier.dst_stages, PipelineStages::FragmentShader | PipelineStages::VertexShader);
  EXPECT_EQ(barrier.src_access, Access::ShaderRead);
  EXPECT_EQ(barrier.dst_access, Access::ShaderRead);
  EXPECT_EQ(barrier.first_array_layer, 0);
  EXPECT_EQ(barrier.first_mip_level, 0);
  EXPECT_EQ(barrier.num_array_layers, REMAINING_ARRAY_LAYERS);
  EXPECT_EQ(barrier.num_mip_levels, REMAINING_MIP_LEVELS);
}

TEST(GFX, WriteAfterRead)
{
  ImageMemoryBarrier barrier;
  ImageState              state{.access   = {ImageAccess{.stages = PipelineStages::None,
                                                         .access = Access::None,
                                                         .layout = ImageLayout::General}},
                                .sequence = AccessSequence::None};

  ImageAccess shader_read1{.stages = PipelineStages::FragmentShader,
                           .access = Access::ShaderRead,
                           .layout = ImageLayout::General};
  ImageAccess shader_read2{.stages = PipelineStages::VertexShader,
                           .access = Access::ShaderRead,
                           .layout = ImageLayout::General};

  EXPECT_FALSE(state.sync(shader_read1, barrier));
  EXPECT_FALSE(state.sync(shader_read2, barrier));
  EXPECT_EQ(state.sequence, AccessSequence::NoneAfterRead);
  EXPECT_EQ(state.access[0].layout, ImageLayout::General);
  EXPECT_EQ(state.access[0].access, Access::ShaderRead);
  EXPECT_EQ(state.access[0].stages, PipelineStages::FragmentShader | PipelineStages::VertexShader);

  ImageAccess attachment_write{.stages = PipelineStages::ColorAttachmentOutput,
                               .access = Access::ColorAttachmentWrite,
                               .layout = ImageLayout::General};

  EXPECT_TRUE(state.sync(attachment_write, barrier));
  EXPECT_EQ(state.sequence, AccessSequence::NoneAfterWrite);
  EXPECT_EQ(barrier.old_layout, ImageLayout::General);
  EXPECT_EQ(barrier.new_layout, ImageLayout::General);
  EXPECT_EQ(barrier.src_stages, PipelineStages::FragmentShader | PipelineStages::VertexShader);
  EXPECT_EQ(barrier.dst_stages, PipelineStages::ColorAttachmentOutput);
  EXPECT_EQ(barrier.src_access, Access::ShaderRead);
  EXPECT_EQ(barrier.dst_access, Access::ColorAttachmentWrite);
  EXPECT_EQ(barrier.first_array_layer, 0);
  EXPECT_EQ(barrier.first_mip_level, 0);
  EXPECT_EQ(barrier.num_array_layers, REMAINING_ARRAY_LAYERS);
  EXPECT_EQ(barrier.num_mip_levels, REMAINING_MIP_LEVELS);

  EXPECT_TRUE(state.sync(attachment_write, barrier));
  EXPECT_EQ(state.sequence, AccessSequence::NoneAfterWrite);
  EXPECT_EQ(barrier.old_layout, ImageLayout::General);
  EXPECT_EQ(barrier.new_layout, ImageLayout::General);
  EXPECT_EQ(barrier.src_stages, PipelineStages::ColorAttachmentOutput);
  EXPECT_EQ(barrier.dst_stages, PipelineStages::ColorAttachmentOutput);
  EXPECT_EQ(barrier.src_access, Access::ColorAttachmentWrite);
  EXPECT_EQ(barrier.dst_access, Access::ColorAttachmentWrite);
  EXPECT_EQ(barrier.first_array_layer, 0);
  EXPECT_EQ(barrier.first_mip_level, 0);
  EXPECT_EQ(barrier.num_array_layers, REMAINING_ARRAY_LAYERS);
  EXPECT_EQ(barrier.num_mip_levels, REMAINING_MIP_LEVELS);
}
