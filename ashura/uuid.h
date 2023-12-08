#pragma once
#include "ashura/pcg.h"
#include "ashura/primitives.h"

namespace ash
{

using uuid = u64;

/// A PRNG-Based UUID Generator
/// https://datatracker.ietf.org/doc/html/rfc4122
struct PrngUuidGenerator
{
  constexpr PrngUuidGenerator() = default;

  STX_MAKE_PINNED(PrngUuidGenerator)

  Pcg32Rng rng;

  uuid generate()
  {
    u64 r0 = rng.generate();
    rng.state += 1;
    u64 r1 = rng.generate();
    return (r0 << 32) | r1;
  }
};

}        // namespace ash
