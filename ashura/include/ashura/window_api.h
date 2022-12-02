#pragma once

#include <map>
#include <memory>

#include "ashura/event.h"
#include "ashura/utils.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/vec.h"

namespace asr {

enum class WindowID : u32 {};

struct Window;

// not thread-safe, only one instance should be possible
// TODO(lamarrr): setup loggers, this needs a window api logger

// this also dispatches events to the created windows
struct WindowApi {
  WindowApi();

  STX_MAKE_PINNED(WindowApi)

  ~WindowApi();

  void add_window_info(WindowID id, Window *win) {
    windows_info_.emplace(id, win);
  }

  Window *get_window_info(WindowID id) {
    auto window_entry_pos = windows_info_.find(id);
    ASR_CHECK(window_entry_pos != windows_info_.end());

    return window_entry_pos->second;
  }

  // must be a valid window registered with this api and added with the
  // 'add_window_info' method
  void remove_window_info(WindowID id) {
    auto pos = windows_info_.find(id);
    ASR_CHECK(pos != windows_info_.end());
    windows_info_.erase(pos);
  }

  // polls for events, returns true if an event occured, otherwise false
  bool poll_events();

  // this implies that we need to detach the handle from here first
  //
  // also CHECK all api calls occur on the main thread.
  std::map<WindowID, Window *> windows_info_;
};

}  // namespace asr
