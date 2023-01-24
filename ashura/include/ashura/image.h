#pragma once
#include "ashura/primitives.h"
#include "stx/memory.h"

namespace asr {
namespace gfx {

/// NOTE: resource image with index 0 must be a transparent white image
using image = u64;

struct RgbaImageBuffer {
  stx::Memory memory;
  extent extent;
};

}  // namespace gfx
}  // namespace asr
