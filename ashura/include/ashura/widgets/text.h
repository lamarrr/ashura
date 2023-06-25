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

  virtual Layout layout(Context &ctx, rect area) override
  {
    TextRun runs[] = {
        TextRun{.text = text}};

    Paragraph paragraph{
        .runs  = runs,
        .props = props,
        .align = TextAlign::Left};

    text_layout.layout(paragraph, ctx.font_bundle, area.extent.x);

    return Layout{.area = area.with_extent(text_layout.span)};
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    TextRun runs[] = {
        TextRun{.text = text}};

    Paragraph paragraph{
        .runs  = runs,
        .props = props,
        .align = TextAlign::Left};

    canvas.draw_text(paragraph, text_layout, context.font_bundle, area.offset);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {}

  stx::String text;
  TextProps   props;
  TextLayout  text_layout;
};

}        // namespace ash
