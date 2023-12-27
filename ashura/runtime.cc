#include "ashura/runtime.h"

std::atomic<ash::u64> panic_count = 0;

void custom_panic_handler();

PFN_panic_handler panic_handler = custom_panic_handler;

void custom_panic_handler()
{
  // no-op
  // TODO(lamarrr): print backtrace without using memory
}