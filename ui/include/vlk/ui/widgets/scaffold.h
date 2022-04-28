#pragma once

#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

struct Scaffold : public Widget {
  Scaffold() : Widget{} {
    Widget::update_self_extent(
        SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)});
    Widget::update_view_extent(
        ViewExtent{Constrain::relative(1.0f), Constrain::absolute(stx::u32_max)});
  }


  virtual void
};

}  // namespace ui
}  // namespace vlk