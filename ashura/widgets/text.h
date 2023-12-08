#pragma once

#include "ashura/font.h"
#include "ashura/text.h"
#include "ashura/widget.h"

namespace ash
{
namespace gui
{

struct TextProps
{
  TextStyle    style;
  Constraint2D frame = Constraint2D::relative(1, 1);
};

struct Text : public Widget
{
  explicit Text(std::string_view itext, TextProps iprops = {}) :
      text{stx::string::make(stx::os_allocator, itext).unwrap()}, props{iprops}
  {
  }

  virtual WidgetDebugInfo get_debug_info(Context &ctx) override
  {
    return WidgetDebugInfo{.type = "Text"};
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
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

      Vec2 size = props.frame.resolve(allocated_size);

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

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
  }

  void update_text(std::string_view itext, TextProps iprops)
  {
    text            = stx::string::make(stx::os_allocator, itext).unwrap();
    props           = iprops;
    is_layout_dirty = true;
  }

  stx::String text;
  TextProps   props;
  TextLayout  text_layout;
  bool        is_layout_dirty = true;
};

}        // namespace gui
}        // namespace ash
