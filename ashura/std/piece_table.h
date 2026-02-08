/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/rc.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

template <typename ViewType>
struct Piece
{
    using View = ViewType;

    Rc<View> buffer_;

    constexpr Piece(Rc<View> buffer) : buffer_{std::move(buffer)}
    {
    }

    constexpr usize size() const
    {
        return buffer_.get().size();
    }

    constexpr View view() const
    {
        return buffer_.get().view();
    }

    constexpr Piece subslice(Slice s) const
    {
        return Piece{transmute(buffer_.alias(), buffer_.get().view().slice(s))};
    }

    constexpr Piece with_slice(Slice s)
    {
        auto view = buffer_.get().view().slice(s);
        return Piece{transmute(std::move(buffer_), view)};
    }
};

template <typename PieceType>
requires (NonConst<PieceType>)
struct PieceTable
{
    using Piece     = PieceType;
    using PieceView = typename Piece::View;

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
            return piece_->view().slice(iter_ - piece_pos_, 1).data()[0];
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
            return !(*this == iter_end);
        }

        constexpr usize operator-(CharIter const & other) const
        {
            return iter_ - other.iter_;
        }
    };

    struct PieceIter
    {
        using Type = PieceView;

        Piece const * piece_     = nullptr;
        Piece const * piece_end_ = nullptr;
        usize         piece_pos_ = 0;
        usize         iter_      = 0;
        usize         end_       = 0;

        constexpr PieceView view() const
        {
            return piece_->view().slice(iter_ - piece_pos_, end_ - piece_pos_);
        }

        constexpr Piece alias() const
        {
            return piece_->subslice(Slice::slice(iter_ - piece_pos_, end_ - iter_));
        }

        constexpr Tuple<usize, usize> diff(PieceIter const & rhs) const
        {
            return {static_cast<usize>(piece_ - rhs.piece_), iter_ - rhs.iter_};
        }

        constexpr PieceView operator*() const
        {
            return view();
        }

        constexpr PieceIter & operator++()
        {
            piece_pos_ = piece_pos_ + piece_->size();
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
            return !(*this == iter_end);
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
                total_size += piece.size();
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
                auto current = Slice::slice(piece_pos, piece_iter->size());
                if (current.contains(range.begin()))
                {
                    break;
                }

                piece_pos = current.end();
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
            return iter_end;
        }

        constexpr auto piece_begin(Slice range) const
        {
            auto       piece_iter = pieces_.data();
            auto const piece_end  = pieces_.data() + pieces_.size();
            usize      piece_pos  = 0;

            while (piece_iter != piece_end)
            {
                auto current = Slice::slice(piece_pos, piece_iter->size());
                if (current.contains(range.begin()))
                {
                    break;
                }

                piece_pos = current.end();
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
            return iter_end;
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
        constexpr Result<> compact(Slice range, Out & out) const
        {
            for (auto iter = piece_begin(range); iter != piece_end(); iter++)
            {
                if (!out.append(*iter))
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
        auto old = Slice::slice(0, pieces_.size());

        pieces_.reserve_extend(pieces_.size() + 8).unwrap();

        auto offset = 0uz;

        for (auto & piece : pieces_)
        {
            auto range      = Slice::slice(offset, piece.size());
            bool intersects = erase.intersects(range);

            if (!intersects)
            {
                pieces_.push(std::move(piece)).unwrap();
            }
            else if (erase.contains(range))
            {
            }
            else if (range.contains(erase))
            {
                auto left_range  = Slice::offsets(range.begin(), erase.begin());
                auto right_range = Slice::offsets(erase.end(), range.end());

                auto left_slice =
                  Slice::slice(left_range.begin() - range.begin(), left_range.span);
                auto right_slice =
                  Slice::slice(right_range.begin() - range.begin(), right_range.span);

                auto left  = piece.subslice(left_slice);
                auto right = piece.subslice(right_slice);

                if (!left_slice.is_empty())
                {
                    pieces_.push(std::move(left)).unwrap();
                }
                if (!right_slice.is_empty())
                {
                    pieces_.push(std::move(right)).unwrap();
                }
            }
            else if (range.begin() > erase.begin())
            {
                auto result = Slice::offsets(erase.end(), range.end());
                auto slice  = Slice::slice(result.offset - range.begin(), result.span);
                pieces_.push(piece.with_slice(slice)).unwrap();
            }
            else
            {
                auto result = Slice::offsets(range.begin(), erase.begin());
                auto slice  = Slice::slice(result.offset - range.begin(), result.span);
                pieces_.push(piece.with_slice(slice)).unwrap();
            }

            offset = range.end();
        }

        pieces_.erase(old);
    }

    template <typename... Args>
    constexpr Result<> insert(usize pos, Args &&... args)
    {
        usize ipiece    = 0;
        usize piece_pos = 0;

        for (; ipiece < pieces_.size(); ipiece++)
        {
            auto & piece = pieces_[ipiece];
            auto   range = Slice::slice(piece_pos, piece.size());
            if (range.contains(pos))
            {
                break;
            }
            piece_pos = range.end();
        }

        // insert at end of piece
        if (ipiece == pieces_.size())
        {
            return pieces_.push(std::forward<Args>(args)...);
        }

        // insert before piece
        if (piece_pos == pos)
        {
            return pieces_.insert(ipiece, std::forward<Args>(args)...);
        }

        // insert in middle of piece
        auto & left        = pieces_[ipiece];
        auto   offset      = pos - piece_pos;
        auto   left_slice  = Slice::slice(0, offset);
        auto   right_slice = Slice::slice(offset, USIZE_MAX);

        auto right = left.subslice(right_slice);
        left       = left.with_slice(left_slice);

        Piece pieces[] = {Piece{std::forward<Args>(args)...}, std::move(right)};

        return pieces_.insert_span_move(ipiece + 1, span(pieces));
    }

    template <typename... Args>
    constexpr Result<> append(Args &&... args)
    {
        return pieces_.push(std::forward<Args>(args)...);
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
    constexpr Result<> compact(Slice range, Out & out) const
    {
        return view().compact(range, out);
    }
};

using Piece8  = Piece<Str8>;
using Piece16 = Piece<Str16>;
using Piece32 = Piece<Str32>;

using PieceTable8  = PieceTable<Piece8>;
using PieceTable16 = PieceTable<Piece16>;
using PieceTable32 = PieceTable<Piece32>;

using PieceTableView8  = typename PieceTable8 ::View;
using PieceTableView16 = typename PieceTable16::View;
using PieceTableView32 = typename PieceTable32::View;

}    // namespace ash
