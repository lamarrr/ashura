#pragma once
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
struct RemoveRef
{
  using Type = T;
};

template <typename T>
struct RemoveRef<T &> : RemoveRef<T>
{
};

template <typename T>
struct RemoveRef<T &&> : RemoveRef<T>
{
};

template <typename T>
using remove_ref = typename RemoveRef<T>::Type;

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

template <typename T>
concept NonConst = !IsConstImpl<T>::value;

/// can be overloaded for custom types
template <typename T>
struct IsTriviallyRelocatable
{
  static constexpr bool value = std::is_trivially_destructible_v<T> &&
                                std::is_trivially_move_constructible_v<T>;
};

// trivially move constructible to uninitialized memory, and have old object
// trivially destroyed
template <typename T>
concept TriviallyRelocatable = IsTriviallyRelocatable<T>::value;

template <typename T>
concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

template <typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

template <typename T>
concept TriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;

template <typename T>
concept TriviallyMoveConstructible = std::is_trivially_move_constructible_v<T>;

template <typename T, typename Base>
concept Derives = std::is_base_of_v<Base, T>;

}        // namespace ash
