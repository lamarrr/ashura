#include "vlk/ui/widget.h"

#include "fmt/format.h"

std::string vlk::ui::format(Widget const &widget) {
  vlk::ui::WidgetDebugInfo const debug_info = widget.get_debug_info();
  return fmt::format("Widget: {} (type hint: {}, address: {})", debug_info.name,
                     debug_info.type_hint, (void *)&widget);
}

std::string vlk::ui::operator>>(stx::ReportQuery,
                                     vlk::ui::Widget const &widget) {
  return vlk::ui::format(widget);
}
