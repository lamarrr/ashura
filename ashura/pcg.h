#pragma once
#include "ashura/primitives.h"

namespace ash
{
constexpr u32 DEFAULT_PCG32_MULTIPLIER = 747796405U;
constexpr u32 DEFAULT_PCG32_INCREMENT  = 2891336453U;
constexpr u32 DEFAULT_PCG32_SEED       = 0x46b56677U;

/// Permuted Congruential Renerator.
/// GPU/Multithreaded-compatible PRNG and Hash function.
/// See: https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
/// 32-bit “RXS-M-XS” PCG
///
/// https://github.com/imneme/pcg-c/blob/83252d9c23df9c82ecb42210afed61a7b42402d7/include/pcg_variants.h#L182
///
/// given the current machine state, generate a random value. i.e. maps a linear
/// state to a non-linear/randomised value
///
constexpr u32 pcg32_rxs_m_xs(u32 state)
{
  u32 word = ((state >> ((state >> 28U) + 4U)) ^ state) * 277803737U;
  return (word >> 22U) ^ word;
}

/// linearly step/change the state of the machine
constexpr u32 pcg32_step(u32 state)
{
  u32 new_state = state * DEFAULT_PCG32_MULTIPLIER + DEFAULT_PCG32_INCREMENT;
  return new_state;
}

/// given a linear u32 input, map it randomly to the u32 value range
constexpr u32 pcg32(u32 input)
{
  u32 state = pcg32_step(input);
  return pcg32_rxs_m_xs(state);
}

constexpr u32 pcg32_generate(u32 &state)
{
  u32 output = pcg32_rxs_m_xs(state);
  state      = pcg32_step(state);
  return output;
}

/// use a PCG hash as the seed/state for generating the next hash value
constexpr u32 pcg32_combine(u32 pcg0, u32 input)
{
  u32 state = pcg32_step(pcg0 + input);
  return pcg32_rxs_m_xs(state);
}

constexpr u32 pcg32_hash_bytes(void const *ptr, usize size)
{
  // get bytes
  u8 const * bytes = (u8 const*)ptr;
  u32 pcg = DEFAULT_PCG32_SEED;
  // TODO(lamarrr): implement packed loads
  // for all u32 packs
  // pcg= pcg32_combine(pcg, pack);
  return pcg;
}

/// Super-fast PCG random number generator.
struct Pcg32Rng
{
  // RNG state/seed. can be set to any value
  u32 state = DEFAULT_PCG32_SEED;

  constexpr u32 generate()
  {
    return pcg32_generate(state);
  }
};

}        // namespace ash
