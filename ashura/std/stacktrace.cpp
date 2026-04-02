/// SPDX-License-Identifier: MIT
#include "ashura/std/stacktrace.hpp"

#include "ashura/std/cfg.hpp"

#if ASH_CFG(OS, LINUX) && ASH_CFG(BINARY, ELF) && \
  (ASH_CFG(ARCH, X86_64) || ASH_CFG(ARCH, ARM64))

#else

#endif
