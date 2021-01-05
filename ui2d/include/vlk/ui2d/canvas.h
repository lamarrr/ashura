#pragma once

struct SkCanvas;

namespace vlk {
namespace ui2d {

struct Canvas {
  explicit constexpr Canvas(SkCanvas* pimpl) noexcept : pimpl_{pimpl} {}

  static constexpr Canvas FromSkia(SkCanvas* pimpl) noexcept {
    return Canvas{pimpl};
  }

  constexpr SkCanvas* as_skia() noexcept {
    return static_cast<SkCanvas*>(pimpl_);
  }

 private:
  void* pimpl_;
};

}  // namespace ui2d
}  // namespace vlk
