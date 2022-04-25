#pragma once

#include <type_traits>

#include "vlk/primitives.h"

class SkCanvas;

namespace vlk {
namespace ui {

/// a wrapper over the Skia canvas
struct Canvas {
  explicit constexpr Canvas(SkCanvas& canvas, Extent extent, Dpr dpr)
      : canvas_addr_{&canvas}, extent_{extent}, dpr_{dpr} {}

  static constexpr Canvas from_skia(SkCanvas& pimpl, Extent extent, Dpr dpr) {
    return Canvas{pimpl, extent, dpr};
  }

  SkCanvas& to_skia() const { return *canvas_addr_; }

  constexpr Extent extent() const { return extent_; }

  Dpr get_device_pixel_ratio() const { return dpr_; }

 private:
  SkCanvas* canvas_addr_;
  Extent extent_;
  Dpr dpr_;
};

}  // namespace ui
}  // namespace vlk
