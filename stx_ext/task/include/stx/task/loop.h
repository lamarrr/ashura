#pragma once

#include <type_traits>
#include <utility>

#include "stx/async.h"
#include "stx/fn.h"

namespace stx {

struct LoopState {
  // only valid if `serviced`
  stx::ServiceToken service_token{};
  bool serviced = false;
};

// this task never completes until it is canceled.
// it can be suspended and canceled.
//
// once a suspend/cancel request comes in, it is serviced and the loop's state
// is updated accordingly
//
template <typename Fn>
struct Loop {
  static_assert(std::is_invocable_v<Fn &>,
                "function in loop must be invocable with no arguments");

  using function_type = stx::raw_function_decay<Fn>;

  explicit constexpr Loop(Fn &&ifn) : fn{std::move(ifn)} {}

  Loop(Loop const &) = delete;
  Loop &operator=(Loop const &) = delete;
  Loop(Loop &&) = default;
  Loop &operator=(Loop &&) = default;

  void resume(LoopState &state, stx::RequestProxy const &proxy) {
    while (true) {
      fn();

      stx::CancelRequest const cancel_request = proxy.fetch_cancel_request();
      stx::SuspendRequest const suspend_request = proxy.fetch_suspend_request();

      if (cancel_request.state == stx::RequestedCancelState::Canceled) {
        state.service_token = stx::ServiceToken{cancel_request};
        state.serviced = true;
        return;
      }

      if (suspend_request.state == stx::RequestedSuspendState::Suspended) {
        state.service_token = stx::ServiceToken{suspend_request};
        state.serviced = true;
        return;
      }
    }
  }

  function_type fn;
};

}  // namespace stx
