#pragma once

#include "ashura/engine/widget.h"

namespace ash
{

struct WidgetSystem
{
  WidgetContext ctx    = {};
  Canvas        canvas = {};

  void frame(Widget *root)
  {
    if (root == nullptr)
    {
      return;
    }

    // build tree
    // hit testing, dispatch events
    // tick
    // layout: fit and position, stack, clip
    // render
  }
};

}        // namespace ash