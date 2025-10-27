/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/range.h"
#include "ashura/std/rc.h"
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
  struct Iter
  {
  };

  Vec<Rc<Char const *>> segments_;
  Vec<usize>            run_offsets_;

  constexpr Pieced(Vec<Rc<Char const *>> segments, Vec<usize> run_offsets) :
    segments_{std::move(segments)},
    run_offsets_{std::move(run_offsets)}
  {
  }

  constexpr Pieced(Allocator segments_allocator, Allocator run_allocator) :
    segments_{segments_allocator},
    run_offsets_{run_allocator}
  {
  }

  void erase(Slice range)
  {
    if (run_offsets_.is_empty() || range.is_empty()) [[unlikely]]
    {
      return;
    }

    auto first = binary_find(run_offsets_.view(), gt, range.begin());

    if (first.is_empty())
    {
      return;
    }

    auto last = binary_find(first, geq, range.end());

    auto first_run = (usize) (first.pbegin() - run_offsets_.view().pbegin()) - 1;
    auto last_run  = (usize) (last.pbegin() - run_offsets_.view().pbegin()) - 1;

    if(run_offsets_[first_run] == range.begin() && run_offsets_[last_run + 1] == range.end())
    {
    }
    else if()
    {
    }
    else
    {
    }
    
  }

  void insert(usize pos, Span<Char const> text)
  {
  }

  void extend(Span<Char const> text)
  {
  }

  void clear()
  {
  }

  void reset()
  {
  }

  void replace(Slice range, Span<Char const> text)
  {
  }

  void get(Slice range, Vec<Char> & out)
  {
  }

  template <typename F>
  void iter(Slice range, F && func)
  {
  }

  Iter begin()
  {
  }

  IterEnd end()
  {
  }
};

}    // namespace ash
