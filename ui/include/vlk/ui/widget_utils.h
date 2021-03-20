
#include <initializer_list>
#include <vector>

#include "stx/option.h"
#include "stx/span.h"

#include <cstdint>

namespace vlk {
namespace ui {

struct Widget;

/// builds a list of widgets, the function is called with the index, until it
/// returns `stx::None`
using WidgetBuilder = std::function<stx::Option<Widget *>(size_t)>;

// each widget accepting children args must have each of these.
// Widget::Builder, stx::Span<Widget *>, std::initializer_list<Widget*>
inline std::vector<Widget *> build_children(
    stx::Span<Widget *const> const &src_children) {
  std::vector<Widget *> children;
  children.resize(src_children.size());
  std::copy(src_children.begin(), src_children.end(), children.begin());
  return children;
}

inline std::vector<Widget *> build_children(
    std::initializer_list<Widget *const> const &src_children) {
  return build_children(
      stx::Span<Widget *const>(src_children.begin(), src_children.end()));
}

inline std::vector<Widget *> build_children(WidgetBuilder &builder) {
  std::vector<Widget *> children;
  size_t i = 0;
  while (auto child = builder(i)) {
    children.push_back(child.value());
    i++;
  }
  return children;
}

}  // namespace ui
}  // namespace vlk
