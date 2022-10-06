#pragma once

#include <chrono>

#include "stx/async.h"
#include "stx/fn.h"

namespace vlk {

// on_completed
template <typename T>
struct FutureAwaiter {
  enum class State { Pending, Completed };

  FutureAwaiter(stx::Future<T> future, stx::UniqueFn<void()> on_completed)
      : future_{std::move(future)}, on_completed_{std::move(on_completed)} {}

  void reset(stx::Future<T> future, stx::UniqueFn<void()> on_completed) {
    future_ = std::move(future);
    state_ = State::Pending;
    on_completed_ = std::move(on_completed);
  }

  void tick(std::chrono::nanoseconds) {
    switch (future_.fetch_status()) {
      case stx::FutureStatus::Completed:
        if (state_ == State::Pending) {
          state_ = State::Completed;
          on_completed_.handle();
        }
        break;
      default: {
      }
    }
  }

  stx::Future<T> future_;

  // i.e. call Widget::mark_render_dirty() once image is loaded
  stx::UniqueFn<void()> on_completed_ = stx::fn::rc::make_unique_static([] {});

  State state_ = State::Pending;
};
}  // namespace vlk