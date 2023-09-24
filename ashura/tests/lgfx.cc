#include "ashura/lgfx.h"
#include "gtest/gtest.h"

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H

#include "spirv_reflect.h"

using namespace ash::lgfx;
using namespace ash;

// struct PipelineAttribute{
// ASH_SHADER_ATTRIBUTE(VEC2, uv),
// ASH_SHADER_ATTRIBUTE(VEC4, color),
// };

TEST(LGFX, Basic)
{
  Graph     graph;
  CmdBuffer cmd_buffer;

  Image image = graph.create_image(ImageDesc{.format = Format::R8G8B8A8_UNORM,
                                             .usages = ImageUsages::Sampled | ImageUsages::TransferSrc | ImageUsages::TransferDst,
                                             .extent = {100, 100},
                                             .mips   = 2});

  EXPECT_NE(image, Image::None);

  ImageView view = graph.create_image_view(ImageViewDesc{.image           = image,
                                                         .view_format     = Format::R8G8B8A8_UNORM,
                                                         .first_mip_level = 1,
                                                         .num_mip_levels  = 1,
                                                         .aspect          = ImageAspect::Color});

  EXPECT_NE(view, ImageView::None);

  EXPECT_NO_FATAL_FAILURE(validate_resources(graph));

  ImageView view2 = graph.create_image_view(ImageViewDesc{.image           = image,
                                                          .view_format     = Format::R8G8B8A8_UNORM,
                                                          .first_mip_level = 2,
                                                          .num_mip_levels  = 1,
                                                          .aspect          = ImageAspect::Color});

  EXPECT_NE(view2, ImageView::None);

  EXPECT_DEATH(validate_resources(graph), ".*");

  Image image2 = graph.create_image(ImageDesc{.format = Format::R8G8B8A8_UNORM,
                                              .usages = ImageUsages::Sampled | ImageUsages::TransferSrc | ImageUsages::TransferDst,
                                              .extent = {100, 100},
                                              .mips   = 2});

  ImageCopy good_copy[] = {ImageCopy{.src_area      = URect{.offset = {}, .extent = {20, 20}},
                                     .src_mip_level = 0,
                                     .src_aspect    = ImageAspect::Color,
                                     .dst_offset    = Offset{0, 0},
                                     .dst_mip_level = 0,
                                     .dst_aspect    = ImageAspect::Color}};
  cmd_buffer.add(cmd::CopyImage{.src    = image,
                                .dst    = image2,
                                .copies = good_copy});

  EXPECT_NO_FATAL_FAILURE(validate_commands(graph, cmd_buffer.cmds));

  ImageCopy bad_copy[] = {ImageCopy{.src_area      = URect{.offset = {}, .extent = {20, 20}},
                                    .src_mip_level = 0,
                                    .src_aspect    = ImageAspect::Color,
                                    .dst_offset    = Offset{0, 0},
                                    .dst_mip_level = 2,
                                    .dst_aspect    = ImageAspect::Color}};

  CmdBuffer bad_cmd_buffer;

  bad_cmd_buffer.add(cmd::CopyImage{.src = image, .dst = image2, .copies = bad_copy});

  EXPECT_DEATH(validate_commands(graph, bad_cmd_buffer.cmds), ".*");

  stx::Vec<QueueBarrier> queue_barriers;
  stx::Vec<u32>          cmd_barriers;

  generate_barriers(graph, cmd_buffer.cmds, queue_barriers, cmd_barriers);

  EXPECT_EQ(cmd_barriers.size(), 1);
  EXPECT_EQ(cmd_barriers[0], 2);
  EXPECT_EQ(graph.get_state(image).layout, ImageLayout::TransferSrcOptimal);
  EXPECT_EQ(graph.get_state(image2).layout, ImageLayout::TransferDstOptimal);
}
