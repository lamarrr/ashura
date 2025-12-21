// SPDX-License-Identifier: MIT
#include "ashura/std/types.h"
#include "gtest/gtest.h"

using namespace ash;

static_assert(ring_sub(1U, 5U, 20U) == 16U, "");
static_assert(ring_sub(0U, 5U, 1U) == 0U, "");
static_assert(ring_add(0U, 2U, 1U) == 0U, "");
static_assert(ring_add(8U, 21U, 20U) == 9U, "");
static_assert(ring_add(8U, 21U, 20U) == 9U, "");
