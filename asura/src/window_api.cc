
#include "asura/ui/window_api.h"

#include <map>

#include "asura/ui/window_api_handle.h"
#include "asura/utils.h"

namespace asr {
namespace ui {

WindowApi WindowApi::init() {
  WindowApi api{};
  api.handle = std::shared_ptr<WindowApiHandle>(new WindowApiHandle{});

  api.handle->init();

  return std::move(api);
}

bool WindowApi::poll_events() const { return handle->poll_events(); }

}  // namespace ui
}  // namespace asr
