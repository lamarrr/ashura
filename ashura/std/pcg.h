#pragma once
#include "ashura/std/types.h"

namespace ash
{
constexpr u32 DEFAULT_PCG32_MULTIPLIER = 747796405U;
constexpr u32 DEFAULT_PCG32_INCREMENT  = 2891336453U;
constexpr u32 DEFAULT_PCG32_SEED       = 0x46b56677U;
constexpr u64 DEFAULT_PCG64_MULTIPLIER = 6364136223846793005ULL;
constexpr u64 DEFAULT_PCG64_INCREMENT  = 1442695040888963407ULL;
constexpr u64 DEFAULT_PCG64_SEED       = 0x4d595df4d0f33173ULL;

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

constexpr u64 pcg64_rxs_m_xs(u64 state)
{
  u64 word =
      ((state >> ((state >> 59U) + 5U)) ^ state) * 12605985483714917081ULL;
  return (word >> 43U) ^ word;
}

/// linearly step/change the state of the machine
constexpr u32 pcg32_step(u32 state)
{
  u32 new_state = state * DEFAULT_PCG32_MULTIPLIER + DEFAULT_PCG32_INCREMENT;
  return new_state;
}

constexpr u64 pcg64_step(u64 state)
{
  u64 new_state = state * DEFAULT_PCG64_MULTIPLIER + DEFAULT_PCG64_INCREMENT;
  return new_state;
}

/// given a linear u32 input, map it randomly to the u32 value range
constexpr u32 pcg32(u32 input)
{
  u32 state = pcg32_step(input);
  return pcg32_rxs_m_xs(state);
}

constexpr u64 pcg64(u64 input)
{
  u64 state = pcg64_step(input);
  return pcg64_rxs_m_xs(state);
}

constexpr u32 pcg32_generate(u32 &state)
{
  u32 output = pcg32_rxs_m_xs(state);
  state      = pcg32_step(state);
  return output;
}

constexpr u64 pcg64_generate(u64 &state)
{
  u64 output = pcg64_rxs_m_xs(state);
  state      = pcg64_step(state);
  return output;
}

/// use a PCG hash as the seed/state for generating the next hash value
constexpr u32 pcg32_combine(u32 pcg0, u32 input)
{
  u32 state = pcg32_step(pcg0 + input);
  return pcg32_rxs_m_xs(state);
}

constexpr u64 pcg64_combine(u64 pcg0, u64 input)
{
  u64 state = pcg64_step(pcg0 + input);
  return pcg64_rxs_m_xs(state);
}

inline u32 pcg32_hash_bytes(void const *ptr, usize size)
{
  // get bytes
  u8 const *bytes = (u8 const *) ptr;
  u32       pcg   = DEFAULT_PCG32_SEED;
  // TODO(lamarrr): implement packed loads
  // for all u32 packs
  // pcg= pcg32_combine(pcg, pack);
  return pcg;
}

inline u64 pcg64_hash_bytes(void const *ptr, usize size);

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

struct Pcg64Rng
{
  // RNG state/seed. can be set to any value
  u64 state = DEFAULT_PCG64_SEED;

  constexpr u64 generate()
  {
    return pcg64_generate(state);
  }
};

}        // namespace ash
