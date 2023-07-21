#pragma once

#include "ashura/font.h"
#include "ashura/text.h"
#include "ashura/widget.h"

namespace ash
{

struct Text : public Widget
{
  explicit Text(std::string_view itext, TextStyle istyle = TextStyle{}) :
      text{stx::string::make(stx::os_allocator, itext).unwrap()}, style{istyle}
  {}

  virtual WidgetDebugInfo get_debug_info(Context &ctx) override
  {
    return WidgetDebugInfo{.type = "Text"};
  }

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    TextRun runs[] = {
        TextRun{.size = (usize) -1, .style = 0}};

    TextBlock text_block{
        .text          = std::string_view{text.data(), text.size()},
        .runs          = runs,
        .styles        = {},
        .default_style = style,
        .align         = TextAlign::Start,
        .direction     = TextDirection::LeftToRight,
        .language      = {}};

    text_layout.layout(text_block, ctx.font_bundle, allocated_size.x);

    return text_layout.span;
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    TextRun runs[] = {
        TextRun{.size = (usize) -1, .style = 0}};

    TextBlock text_block{
        .text          = std::string_view{text.data(), text.size()},
        .runs          = runs,
        .styles        = {},
        .default_style = style,
        .align         = TextAlign::Start,
        .direction     = TextDirection::LeftToRight,
        .language      = {}};

    canvas.draw_text(text_block, text_layout, ctx.font_bundle, area.offset);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {}

  stx::String text;
  TextStyle   style;
  TextLayout  text_layout;
  bool        needs_relayout = true;
};

}        // namespace ash
