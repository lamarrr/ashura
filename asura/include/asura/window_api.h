#pragma once

#include <memory>

#include "stx/option.h"

namespace asr {
namespace ui {

struct WindowApiHandle;

// not thread-safe, only one instance should be possible
// TODO(lamarrr): setup loggers, this needs a window api logger
struct WindowApi {
  // TODO(lamarrr): check to ensure we only have one instance
  static WindowApi init();

  bool poll_events() const;

  std::shared_ptr<WindowApiHandle> handle;
};

}  // namespace ui
}  // namespace asr
