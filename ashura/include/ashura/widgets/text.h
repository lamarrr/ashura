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

  virtual WidgetDebugInfo get_debug_info(Context &ctx) override
  {
    return WidgetDebugInfo{.type = "Text"};
  }

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    if (needs_relayout)
    {
      spdlog::info("re-laid out text");
      TextRun runs[] = {
          TextRun{.text = text}};

      Paragraph paragraph{
          .runs  = runs,
          .props = props,
          .align = TextAlign::Left};

      text_layout.layout(paragraph, ctx.font_bundle, allocated_size.x);
      needs_relayout = false;
    }

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
  bool        needs_relayout = true;
};

}        // namespace ash
