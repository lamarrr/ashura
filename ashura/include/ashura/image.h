#pragma once
#include "stx/rc.h"

namespace asr {

namespace vk {
struct ImageSampler;
}

namespace gfx {
// stored in vulkan context
using Image = stx::Rc<vk::ImageSampler*>;
}  // namespace gfx

}  // namespace asr
