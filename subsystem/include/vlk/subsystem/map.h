#pragma once

#include <functional>
#include <map>
#include <string>

#include "vlk/subsystem/impl.h"

namespace vlk {

struct SubsystemImplInfo {
  stx::Rc<SubsystemImpl *> impl;
  stx::FutureAny cancelation_future;
};

// stable address map
using SubsystemsMap = std::map<std::string, SubsystemImplInfo, std::less<>>;
}  // namespace vlk
