#pragma once
#include "stx/panic.h"
#include "stx/vec.h"
#include <cinttypes>
#include <cstddef>
#include <type_traits>

#define STX_SPARSE_VEC_ENSURE(condition, message)       \
  do                                                    \
  {                                                     \
    if (!(condition))                                   \
    {                                                   \
      ::stx::panic("condition: '" #condition            \
                   "' failed. explanation: " #message); \
    }                                                   \
  } while (0)

STX_BEGIN_NAMESPACE

template <typename T, typename Handle = uint64_t>
struct SparseVec
{
  struct Iterator
  {
    SparseVec *array = nullptr;
    size_t     index = 0;

    Iterator &operator++()
    {
      while (index < array->sparse.size())
      {
        if ((array->validity_masks[index >> 6] & (1ULL << (index & 63))) !=
            0ULL)
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
      size_t index = free_indices.pop().unwrap();
      T     *mem   = sparse.data() + index;
      new (mem) T{(T &&) element};
      validity_masks[index >> 6] |= (1ULL << (index & 63));
      return (Handle) (index + 1);
    }
    else
    {
      size_t index = sparse.size();
      sparse.push_inplace((T &&) element).unwrap();
      size_t pack = index >> 6;
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
    STX_SPARSE_VEC_ENSURE(is_valid(handle), "invalid handle");
    size_t    index = ((size_t) handle) - 1;
    uint64_t &mask  = validity_masks[index >> 6];
    mask &= ~(1ULL << (index & 63));
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
      sparse.data()[index].~T();
    }
    free_indices.push_inplace(index).unwrap();
  }

  T &operator[](Handle handle)
  {
    STX_SPARSE_VEC_ENSURE(is_valid(handle), "invalid handle");
    return sparse.data()[(size_t) handle];
  }

  T const &operator[](Handle handle) const
  {
    STX_SPARSE_VEC_ENSURE(is_valid(handle), "invalid handle");
    return sparse.data()[(size_t) handle];
  }

  bool is_valid(Handle handle) const
  {
    size_t index = (size_t) handle;
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

  Vec<T>        sparse;
  Vec<uint64_t> validity_masks;
  Vec<size_t>   free_indices;
};

STX_END_NAMESPACE
