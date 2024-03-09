
#pragma once
#include "ashura/renderer/render_graph.h"
#include "ashura/renderer/view.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/box.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// E' = Blur(E)
/// D' = Blur(D) + E'
/// C' = Blur(C) + D'
/// B' = Blur(B) + C'
/// A' = Blur(A) + B'
// downsample to mip chains of 5 total
// perform gaussian blur of the image
// addittive composite back unto the first mip
struct BloomParams
{
  BloomConfig     config;
  rdg::ImageAttachment src;
  gfx::Offset     src_offset;
  gfx::Offset     src_extent;
  rdg::ImageAttachment dst;
  gfx::Offset     dst_offset;
  gfx::Offset     dst_extent;
};

struct BloomPass
{
  static void add_pass(rdg::Graph *graph, BloomParams const *params);
};

}        // namespace ash
