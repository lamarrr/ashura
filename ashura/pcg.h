#pragma once
#include "ashura/primitives.h"

namespace ash
{

/// Permuted Congruential Renerator.
/// GPU/Multithreaded-compatible PRNG and Hash function.
/// See: https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
/// 32-bit “RXS-M-XS” PCG
constexpr u32 pcg32(u32 input)
{
  u32 state = input * 747796405u + 2891336453u;
  u32 word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
  return (word >> 22u) ^ word;
}

/// Super-fast PCG random number generator.
struct Pcg32Rng
{
  u32 state = 0;        // RNG state.  All values are possible.

  constexpr u32 generate()
  {
    u32 const old_state = state;
    state               = old_state * 747796405u + 2891336453u;
    u32 word =
        ((old_state >> ((old_state >> 28u) + 4u)) ^ old_state) * 277803737u;
    return (word >> 22u) ^ word;
  }
};

}        // namespace ash
