#pragma once

#include <cinttypes>

namespace vlk {

struct Asset {
  explicit constexpr Asset(uint64_t size_in_bytes = 0)
      : size_bytes_{size_in_bytes} {}

  virtual ~Asset() {}

  constexpr uint64_t size_bytes() const { return size_bytes_; }

 protected:
  uint64_t size_bytes_ = 0;
};

};  // namespace vlk
