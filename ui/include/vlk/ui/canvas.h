#pragma once

#include <type_traits>
#include "vlk/ui/primitives.h"

struct SkCanvas;

namespace vlk {
namespace ui {

/// a view over a specific canvas implementation. Only supports Skia (with no
/// implementation abstraction) at the moment.
struct Canvas {
    explicit constexpr Canvas(SkCanvas* pimpl, Extent const& extent)
      : pimpl_{pimpl}, extent_{extent} {}

  static constexpr Canvas from_skia(SkCanvas* pimpl, Extent const& extent) {
    return Canvas{pimpl, extent};
  }

  constexpr SkCanvas* as_skia() const { return static_cast<SkCanvas*>(pimpl_); }

  constexpr Extent extent() const { return extent_; }

 private:
  void* pimpl_;
  Extent extent_;
};

}  // namespace ui
}  // namespace vlk
