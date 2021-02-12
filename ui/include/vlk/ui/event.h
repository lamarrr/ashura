#pragma once

#include <chrono>

#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

struct GlobalEventCapture {
  std::chrono::nanoseconds interval;

  struct ScreenEvent {
    enum Action { kNoneBit = 0, kResizedBit = 1, kScrollingBit = 2 } actions;
    Extent extent;

    constexpr bool resized() const noexcept { return actions & kResizedBit; }
    constexpr bool scrolling() const noexcept {
      return actions & kScrollingBit;
    }
  };

  struct MouseEvent {
    enum Action { kNoneBit = 0, kClickedBit = 1, kReleasedBit = 2 } actions;
    // relative to the framebuffer's dimensions
    RelativeOffset position;

    constexpr bool pressed() const noexcept { return actions & kClickedBit; }
    constexpr bool released() const noexcept { return actions & kReleasedBit; }
  };
};

struct WidgetEventCapture {
  struct MouseEvent {
    enum Action {
      kNone = 0,
      kClickedBit = 1,
      kHoveredBit = 2,
    } actions;

    constexpr bool clicked() const noexcept { return actions & kClickedBit; }
    constexpr bool hovered() const noexcept { return actions & kHoveredBit; }
  };
};

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
// TODO (lamarrr): events will of course be forwarded on every tick. there's
// ADD  a new always valid event named std::chrono::nanoseconds;
// technically no reason for splitting this from the event handler and having
// the renderer use it.
// virtual void tick([[maybe_unused]] std::chrono::nanoseconds const
// &interval) { no-op
// }
