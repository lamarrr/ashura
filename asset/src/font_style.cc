#include "vlk/font_style.h"

#include "fmt/format.h"

namespace vlk {
std::string format(FontStyle style) {
  return fmt::format("weight: {}, width: {}, slant: {}", format(style.weight),
                     format(style.width), format(style.slant));
}

}  // namespace vlk