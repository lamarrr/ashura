#pragma once

#include "ashura/std/fn.hpp"
#include "ashura/std/span.hpp"

namespace ash
{

using StackTraceFn = Fn<void(Str module_name, usize i, Str function_name, void * addr)>;

void stacktrace(StackTraceFn callback);

}    // namespace ash
