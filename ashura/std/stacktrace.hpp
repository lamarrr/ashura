#pragma once

#include "ashura/std/fn.hpp"
#include "ashura/std/span.hpp"
#include "ashura/std/result.hpp"

namespace ash
{

Result<> walk_stack_functions(Fn<void(Str, usize)> callback);


}    // namespace ash
