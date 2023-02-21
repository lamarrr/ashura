#pragma once
#include "ashura/primitives.h"
#include "stx/memory.h"
#include "stx/span.h"

namespace ash {
namespace gfx {

/// NOTE: resource image with index 0 must be a transparent white image
using image = u64;

}  // namespace gfx

struct RgbaImageBuffer {
  stx::Memory memory;
  extent extent;

  stx::Span<u8 const> span() const {
    return stx::Span{AS(u8*, memory.handle), extent.area() * 4};
  }
};

}  // namespace ash
