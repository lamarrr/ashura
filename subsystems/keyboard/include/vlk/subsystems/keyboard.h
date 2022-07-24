#pragma once

#include <chrono>
#include <utility>

#include "SDL.h"
#include "SDL_keyboard.h"
#include "stx/fn.h"
#include "stx/vec.h"
#include "vlk/subsystem.h"

namespace vlk {

struct KeyboardEvent {
  SDL_Scancode scan_code{SDL_SCANCODE_UNKNOWN};
  SDL_Keycode key_code{SDLK_UNKNOWN};
  SDL_Keymod modifier{KMOD_NONE};  // bitwise OR'd together
  bool repeated = false;
  enum class State { Released, Pressed } state = State::Pressed;
};

struct Keyboard : public Subsystem {
  explicit Keyboard(stx::Allocator allocator) : listeners_{allocator} {}

  virtual void tick(std::chrono::nanoseconds) {}

  void listen(stx::RcFn<void(KeyboardEvent)> callback) {
    listeners_ =
        stx::vec::push(std::move(listeners_), std::move(callback)).unwrap();
  }

  void __fire();

  stx::Vec<stx::RcFn<void(KeyboardEvent)>> listeners_;
};

}  // namespace vlk
