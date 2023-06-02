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

  virtual WidgetInfo get_info() override
  {
    return WidgetInfo{.type = "Text"};
  }

  virtual Layout layout(rect area) override
  {
    //
  }

  virtual void draw(gfx::Canvas &canvas, rect area) override
  {
    // TODO(lamarrr): script and others
    // canvas.draw
  }

  virtual void tick(Context &context, std::chrono::nanoseconds interval) override
  {}

  // on_mouse_down
  // on_mouse_up

  virtual simdjson::dom::element save(Context         &context,
                                      simdjson::dom::parser &parser) override
  {}

  virtual void restore(Context                &context,
                       simdjson::dom::element const &element) override
  {}

  stx::String text;
  TextProps   props;
};

}        // namespace ash
