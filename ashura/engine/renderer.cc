/// SPDX-License-Identifier: MIT
#include "ashura/engine/renderer.h"
#include "ashura/engine/canvas.h"
#include "ashura/std/trace.h"

namespace ash
{

void Renderer::render_canvas(FrameGraph & graph, Canvas const & c,
                             Framebuffer const &             fb,
                             Span<ColorTexture const>        scratch_colors,
                             Span<DepthStencilTexture const> scratch_ds)
{
  ScopeTrace trace;
}

}    // namespace ash
