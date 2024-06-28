#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/widget.h"
#include "ashura/std/types.h"

namespace ash
{

// TODO(lamarrr): re-layout hint
struct TextBox : public Widget
{
  virtual Vec2 fit(Context &ctx, vec2 allocated_size,
                   stx::Span<vec2 const> children_allocations,
                   stx::Span<vec2 const> children_sizes,
                   stx::Span<vec2>       children_positions) override
  {
    if (is_layout_dirty ||
        text_layout.text_scale_factor != ctx.text_scale_factor)
    {
      TextRun runs[] = {TextRun{.size = (usize) -1, .style = 0}};

      TextBlock text_block{.text   = std::string_view{text.data(), text.size()},
                           .runs   = runs,
                           .styles = {},
                           .default_style = props.style,
                           .align         = TextAlign::Start,
                           .direction     = TextDirection::LeftToRight,
                           .language      = {}};

      vec2 size = props.frame.resolve(allocated_size);

      text_layout.layout(text_block, ctx.text_scale_factor, ctx.font_bundle,
                         size.x);
      is_layout_dirty = false;
    }

    return text_layout.span;
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    TextRun runs[] = {TextRun{.size = (usize) -1, .style = 0}};

    TextBlock text_block{.text   = std::string_view{text.data(), text.size()},
                         .runs   = runs,
                         .styles = {},
                         .default_style = props.style,
                         .align         = TextAlign::Start,
                         .direction     = TextDirection::LeftToRight,
                         .language      = {}};

    canvas.draw_text(text_block, text_layout, ctx.font_bundle, area.offset);
  }

  TextBlock      block  = {};
  TextBlockStyle style  = {};
  TextLayout     layout = {};
};

}        // namespace ash