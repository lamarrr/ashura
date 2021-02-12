
#include <vector>

#include "stx/span.h"

#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

// each widget accepting children args must have each of these.
// Widget::Builder, stx::Span<Widget *>, std::initializer_list<Widget*>
inline void build_children(std::vector<Widget *> &children,
                           stx::Span<Widget *const> const &src_children) {
  children.resize(src_children.size());
  std::copy(src_children.begin(), src_children.end(), children.begin());
}

inline void build_children(
    std::vector<Widget *> &children,
    std::initializer_list<Widget *const> const &src_children) {
  build_children(children, stx::Span<Widget *const>(src_children.begin(),
                                                    src_children.end()));
}

inline void build_children(std::vector<Widget *> &children,
                           Widget::Builder &builder) {
  size_t i = 0;
  while (auto child = builder(i)) {
    children.push_back(child.value());
    i++;
  }
}

}  // namespace ui
}  // namespace vlk
