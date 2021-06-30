
#include "vlk/ui/window_api.h"

#include <map>

#include "vlk/ui/window_api_handle.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

WindowApi WindowApi::init() {
  WindowApi api{};
  api.handle = std::shared_ptr<WindowApiHandle>(new WindowApiHandle{});

  return std::move(api);
}

void WindowApi::poll_events() const { handle->poll_events(); }

}  // namespace ui
}  // namespace vlk
