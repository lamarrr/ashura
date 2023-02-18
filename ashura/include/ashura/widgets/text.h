#pragma once

#include "ashura/font.h"
#include "ashura/widget.h"

namespace ash {

// TODO(lamarrr): font family with font provider?
struct Text : public Widget {
  explicit Text(std::string_view itext)
      : text{stx::string::make(stx::os_allocator, itext).unwrap()}, style{} {}

  Text(TextStyle istyle, std::string_view itext)
      : text{stx::string::make(stx::os_allocator, itext).unwrap()},
        style{istyle} {}

  virtual WidgetInfo get_info() override {
    return WidgetInfo{.type = "Text", .id = Widget::id};
  }

  virtual Layout layout(rect area) override {
    //
  }

  virtual void draw(gfx::Canvas& canvas, rect area) override {
    // TODO(lamarrr): script and others
    TextRun runs = {{.text = text, .font = 0, .style = style}};
    Paragraph paragraph{
        .runs = runs, .align = TextAlign::Left, .overflow = TextOverflow::None};
    canvas.draw
  }

  virtual void tick(WidgetContext& context,
                    std::chrono::nanoseconds interval) override {}

  // on_mouse_down
  // on_mouse_up

  virtual simdjson::dom::element save(simdjson::dom::parser& parser) override {}

  virtual void restore(simdjson::dom::element const& element) override {}

  TextStyle style;
  stx::String text;
  gfx::Canvas layout_canvas{vec2{0, 0}};
};

}  // namespace ash
