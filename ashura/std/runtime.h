#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/types.h"
#include <atomic>

namespace ash
{
typedef void (*PanicHandler)();
}

extern ash::PanicHandler const  panic_handler;
extern std::atomic<ash::u64>    panic_count;
extern ash::AllocatorImpl const default_allocator;