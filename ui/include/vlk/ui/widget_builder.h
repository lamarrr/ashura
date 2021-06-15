#pragma once

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <initializer_list>
#include <vector>

#include "stx/option.h"
#include "stx/span.h"

namespace vlk {
namespace ui {

struct Widget;

//! each widget accepting children could use these helper functions:
//! they should have function overloads with: `WidgetBuilder`, `stx::Span<Widget
//! *>`, `std::initializer_list<Widget*>`, and `std::vector<Widget*>`
//!
//! builds a list of widgets. the function is called with the index, until it
//! returns `nullptr`
using WidgetBuilder = std::function<Widget *(size_t)>;

inline std::vector<Widget *> build_children(
    stx::Span<Widget *const> const &src_children) {
  std::vector<Widget *> children;
  children.resize(src_children.size());
  std::copy(src_children.begin(), src_children.end(), children.begin());
  return children;
}

inline std::vector<Widget *> build_children(
    std::initializer_list<Widget *const> const &src_children) {
  return build_children(stx::Span<Widget *const>(src_children));
}

inline std::vector<Widget *> build_children(WidgetBuilder &builder) {
  // l-value ref argument cos the builder could possibly have internal state
  std::vector<Widget *> children;
  size_t i = 0;
  while (auto child = builder(i)) {
    children.push_back(child);
    i++;
  }
  return children;
}

}  // namespace ui
}  // namespace vlk
