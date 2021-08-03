
#pragma once

#include "include/core/SkRect.h"
#include "vlk/primitives.h"

namespace vlk {

constexpr SkRect to_sk_rect(Rect const& rect) {
  return SkRect::MakeXYWH(rect.x(), rect.y(), rect.width(), rect.height());
}

constexpr SkRect to_sk_rect(IRect const& rect) {
  return SkRect::MakeXYWH(rect.x(), rect.y(), rect.width(), rect.height());
}

constexpr SkRect to_sk_rect(VRect const& rect) {
  return SkRect::MakeXYWH(rect.offset.x, rect.offset.y, rect.extent.width,
                          rect.extent.height);
}

inline Rect to_vlk_rect(SkRect const& rect) {
  auto x = static_cast<int32_t>(rect.x());
  auto y = static_cast<int32_t>(rect.y());
  auto w = static_cast<int32_t>(rect.width());
  auto h = static_cast<int32_t>(rect.height());

  VLK_ENSURE(fits_u32(x));
  VLK_ENSURE(fits_u32(y));
  VLK_ENSURE(fits_u32(w));
  VLK_ENSURE(fits_u32(h));

  return Rect{Offset{static_cast<uint32_t>(x), static_cast<uint32_t>(y)},
              Extent{static_cast<uint32_t>(w), static_cast<uint32_t>(h)}};
}

inline IOffset to_vlk_ioffset(SkRect const& rect) {
  auto x = static_cast<int32_t>(rect.x());
  auto y = static_cast<int32_t>(rect.y());

  return IOffset{x, y};
}

inline Extent to_vlk_extent(SkRect const& rect) {
  auto w = static_cast<int32_t>(rect.width());
  auto h = static_cast<int32_t>(rect.height());

  VLK_ENSURE(fits_u32(w));
  VLK_ENSURE(fits_u32(h));

  return Extent{static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
}

inline IRect to_vlk_irect(SkRect const& rect) {
  return IRect{to_vlk_ioffset(rect), to_vlk_extent(rect)};
}

}  // namespace vlk
