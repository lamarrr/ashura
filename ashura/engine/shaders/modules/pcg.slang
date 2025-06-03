/// SPDX-License-Identifier: MIT

#pragma once

#include "types.slang"

namespace pcg
{

// clang-format off
u32 hash(u32 state)
{
  state     = state * 747796405U + 2891336453U;
  u32 word = ((state >> ((state >> 28U) + 4U)) ^ state) * 277803737U;
  return (word >> 22U) ^ word;
}

// clang-format on

}    // namespace pcg
