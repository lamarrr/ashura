#pragma once

#include <chrono>

#include "vlk/primitives.h"

namespace vlk {
namespace ui {
// global specific events
// widget specific events via subscription to optimize calculations

}  // namespace ui
}  // namespace vlk

/*
enum class ScreenOrientation { Portrait, Landscape };

struct ScreenState {
  Extent extent;

  constexpr ScreenOrientation orientation() const noexcept {
    return extent.height >= extent.width ? ScreenOrientation::Portrait
                                         : ScreenOrientation::Landscape;
  }

  float aspect_ratio() const noexcept {
    return static_cast<float>(extent.width) / static_cast<float>(extent.height);
  }
};

struct Context {
  ScreenState screen_state;
};
*/

// for UI related processing, all other-processing should be done
// asynchronously else will block the rendering loop
// similar to Unreal Engine's `FTickable::Tick(float)`
// [====*====] events will of course be forwarded on every tick. there's
// ADD  a new always valid event named std::chrono::nanoseconds;
// technically no reason for splitting this from the event handler and having
// the renderer use it.
// virtual void tick([[maybe_unused]] std::chrono::nanoseconds const
// &interval) { no-op
// }
