#pragma once
#include <cinttypes>

namespace asr {

struct Version {
  uint8_t major = 0;
  uint8_t minor = 0;
  uint8_t patch = 0;
};

}  // namespace asr