#pragma once

#include "ashura/font.h"
#include "ashura/text.h"
#include "ashura/widget.h"

namespace ash
{

// TODO(lamarrr): font family with font provider?
struct Text : public Widget
{
  explicit Text(std::string_view itext) :
      text{stx::string::make(stx::os_allocator, itext).unwrap()}, props{}
  {}

  Text(TextProps iprops, std::string_view itext) :
      text{stx::string::make(stx::os_allocator, itext).unwrap()}, props{iprops}
  {}

  virtual WidgetInfo get_info(Context &context) override
  {
    return WidgetInfo{.type = "Text"};
  }

  virtual Layout layout(Context &context, rect area) override
  {
    //
  }

  virtual void draw(Context &context, gfx::Canvas &canvas, rect area) override
  {
    // TODO(lamarrr): script and others
    // canvas.draw
  }

  virtual void tick(Context &context, std::chrono::nanoseconds interval) override
  {}

  // on_mouse_down
  // on_mouse_up
  stx::String text;
  TextProps   props;
};

}        // namespace ash
