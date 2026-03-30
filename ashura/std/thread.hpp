/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/span.hpp"

namespace ash
{

void set_thread_name(Str name);

Str get_thread_name();

}    // namespace ash
