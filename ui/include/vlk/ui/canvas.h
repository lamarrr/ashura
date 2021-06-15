#pragma once

#include <type_traits>

#include "include/core/SkCanvas.h"
#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

//! a wrapper over the Skia canvas
struct Canvas {
  explicit constexpr Canvas(SkCanvas& canvas, Extent const& extent)
      : canvas_addr_{&canvas}, extent_{extent} {}

  static constexpr Canvas from_skia(SkCanvas& pimpl, Extent const& extent) {
    return Canvas{pimpl, extent};
  }

  SkCanvas& to_skia() const { return *canvas_addr_; }

  constexpr Extent extent() const { return extent_; }

 private:
  SkCanvas* canvas_addr_;
  Extent extent_;
};

}  // namespace ui
}  // namespace vlk
