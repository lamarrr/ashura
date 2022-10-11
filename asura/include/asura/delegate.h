#pragma once
#include <chrono>

#include "stx/fn.h"
#include "stx/vec.h"

// thread-safety? only called in tick
// TODO(lamarrr): we don't need to store
template <typename... T>
struct MulticastDelegate {
  stx::Vec<stx::UniqueFn<void(T const&...)>> listeners{stx::os_allocator};
  stx::Vec<std::tuple<T...>> events{stx::os_allocator};

  void broadcast(T... t) { events.push(std::make_tuple(t...)).unwrap(); }

  void listen(stx::UniqueFn<void(T const&...)> listener) {
    listeners.push(std::move(listener)).unwrap();
  }

  void dispatch() {
    for (auto const& event : events) {
      for (auto& listener : listeners) std::apply(listener.handle, event);
    }
    events.clear();
  }
};

template<>
struct MulticastDelegate<void> {
  stx::Vec<stx::UniqueFn<void()>> listeners{stx::os_allocator};
  uint64_t num_events_ = 0;

  void broadcast() { num_events_++; }

  void listen(stx::UniqueFn<void()> listener) {
    listeners.push(std::move(listener)).unwrap();
  }

  void dispatch() {
    for (uint64_t i = 0; i < num_events_; i++) {
      for (auto& listener : listeners) listener.handle();
    }
    num_events_ = 0;
  }
};
