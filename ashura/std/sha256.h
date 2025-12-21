#pragma once
#include "ashura/std/types.h"

namespace ash
{

using Sha256 = Array<u8, 32>;

typedef struct ISha256State * Sha256State;

struct ISha256State
{
  static constexpr usize BLOCK_SIZE = 64;

  u32 state_[8];
  u64 size_;
  u32 offset_;
  u8  buf_[BLOCK_SIZE];

  ISha256State();
  ISha256State(ISha256State const &)             = delete;
  ISha256State & operator=(ISha256State const &) = delete;
  ISha256State(ISha256State &&)                  = delete;
  ISha256State & operator=(ISha256State &&)      = delete;
  ~ISha256State()                                = default;

  void update(Span<u8 const> data);

  Sha256 finalize();

  void transform_(u8 const * buff);
};

}    // namespace ash
