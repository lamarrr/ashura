#pragma once

#include "stx/option.h"

namespace vlk {
template <typename Target, typename Source>
inline stx::Option<Target*> upcast(Source& source) {
  Target* upcast_ptr = dynamic_cast<Target*>(&source);
  if (upcast_ptr == nullptr) {
    return stx::None;
  } else {
    return stx::Some(static_cast<Target*>(upcast_ptr));
  }
}

struct Subsystem {
  template <typename Target>
  stx::Option<Target*> as() {
    return upcast<Target, Subsystem>(*this);
  }

  template <typename Target>
  stx::Option<Target const*> as() const {
    return upcast<Target const, Subsystem const>(*this);
  }
};

}  // namespace vlk
