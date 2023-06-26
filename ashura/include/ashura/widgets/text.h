#pragma once

#include "ashura/font.h"
#include "ashura/text.h"
#include "ashura/widget.h"

namespace ash
{

struct Text : public Widget
{
  explicit Text(std::string_view itext, TextProps iprops = TextProps{}) :
      text{stx::string::make(stx::os_allocator, itext).unwrap()}, props{iprops}
  {}

  virtual WidgetInfo get_info(Context &ctx) override
  {
    return WidgetInfo{.type = "Text"};
  }

  virtual vec2 layout(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    TextRun runs[] = {
        TextRun{.text = text}};

    Paragraph paragraph{
        .runs  = runs,
        .props = props,
        .align = TextAlign::Left};

    text_layout.layout(paragraph, ctx.font_bundle, area.extent.x);

    return text_layout.span;
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    TextRun runs[] = {
        TextRun{.text = text}};

    Paragraph paragraph{
        .runs  = runs,
        .props = props,
        .align = TextAlign::Left};

    canvas.draw_text(paragraph, text_layout, ctx.font_bundle, area.offset);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {}

  stx::String text;
  TextProps   props;
  TextLayout  text_layout;
};

}        // namespace ash
