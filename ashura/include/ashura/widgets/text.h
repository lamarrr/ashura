#pragma once

#include "ashura/font.h"
#include "ashura/text.h"
#include "ashura/widget.h"

namespace ash
{

struct Text : public Widget
{
  explicit Text(std::string_view itext) :
      text{stx::string::make(stx::os_allocator, itext).unwrap()}, props{}
  {}

  Text(std::string_view itext, TextProps iprops) :
      text{stx::string::make(stx::os_allocator, itext).unwrap()}, props{iprops}
  {}

  virtual WidgetInfo get_info(Context &context) override
  {
    return WidgetInfo{.type = "Text"};
  }

  virtual Layout layout(Context &context, rect area) override
  {
    TextRun runs[] = {
        TextRun{.text = text}};

    Paragraph paragraph{
        .runs  = runs,
        .props = props,
        .align = TextAlign::Left};

    text_layout.layout(paragraph, context.font_bundle, area.extent.x);

    return Layout{.area = rect{.offset = area.offset, .extent = text_layout.span}};
  }

  virtual void draw(Context &context, gfx::Canvas &canvas, rect area) override
  {
    TextRun runs[] = {
        TextRun{.text = text}};

    Paragraph paragraph{
        .runs  = runs,
        .props = props,
        .align = TextAlign::Left};

    canvas.draw_text(paragraph, text_layout, context.font_bundle, area.offset);
  }

  virtual void tick(Context &context, std::chrono::nanoseconds interval) override
  {}

  stx::String text;
  TextProps   props;
  TextLayout  text_layout;
};

}        // namespace ash
