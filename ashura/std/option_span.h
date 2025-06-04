/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/option.h"
#include "ashura/std/types.h"

namespace ash
{

template <typename T, typename MaskRepr>
struct OptionSpanIter
{
  using SpanType = SpanIter<T>;
  using Mask     = BitSpanIter<MaskRepr>;
  using Type     = Option<T>;
  using Ref      = Option<T &>;

  SpanType span_    = {};
  Mask     is_some_ = {};

  constexpr OptionSpanIter & operator++()
  {
    span_++;
    is_some_++;
    return *this;
  }

  constexpr Ref operator*() const
  {
    if (!(*is_some_))
    {
      return none;
    }

    return *span_;
  }

  constexpr bool operator!=(IterEnd) const
  {
    return span_ != iter_end;
  }
};

template <typename T, typename MaskRepr>
requires ((Const<T> && Const<MaskRepr>) || (NonConst<T> && NonConst<MaskRepr>) )
struct OptionSpan
{
  using SpanType = Span<T>;
  using Mask     = BitSpan<MaskRepr>;
  using Iter     = OptionSpanIter<T, MaskRepr>;
  using Type     = Option<T>;
  using Ref      = Option<T &>;

  SpanType span_;
  Mask     is_some_;

  constexpr OptionSpan(SpanType span, Mask is_some) :
    span_{span},
    is_some_{is_some}
  {
  }

  constexpr OptionSpan()                               = default;
  constexpr OptionSpan(OptionSpan const &)             = default;
  constexpr OptionSpan(OptionSpan &&)                  = default;
  constexpr OptionSpan & operator=(OptionSpan const &) = default;
  constexpr OptionSpan & operator=(OptionSpan &&)      = default;
  constexpr ~OptionSpan()                              = default;

  constexpr auto span() const
  {
    return span_;
  }

  constexpr auto mask() const
  {
    return is_some_;
  }

  constexpr auto begin() const
  {
    return Iter{.span_ = span_.begin(), .is_some_ = is_some_.begin()};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr bool is_some(usize i) const
  {
    return is_some_[i];
  }

  constexpr usize size() const
  {
    return span_.size();
  }

  constexpr Ref operator[](usize i) const
  {
    return get(i);
  }

  constexpr Ref get(usize i) const
  {
    if (!is_some(i))
    {
      return none;
    }

    return span_[i];
  }

  constexpr Ref first() const
  {
    return get(0);
  }

  constexpr Ref last() const
  {
    return get(size() - 1);
  }

  constexpr void set(usize i, Option<T> value) const
    requires (NonConst<T> && NonConst<MaskRepr>)
  {
    is_some_.set(i, value.is_some());

    if constexpr (TriviallyMoveConstructible<T> && TriviallyDestructible<T>)
    {
      span_[i] = static_cast<T &&>(value.v0_);
    }
    else
    {
      if (!is_some(i))
      {
        if (value.is_some())
        {
          new (span_.data() + i) T{static_cast<T &&>(value.v0_)};
        }
      }
      else
      {
        if (value.is_some())
        {
          span_[i] = static_cast<T &&>(value.v0_);
        }
        else
        {
          (span_.data() + i)->~T();
        }
      }
    }
  }

  constexpr auto as_const() const
  {
    return OptionSpan<T const, MaskRepr const>{span_.as_const(),
                                               is_some_.as_const()};
  }
};

template <typename Repr, typename MaskRepr>
struct OptionBitIter
{
  using SpanType = BitSpanIter<Repr>;
  using Mask     = BitSpanIter<MaskRepr>;
  using Type     = Option<bool>;

  SpanType span_    = {};
  Mask     is_some_ = {};

  constexpr OptionBitIter & operator++()
  {
    span_++;
    is_some_++;
    return *this;
  }

  constexpr Type operator*() const
  {
    if (!(*is_some_))
    {
      return none;
    }

    return *span_;
  }

  constexpr bool operator!=(IterEnd) const
  {
    return span_ != iter_end;
  }
};

template <typename Repr, typename MaskRepr>
requires ((Const<Repr> && Const<MaskRepr>) ||
          (NonConst<Repr> && NonConst<MaskRepr>) )
struct OptionBitSpan
{
  using SpanType = BitSpan<Repr>;
  using Mask     = BitSpan<MaskRepr>;
  using Iter     = OptionSpanIter<Repr, MaskRepr>;
  using Type     = Option<bool>;

  SpanType span_;
  Mask     is_some_;

  constexpr OptionBitSpan(SpanType span, Mask is_some) :
    span_{span},
    is_some_{is_some}
  {
  }

  constexpr OptionBitSpan()                                  = default;
  constexpr OptionBitSpan(OptionBitSpan const &)             = default;
  constexpr OptionBitSpan(OptionBitSpan &&)                  = default;
  constexpr OptionBitSpan & operator=(OptionBitSpan const &) = default;
  constexpr OptionBitSpan & operator=(OptionBitSpan &&)      = default;
  constexpr ~OptionBitSpan()                                 = default;

  constexpr auto span() const
  {
    return span_;
  }

  constexpr auto mask() const
  {
    return is_some_;
  }

  constexpr auto begin() const
  {
    return Iter{.span_ = span_.begin(), .is_some_ = is_some_.begin()};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr bool is_some(usize i) const
  {
    return is_some_[i];
  }

  constexpr usize size() const
  {
    return span_.size();
  }

  constexpr Type operator[](usize i) const
  {
    return get(i);
  }

  constexpr Type get(usize i) const
  {
    if (!is_some(i))
    {
      return none;
    }

    return Type{span_[i]};
  }

  constexpr auto first() const
  {
    return get(0);
  }

  constexpr auto last() const
  {
    return get(size() - 1);
  }

  constexpr void set(usize i, Type value) const requires (NonConst<Repr>)
  {
    is_some_.set(i, value);
    span_.set(i, value.v0_);
  }

  constexpr auto as_const() const
  {
    return OptionBitSpan<Repr const, MaskRepr const>{span_.as_const(),
                                                     is_some_.as_const()};
  }
};

}    // namespace ash
