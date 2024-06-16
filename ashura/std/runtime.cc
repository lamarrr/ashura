#include "ashura/std/runtime.h"

std::atomic<ash::u64> panic_count = 0;

void custom_panic_handler();

ash::PanicHandler const panic_handler = custom_panic_handler;

void custom_panic_handler()
{
  // no-op
}
