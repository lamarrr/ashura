#pragma once

#include <map>
#include <memory>

#include "asura/utils.h"
#include "asura/window_event_queue.h"
#include "stx/option.h"
#include "stx/rc.h"

namespace asr {

enum class WindowID : uint32_t {};

// not thread-safe, only one instance should be possible
// TODO(lamarrr): setup loggers, this needs a window api logger

// this also dispatches events to the created windows
struct WindowApi {
  struct WindowInfo {
    // queue to receive events from
    WindowEventQueue* queue = nullptr;
  };

  WindowApi();
  STX_DISABLE_COPY(WindowApi)
  STX_DISABLE_MOVE(WindowApi)
  ~WindowApi();

  void add_window_info(WindowID id, WindowInfo info) {
    windows_info_.emplace(id, info);
  }

  WindowInfo get_window_info(WindowID id) {
    auto window_entry_pos = windows_info_.find(id);
    ASR_ENSURE(window_entry_pos != windows_info_.end());

    return window_entry_pos->second;
  }

  // must be a valid window registered with this api and added with the
  // 'add_window_info' method
  void remove_window_info(WindowID id) {
    auto pos = windows_info_.find(id);
    ASR_ENSURE(pos != windows_info_.end());
    windows_info_.erase(pos);
  }

  bool poll_events();

  // this implies that we need to detach the handle from here first
  //
  // also ensure all api calls occur on the main thread.
  std::map<WindowID, WindowInfo> windows_info_;
};


}  // namespace asr
