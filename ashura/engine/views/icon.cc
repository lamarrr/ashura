/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/icon.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

Icon::Icon(Str32 text, TextStyle const & style, FontStyle const & font,
           AllocatorRef allocator) :
  text_{allocator}
{
  text_.text(text).run(style, font);
}

Icon::Icon(Str8 text, TextStyle const & style, FontStyle const & font,
           AllocatorRef allocator) :
  text_{allocator}
{
  text_.text(text).run(style, font);
}

Icon & Icon::hide(bool hide)
{
  state_.hidden = hide;
  return *this;
}

Icon & Icon::icon(Str8 text, TextStyle const & style, FontStyle const & font)
{
  text_.text(text).run(style, font);
  return *this;
}

Icon & Icon::icon(Str32 text, TextStyle const & style, FontStyle const & font)
{
  text_.text(text).run(style, font);
  return *this;
}

ui::State Icon::tick(Ctx const &, Events const &, Fn<void(View &)>)
{
  return ui::State{.hidden = state_.hidden};
}

Layout Icon::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  text_.layout(allocated.x);
  return Layout{.extent = text_.get_layout().extent};
}

void Icon::render(Canvas & canvas, RenderInfo const & info)
{
  text_.render(canvas.text_renderer(), info.viewport_region.center,
               info.viewport_region.extent.x,
               transform2d_to_3d(info.canvas_transform), info.clip);
}

}    // namespace ui

}    // namespace ash
