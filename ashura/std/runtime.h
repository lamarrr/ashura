#pragma once
#include "ashura/std/types.h"
#include <atomic>

typedef void (*PFN_panic_handler)();

extern PFN_panic_handler     panic_handler;
extern std::atomic<ash::u64> panic_count;
