#include "ashura/types.h"
#include <cstring>

namespace ash
{
namespace mem
{

constexpr usize align_offset(usize alignment, usize offset)
{
  return (offset + (alignment - 1)) / alignment;
}

template <typename T>
void copy(T *src, T *dst, usize count)
{
  std::memcpy(dst, src, sizeof(T) * count);
}

template <typename T>
void copy(Span<T const> src, Span<T> dst)
{
  std::memcpy(dst.data, src.data, src.size_bytes());
}

template <typename T>
void copy(Span<T const> src, T *dst)
{
  std::memcpy(dst, src.data, src.size_bytes());
}

template <typename T>
void zero(T *dst, usize count)
{
  std::memset(dst, 0, sizeof(T) * count);
}

void zero(void *dst, usize size)
{
  std::memset(dst, 0, size);
}

template <typename T>
void zero(Span<T> dst)
{
  std::memset(dst.data, 0, dst.size_bytes());
}

template <typename T>
void fill(T *dst, usize count, u8 byte)
{
  std::memset(dst, byte, sizeof(T) * count);
}

template <typename T>
void fill(Span<T> dst, u8 byte)
{
  std::memset(dst.data, byte, dst.size_bytes());
}

}        // namespace mem
}        // namespace ash
