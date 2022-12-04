#include <unwind.h>

#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <iterator>

using frame_ptr = void*;

struct unwind_state {
  size_t frames_to_skip;
  frame_ptr* current;
  frame_ptr* end;
};

inline _Unwind_Reason_Code unwind_callback(::_Unwind_Context* context,
                                           void* arg) {
  // Note: do not write `::_Unwind_GetIP` because it is a macro on some
  // platforms. Use `_Unwind_GetIP` instead!
  unwind_state* const state = static_cast<unwind_state*>(arg);
  if (state->frames_to_skip) {
    --state->frames_to_skip;
    return _Unwind_GetIP(context) ? _URC_NO_REASON : _URC_END_OF_STACK;
  }

  *state->current = reinterpret_cast<frame_ptr>(_Unwind_GetIP(context));

  ++state->current;
  if (!*(state->current - 1) || state->current == state->end) {
    return _URC_END_OF_STACK;
  }
  return _URC_NO_REASON;
}

size_t collect(frame_ptr* out_frames, size_t max_frames_count, size_t skip) {
  size_t frames_count = 0;
  if (!max_frames_count) {
    return frames_count;
  }
  skip += 1;

  unwind_state state = {skip, out_frames, out_frames + max_frames_count};
  _Unwind_Backtrace(&unwind_callback, &state);
  frames_count = state.current - out_frames;

  if (frames_count && out_frames[frames_count - 1] == 0) {
    --frames_count;
  }

  return frames_count;
}

int main() {
  frame_ptr frames[200];
  collect(frames, std::size(frames), 0);
}