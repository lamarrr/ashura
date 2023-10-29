#pragma once
#include "ashura/primitives.h"
#include "stx/vec.h"

namespace ash
{

template <typename T, typename Handle = u64>
struct SparseVec
{
  struct Iterator
  {
    SparseVec *array = nullptr;
    usize      index = 0;

    Iterator &operator++()
    {
      while (index < array->sparse.size())
      {
        if ((array->validity_masks[index >> 6] & (1ULL << (index & 63))) != 0ULL)
        {
          break;
        }
        index++;
      }

      return *this;
    }

    Iterator operator++(int)
    {
      Iterator copy = *this;
      ++(*this);
      return copy;
    }

    bool operator==(Iterator other) const
    {
      return array == other.array && index == other.index;
    }

    bool operator!=(Iterator other) const
    {
      return !(*this == other);
    }

    T &operator*()
    {
      return array->sparse.data()[index];
    }

    T *operator->()
    {
      return array->sparse.data() + index;
    }
  };

  Handle push(T element)
  {
    if (!free_indices.is_empty())
    {
      usize index = free_indices.pop().unwrap();
      T    *mem   = sparse.data() + index;
      new (mem) T{std::move(element)};
      validity_masks[index >> 6] |= (1ULL << (index & 63));
      return (Handle) (index + 1);
    }
    else
    {
      usize index = sparse.size();
      sparse.push_inplace(std::move(element)).unwrap();
      usize pack = index >> 6;
      if (pack >= validity_masks.size())
      {
        validity_masks.resize(pack + 1).unwrap();
      }
      validity_masks[pack] |= (1ULL << (index & 63));
      return (Handle) (index + 1);
    }
  }

  void remove(Handle handle)
  {
    ASH_CHECK(is_valid(handle));
    usize index = ((usize) handle) - 1;
    u64  &mask  = validity_masks[index >> 6];
    ASH_CHECK((mask & (1ULL << (index & 63))) != 0);
    mask &= ~(1ULL << (index & 63));
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
      sparse.data()[index].~T();
    }
    free_indices.push_inplace(index).unwrap();
  }

  T &operator[](Handle handle)
  {
    ASH_CHECK(is_valid(handle));
    return sparse.data()[(usize) handle];
  }

  T const &operator[](Handle handle) const
  {
    ASH_CHECK(is_valid(handle));
    return sparse.data()[(usize) handle];
  }

  bool is_valid(Handle handle) const
  {
    usize index = (usize) handle;
    if (index == 0 || index > sparse.size())
    {
      return false;
    }

    index--;

    return (validity_masks[index >> 6] & (1ULL << (index & 63))) != 0ULL;
  }

  Iterator begin()
  {
    return Iterator{.array = this, .index = 0};
  }

  Iterator end()
  {
    return Iterator{.array = this, .index = sparse.size()};
  }

  stx::Vec<T>     sparse;
  stx::Vec<u64>   validity_masks;
  stx::Vec<usize> free_indices;
};

}        // namespace ash