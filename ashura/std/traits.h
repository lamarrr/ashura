#pragma once
#include <cinttypes>
#include <type_traits>

namespace ash
{
template <typename T>
struct RemoveConstVolatile
{
  using Type = T;
};

template <typename T>
struct RemoveConstVolatile<T const> : RemoveConstVolatile<T>
{
};

template <typename T>
struct RemoveConstVolatile<volatile T> : RemoveConstVolatile<T>
{
};

template <typename T>
using remove_const_volatile = typename RemoveConstVolatile<T>::Type;

template <typename T>
struct IsConstImpl
{
  static constexpr bool value = false;
};

template <typename T>
struct IsConstImpl<T const>
{
  static constexpr bool value = true;
};

template <typename T>
concept Const = IsConstImpl<T>::value;

/// can be overloaded for custom types
template <typename T>
struct IsTriviallyRelocatable
{
  static constexpr bool value = std::is_trivially_destructible_v<T> &&
                                std::is_trivially_move_constructible_v<T>;
};

template <typename T>
concept TriviallyRelocatable = IsTriviallyRelocatable<T>::value;

template <typename T>
concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

}        // namespace ash
