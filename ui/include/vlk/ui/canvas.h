#pragma once
#include "vlk/ui/primitives.h"

struct SkCanvas;

namespace vlk {
namespace ui {

struct [[nodiscard]] Canvas {
  explicit constexpr Canvas(SkCanvas * pimpl, Extent const& extent) noexcept
      : pimpl_{pimpl}, extent_{extent} {}

  static constexpr Canvas FromSkia(SkCanvas * pimpl,
                                   Extent const& extent) noexcept {
    return Canvas{pimpl, extent};
  }

  [[nodiscard]] constexpr SkCanvas* as_skia() noexcept {
    return static_cast<SkCanvas*>(pimpl_);
  }

  constexpr Extent extent() const noexcept { return extent_; }

 private:
  void* pimpl_;
  Extent extent_;
};

}  // namespace ui
}  // namespace vlk
