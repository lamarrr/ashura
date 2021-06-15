#include "vlk/ui/font.h"

#include "fmt/format.h"

namespace vlk {
namespace ui {

std::string format(FontStyle style) {
  return fmt::format("weight: {}, width: {}, slant: {}", format(style.weight),
                     format(style.width), format(style.slant));
}

}  // namespace ui
}  // namespace vlk
