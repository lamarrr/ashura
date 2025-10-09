/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/dyn.h"
#include "ashura/std/growth.h"
#include "ashura/std/mem.h"
#include "ashura/std/obj.h"
#include "ashura/std/option.h"
#include "ashura/std/rc.h"
#include "ashura/std/result.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

// segmented vector for text editing

// - It should allocate memory in blocks and use them for the partitions; we need to be able to determine the position of a partition
//   from its pointer
//
// [ ] freelist management and memory reclamation

// [ ] if using a contiguous allocator
// [ ] checks for fragmentation in order to compact storage
// [ ] compaction is possible by using memory copies

template <typename Char>
struct Pieced
{
  Vec<Char const *> segments_;
  Vec<usize>        run_offsets_;

  explicit constexpr Pieced(Allocator segments_allocator,
                            Allocator run_allocator) :
    segments_{segments_allocator},
    run_offsets_{run_allocator}
  {
  }

  void erase();
  void insert();
  void extend();
  void clear();
  void replace();
  void get(Slice range, Vec<Char> & out);

  template <typename F>
  void iter(Slice range, F && func);

  void begin();
  void end();
};

}    // namespace ash
