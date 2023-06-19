#pragma once
#include "ashura/primitives.h"
#include <random>

namespace ash
{

using uuid = u64;

struct UuidGenerator
{
  explicit UuidGenerator(timepoint init_timepoint) :
      // SEE: https://datatracker.ietf.org/doc/html/rfc4122
      mersenne_twister{init_timepoint.time_since_epoch().count() / 10}
  {
  }
  STX_MAKE_PINNED(UuidGenerator)

  std::mt19937                       mersenne_twister;
  std::uniform_int_distribution<u64> distribution{stx::U64_MIN, stx::U64_MAX};

  uuid generate()
  {
    return distribution(mersenne_twister);
  }
};

}        // namespace ash
