#pragma once
#include <cinttypes>
#include <utility>

#include "stx/async.h"
#include "stx/fn.h"

namespace stx {

struct ForState {
  // only valid if `next` < `end`
  stx::ServiceToken service_token{};
  int64_t next = 0;
};

// this task completes when its state's `next` index points to the specified
// `end`. if suspension/cancelation is requested before the above condition,
// then the task will suspend/cancel and update its state.
//
template <typename Fn>
struct For {
  static_assert(std::is_invocable_v<Fn &, int64_t>,
                "function must be invocable with the for-loop's index");

  using function_type = stx::raw_function_decay<Fn>;

  constexpr For(int64_t ibegin, int64_t iend, Fn &&ifn)
      : begin{ibegin}, end{iend}, fn{std::move(ifn)} {}

  For(For const &) = delete;
  For &operator=(For const &) = delete;
  For(For &&) = default;
  For &operator=(For &&) = default;

  void resume(ForState &state, stx::RequestProxy const &proxy) {
    for (int64_t i = state.next; i < end; i++) {
      fn(state.next);
      state.next++;

      stx::CancelRequest const cancel_request = proxy.fetch_cancel_request();
      stx::SuspendRequest const suspend_request = proxy.fetch_suspend_request();

      if (cancel_request.state == stx::RequestedCancelState::Canceled) {
        state.service_token = stx::ServiceToken{cancel_request};
        return;
      }

      if (suspend_request.state == stx::RequestedSuspendState::Suspended) {
        state.service_token = stx::ServiceToken{suspend_request};
        return;
      }
    }
  }

  int64_t begin = 0;
  int64_t end = 0;
  function_type fn;
};

}  // namespace stx
