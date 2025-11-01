/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/rc.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"
#include "ashura/std/range.h"

namespace ash
{

template <typename Str>
struct PieceTable
{
  struct Piece
  {
    Rc<Str> buffer{};
    Slice   slice{};
  };

  struct CharIter
  {
    using Type = typename Str::Type;

    Piece const * piece_     = nullptr;
    Piece const * piece_end_ = nullptr;
    usize         piece_pos_ = 0;
    usize         iter_      = 0;
    usize         end_       = 0;

    constexpr Type operator*() const
    {
      return piece_->buffer.get()
        .slice(piece_->slice)
        .slice(iter_ - piece_pos_, 1)
        .data()[0];
    }

    constexpr CharIter & operator++()
    {
      iter_++;
      if (iter_ == (piece_pos_ + piece_->slice.span))
      {
        piece_pos_ = piece_pos_ + piece_->slice.span;
        piece_++;
      }
      return *this;
    }

    constexpr CharIter operator++(int)
    {
      auto old = *this;
      this->operator++();
      return old;
    }

    constexpr bool operator==(CharIter const & other) const
    {
      return (piece_ == other.piece_) & (piece_pos_ == other.piece_pos_) &
             (iter_ == other.iter_) & (end_ == other.end_);
    }

    constexpr bool operator!=(CharIter const & other) const
    {
      return !(*this == other);
    }

    constexpr bool operator==(IterEnd) const
    {
      return (iter_ >= end_) || (piece_ >= piece_end_);
    }

    constexpr bool operator!=(IterEnd) const
    {
      return !(*this == IterEnd{});
    }

    constexpr usize operator-(CharIter const & other) const
    {
      return iter_ - other.iter_;
    }
  };

  struct PieceIter
  {
    using Type = Str;

    Piece const * piece_     = nullptr;
    Piece const * piece_end_ = nullptr;
    usize         piece_pos_ = 0;
    usize         iter_      = 0;
    usize         end_       = 0;

    constexpr Str str() const
    {
      return piece_->buffer.get()
        .slice(piece_->slice)
        .slice(iter_ - piece_pos_, end_ - piece_pos_);
    }

    constexpr Piece alias() const
    {
      auto slice = Slice{piece_->slice.offset + (iter_ - piece_pos_),
                         end_ - iter_}(piece_->buffer.get().size());

      return Piece{piece_->buffer.alias(), slice};
    }

    constexpr Str operator*() const
    {
      return str();
    }

    constexpr PieceIter & operator++()
    {
      piece_pos_ = piece_pos_ + piece_->slice.span;
      piece_++;
      iter_ = piece_pos_;
      return *this;
    }

    constexpr PieceIter operator++(int)
    {
      auto old = *this;
      this->operator++();
      return old;
    }

    constexpr bool operator==(PieceIter const & other) const
    {
      return (piece_ == other.piece_) & (piece_pos_ == other.piece_pos_) &
             (iter_ == other.iter_) & (end_ == other.end_);
    }

    constexpr bool operator!=(PieceIter const & other) const
    {
      return !(*this == other);
    }

    constexpr bool operator==(IterEnd) const
    {
      return (piece_ >= piece_end_) | (iter_ >= end_);
    }

    constexpr bool operator!=(IterEnd) const
    {
      return !(*this == IterEnd{});
    }
  };

  struct View
  {
    Span<Piece const> pieces_{};

    constexpr auto size() const
    {
      usize total_size = 0;

      for (auto const & piece : pieces_)
      {
        total_size += piece.slice.span;
      }

      return total_size;
    }

    constexpr auto num_pieces() const
    {
      return pieces_.size();
    }

    constexpr auto char_begin(Slice range) const
    {
      auto       piece_iter = pieces_.data();
      auto const piece_end  = pieces_.data() + pieces_.size();
      usize      piece_pos  = 0;

      while (piece_iter != piece_end)
      {
        if (Slice{piece_pos, piece_iter->slice.span}.contains(range.begin()))
        {
          break;
        }

        piece_pos += piece_iter->slice.span;
        piece_iter++;
      }

      return CharIter{.piece_     = piece_iter,
                      .piece_end_ = piece_end,
                      .piece_pos_ = piece_pos,
                      .iter_      = range.begin(),
                      .end_       = range.end()};
    }

    constexpr auto char_end() const
    {
      return IterEnd{};
    }

    constexpr auto piece_begin(Slice range) const
    {
      auto       piece_iter = pieces_.data();
      auto const piece_end  = pieces_.data() + pieces_.size();
      usize      piece_pos  = 0;

      while (piece_iter != piece_end)
      {
        if (Slice{piece_pos, piece_iter->slice.span}.contains(range.begin()))
        {
          break;
        }

        piece_pos += piece_iter->slice.span;
        piece_iter++;
      }

      return PieceIter{.piece_     = piece_iter,
                       .piece_end_ = piece_end,
                       .piece_pos_ = piece_pos,
                       .iter_      = range.begin(),
                       .end_       = range.end()};
    }

    constexpr auto piece_end() const
    {
      return IterEnd{};
    }

    constexpr Result<> clone(Slice range, PieceTable & out) const
    {
      for (auto iter = piece_begin(range); iter != piece_end(); iter++)
      {
        if (!out.pieces_.push(iter.alias()))
        {
          return Err{};
        }
      }

      return Ok{};
    }

    template <typename Out>
    constexpr Result<> copy(Slice range, Out & out) const
    {
      for (auto iter = piece_begin(range); iter != piece_end(); iter++)
      {
        if (!out.extend(*iter))
        {
          return Err{};
        }
      }

      return Ok{};
    }
  };

  Vec<Piece> pieces_;

  constexpr PieceTable(Vec<Piece> pieces) : pieces_{std::move(pieces)}
  {
  }

  constexpr PieceTable(Allocator pieces_allocator) : pieces_{pieces_allocator}
  {
  }

  constexpr PieceTable(PieceTable && other)             = default;
  constexpr PieceTable & operator=(PieceTable && other) = default;

  constexpr PieceTable(PieceTable const & other)             = delete;
  constexpr PieceTable & operator=(PieceTable const & other) = delete;

  constexpr ~PieceTable() = default;

  constexpr usize pieces_capacity() const
  {
    return pieces_.capacity();
  }

  constexpr void clear()
  {
    pieces_.clear();
  }

  constexpr void reset()
  {
    pieces_.reset();
  }

  constexpr Result<> reserve(usize capacity)
  {
    return pieces_.reserve(capacity);
  }

  constexpr void erase(Slice erase)
  {
    if (erase.span == 0) [[unlikely]]
    {
      return;
    }

    Option<Tuple<usize, Slice>> first_piece = none;
    Option<Tuple<usize, Slice>> last_piece  = none;
    usize                       offset      = 0;

    for (auto [i, piece] : enumerate(pieces_))
    {
      auto current    = Slice{offset, piece.slice.span};
      bool intersects = current.intersects(erase);

      if (first_piece.is_none() && intersects)
      {
        first_piece = Tuple{i, current};
      }

      if (intersects)
      {
        last_piece = Tuple{i, current};
      }
      else if (last_piece.is_some())
      {
        break;
      }

      offset = current.end();
    }

    if (first_piece.is_none())
    {
      return;
    }

    auto [first_idx, first_range] = first_piece.unwrap();
    auto [last_idx, last_range]   = last_piece.unwrap();

    if (erase.contains(first_range) && erase.contains(last_range))
    {
      pieces_.erase(Slice::elements(first_idx, last_idx));
    }
    else if (erase.contains(last_range))
    {
      pieces_.erase(Slice::elements(first_idx + 1, last_idx));
      auto & piece     = pieces_[first_idx];
      auto   new_range = Slice::range(first_range.begin(), erase.begin());
      piece.slice.span = new_range.span;
    }
    else if (erase.contains(first_range))
    {
      pieces_.erase(Slice::elements(first_idx, last_idx - 1));
      auto & piece     = pieces_[first_idx];
      auto   new_range = Slice::range(erase.end(), last_range.end());
      piece.slice.offset += erase.end() - last_range.begin();
      piece.slice.span = new_range.span;
    }
    else
    {
      pieces_.erase(Slice::elements(first_idx + 1, last_idx - 1));
      auto & first = pieces_[first_idx];
      {
        auto new_range   = Slice::range(first_range.begin(), erase.begin());
        first.slice.span = new_range.span;
      }
      auto & last = pieces_[first_idx + 1];
      {
        auto new_range = Slice::range(erase.end(), last_range.end());
        last.slice.offset += erase.end() - last_range.begin();
        last.slice.span = new_range.span;
      }
    }
  }

  constexpr Result<> insert(usize pos, Piece text)
  {
    usize ipiece    = 0;
    usize piece_pos = 0;

    for (; ipiece < pieces_.size(); ipiece++)
    {
      auto & piece = pieces_[ipiece];
      if (Slice{piece_pos, piece.slice.span}.contains(pos))
      {
        break;
      }
      piece_pos += piece.slice.span;
    }

    // insert at end of piece
    if (ipiece == pieces_.size())
    {
      return pieces_.push(std::move(text));
    }

    // insert before piece
    if (piece_pos == pos)
    {
      return pieces_.insert(ipiece, std::move(text));
    }

    // insert in middle of piece
    auto & piece      = pieces_[ipiece];
    auto   offset     = pos - piece_pos;
    auto   left_range = Slice{0, offset};
    left_range.offset += piece.slice.offset;
    auto right_range = Slice::range(offset, piece.slice.span);
    right_range.offset += piece.slice.offset;

    auto & left    = piece;
    left.slice     = left_range;
    auto  right    = Piece{piece.buffer.alias(), right_range};
    Piece pieces[] = {std::move(text), std::move(right)};

    return pieces_.insert_span_move(ipiece + 1, span(pieces));
  }

  constexpr Result<> insert(usize pos, Rc<Str> text)
  {
    auto size = text.get().size();
    return insert(pos, Piece{
                         std::move(text), Slice{0, size}
    });
  }

  constexpr Result<> extend(Piece text)
  {
    return pieces_.push(std::move(text));
  }

  constexpr Result<> extend(Rc<Str> text)
  {
    auto size = text.get().size();
    return pieces_.push(Piece{
      std::move(text), Slice{0, size}
    });
  }

  constexpr View view() const
  {
    return View{.pieces_ = pieces_.view()};
  }

  constexpr usize size() const
  {
    return view().size();
  }

  constexpr usize num_pieces() const
  {
    return view().num_pieces();
  }

  constexpr auto char_begin(Slice range) const
  {
    return view().char_begin(range);
  }

  constexpr auto char_end() const
  {
    return view().char_end();
  }

  constexpr auto piece_begin(Slice range) const
  {
    return view().piece_begin(range);
  }

  constexpr auto piece_end() const
  {
    return view().piece_end();
  }

  constexpr Result<> clone(Slice range, PieceTable & out) const
  {
    return view().clone(range, out);
  }

  template <typename Out>
  constexpr Result<> copy(Slice range, Out & out) const
  {
    return view().copy(range, out);
  }
};

template <typename Str>
struct IsTriviallyRelocatable<PieceTable<Str>>
{
  static constexpr bool value = TriviallyRelocatable<Str>;
};

using Piece8  = PieceTable<Str8>;
using Piece16 = PieceTable<Str16>;
using Piece32 = PieceTable<Str32>;

}    // namespace ash
