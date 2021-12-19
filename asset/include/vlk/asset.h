#pragma once

#include <cinttypes>

#include "stx/async.h"
#include "vlk/scheduler.h"
#include "vlk/subsystem/context.h"
#include "vlk/subsystem/impl.h"

namespace vlk {

using stx::Future;
using stx::Rc;

struct Asset {
  explicit constexpr Asset(uint64_t size_in_bytes = 0)
      : size_bytes_{size_in_bytes} {}

  virtual ~Asset() = 0;

  constexpr uint64_t size_bytes() const { return size_bytes_; }

 protected:
  uint64_t size_bytes_ = 0;
};


};  // namespace vlk
