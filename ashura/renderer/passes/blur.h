#pragma once
#include "ashura/renderer/render_graph.h"
#include "ashura/renderer/renderer.h"

namespace ash
{
// TODO(lamarrr): should this be less general-purpose?
// do we need to copy directly from the image?
enum class BlurRadius : u8
{
  None     = 0,
  Radius1  = 1,
  Radius2  = 2,
  Radius4  = 4,
  Radius8  = 8,
  Radius16 = 16,
  Radius32 = 32
};

struct BlurParams
{
  BlurRadius      blur_radius = BlurRadius::None;
  rdg::ImageAttachment src;
  gfx::Offset     src_offset;
  gfx::Offset     src_extent;
  rdg::ImageAttachment dst;
  gfx::Offset     dst_offset;
  gfx::Offset     dst_extent;
};

// object-clip space blur
//
// - capture scene at object's screen-space area, dilate by the blur extent
// - reserve scratch stencil image with at least size of the dilated area
// - blur captured area
// - render object to offscreen scratch image stencil only
// - using rendered stencil, directly-write (without blending) onto scene again
struct BlurPass
{
  static void create_kernel();
  static void add_pass(RenderGraph *graph, BlurParams const *params);
};

}        // namespace ash
