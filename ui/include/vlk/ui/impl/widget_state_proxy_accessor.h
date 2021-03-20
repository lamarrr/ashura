#pragma once

#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

struct WidgetStateProxyAccessor {
  static Widget::StateProxy &access(Widget &widget) {
    return widget.state_proxy_;
  }
};

}  // namespace ui
}  // namespace vlk
