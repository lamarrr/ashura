#include <memory>

#include "stx/option.h"
#include "vlk/ui/event.h"

namespace vlk {
namespace ui {

struct WindowApiHandle;

// not thread-safe, only one instance should be possible
// TODO(lamarrr): setup loggers, this needs a window api logger
struct WindowApi {
  // TODO(lamarrr): check to ensure we only have one instance
  static WindowApi init();

  void poll_events() const;

  std::shared_ptr<WindowApiHandle> handle;
};

}  // namespace ui
}  // namespace vlk
