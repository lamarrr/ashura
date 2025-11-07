/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/piece_table.h"
#include "ashura/std/rc.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

// [ ] we need to add metadata to track whether the piece should be preserved and not copied,
// i.e. for memory-mapped files

// [ ] sum the buffer_size of all pieces to get the total duplicated size, not just the size of the pieces themselves
// make the piece constructor always take the orignal buffer and slice into it
// [ ] do not create alias views of slices!!!!!!!
// [ ] we can track the total buffer size of the piece table and compare to the actual size of the (non-paged) pieces to
// determine if to compact, use a ratio of 0.75 expected load factor
// [ ] aliasing of page pieces?
template <typename Char>
requires (NonConst<Char>)
struct EditStr
{
  using Buffer    = Vec<Char>;
  using Str       = Span<Char const>;
  using Piece     = ash::Piece<Str>;
  using Table     = PieceTable<Piece>;
  using TableView = typename Table::View;

  static constexpr usize DEFAULT_PIECE_CAPACITY = 1'024;
  static constexpr usize MAX_PIECES             = 64;

  Allocator entry_allocator_;

  Allocator piece_allocator_;

  Table table_;

  /// @brief capacity of each piece
  usize piece_capacity_;

  /// @brief buffer for holding the current piece data
  Option<Rc<Buffer *>> buffer_;

  /// @brief position of the most recently accessed piece
  Option<Slice> last_buffer_insert_;

  constexpr EditStr(Allocator entry_allocator, Allocator piece_allocator,
                    Allocator table_allocator, usize piece_capacity) :
    entry_allocator_{entry_allocator},
    piece_allocator_{piece_allocator},
    table_{table_allocator},
    piece_capacity_{piece_capacity},
    buffer_{none},
    last_buffer_insert_{none}
  {
  }

  constexpr void clear()
  {
    table_.clear();
    buffer_             = none;
    last_buffer_insert_ = none;
  }

  constexpr void reset()
  {
    table_.reset();
    buffer_             = none;
    last_buffer_insert_ = none;
  }

  constexpr Result<> insert(usize pos, Str str)
  {
    // [ ] error propagation
    // [ ] remove subspan of buffers, and use slice 
    bool is_page = str.size() > piece_capacity_;

    if (is_page)
    {
      auto r0 = Buffer::make(str.size(), piece_allocator_);

      if (!r0)
      {
        return Err{};
      }

      auto r1 = rc<Buffer>(inplace, entry_allocator_, r0.unwrap());

      if (!r1)
      {
        return Err{};
      }

      auto buffer = r1.unwrap();

      buffer->append(within_capacity, str).unwrap();

      auto view   = buffer->view().as_const();
      auto rc_str = transmute(std::move(buffer), view);

      if (!table_.insert(pos, transmute(buffer.alias(), view)))
      {
        return Err{};
      }

      last_buffer_insert_ = none;
      return Ok{};
    }

    if (buffer_.is_none())
    {
      auto r0 = Buffer::make(piece_capacity_, piece_allocator_);

      if (!r0)
      {
        return Err{};
      }

      auto r1 = rc<Buffer>(inplace, entry_allocator_, std::move(r0.unwrap()));

      if (!r1)
      {
        return Err{};
      }

      auto buffer = r1.unwrap();

      buffer->append(within_capacity, str).unwrap();

      auto view = buffer->view().as_const();

      if (!table_.insert(pos, transmute(buffer.alias(), view)))
      {
        return Err{};
      }

      last_buffer_insert_ = Slice::slice(pos, str.size());
      buffer_             = std::move(buffer);

      return Ok{};
    }
    else
    {
      auto & buffer           = *buffer_;
      auto   prev_buffer_size = buffer->size();

      if (buffer->append(within_capacity, str))
      {
        if (last_buffer_insert_.is_some() && last_buffer_insert_->end() == pos)
        {
          auto last_range                = *last_buffer_insert_;
          auto new_range                 = last_range.extend(str.size());
          auto last_insert_buffer_offset = prev_buffer_size - last_range.span;
          auto view =
            buffer->view().as_const().slice(last_insert_buffer_offset);

          table_.erase(last_range);
          if (!table_.insert(new_range.offset, transmute(buffer.alias(), view)))
          {
            return Err{};
          }

          last_buffer_insert_ = new_range;
        }
        else
        {
          auto view = buffer->view().as_const().slice(prev_buffer_size);

          if (!table_.insert(pos, transmute(buffer.alias(), view)))
          {
            return Err{};
          }

          last_buffer_insert_ = Slice::slice(pos, str.size());
        }

        return Ok{};
      }
      else
      {
        auto head_size = buffer->capacity() - buffer->size();

        if (!insert(pos, str.slice(0, head_size)))
        {
          return Err{};
        }

        return insert(pos + head_size, str.slice(head_size));
      }
    }
  }

  constexpr Result<> insert(usize pos, Rc<Str> buffer)
  {
    return table_.insert(pos, std::move(buffer));
  }

  constexpr void erase(Slice slice)
  {
    if (last_buffer_insert_.is_some())
    {
      // optimization for when we are deleting from the last inserted text buffer
      if (slice.begin() >= last_buffer_insert_->begin() &&
          slice.end() == last_buffer_insert_->end())
      {
        last_buffer_insert_ = Slice::slice(
          last_buffer_insert_->begin(), last_buffer_insert_->span - slice.span);
        (*buffer_)->pop(slice.span);
      }
      else
      {
        last_buffer_insert_ = none;
      }
    }

    // [ ] we can also perform dynamic non-explicit compaction here if the resulting buffer is not sized large enough?
    // [ ] we'd need regular compaction calls anyway to avoid unbounded piece table memory growth as the pieces will still be chopped up from larger strings
    // [ ] create metric to determine when to compact based on number of pieces and total size of pieces?

    table_.erase(slice);
  }

  constexpr TableView get_table() const
  {
    return table_.view();
  }

  constexpr Result<> try_compact(){
    // [ ] how many and which should we compact?
  }

  // [ ] fix
  constexpr Result<> clone(Slice range, EditStr & out) const
  {
    // return table_.clone(range, out);
  }
};

using EditStr8  = EditStr<c8>;
using EditStr16 = EditStr<c16>;
using EditStr32 = EditStr<c32>;

}    // namespace ash
