/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/cfg.h"
#include <type_traits>

namespace ash
{

template <typename T>
struct RemoveConst
{
  using Type = T;
};

template <typename T>
struct RemoveConst<T const> : RemoveConst<T>
{
};

template <typename T>
using remove_const = typename RemoveConst<T>::Type;

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
struct RemoveConstVolatile<T volatile> : RemoveConstVolatile<T>
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
using decay = std::decay_t<T>;

template <typename T, typename U>
struct is_same_impl
{
  static constexpr bool value = false;
};

template <typename T>
struct is_same_impl<T, T>
{
  static constexpr bool value = true;
};

template <typename T, typename U>
concept Same =
#if ASH_CFG(COMPILER, CLANG)
    __is_same(T, U);
#else
    is_same_impl<T, U>::value;
#endif

template <typename From, typename To>
concept Convertible = requires (From && from) {
  { static_cast<To>(static_cast<From &&>(from)) };
};

template <typename T>
struct is_const_impl
{
  static constexpr bool value = false;
};

template <typename T>
struct is_const_impl<T const>
{
  static constexpr bool value = true;
};

template <typename T>
concept Const =
#if ASH_CFG(COMPILER, CLANG)
    __is_const(T);
#else
    is_const_impl<T>::value;
#endif

template <typename T>
concept NonConst =
#if ASH_CFG(COMPILER, CLANG)
    !__is_const(T);
#else
    !is_const_impl<T>::value;
#endif

/// can be overloaded for custom types
template <typename T>
struct IsTriviallyRelocatable
{
  static constexpr bool value =
#if ASH_CFG(COMPILER, CLANG)
      __is_trivially_constructible(T, T &&) && __is_trivially_destructible(T);
#else
      std::is_trivially_move_constructible_v<T> &&
      std::is_trivially_destructible_v<T>;
#endif
};

template <typename T>
struct IsTriviallyRelocatable<T const> : IsTriviallyRelocatable<T>
{
};

template <typename T>
struct IsTriviallyRelocatable<T volatile> : IsTriviallyRelocatable<T>
{
};

// trivially move constructible to uninitialized memory, and have old object
// trivially destroyed
template <typename T>
concept TriviallyRelocatable = IsTriviallyRelocatable<T>::value;

template <typename T>
concept TriviallyDestructible =
#if ASH_CFG(COMPILER, CLANG)
    __is_trivially_destructible(T);
#else
    std::is_trivially_destructible_v<T>;
#endif

template <typename T>
concept TriviallyCopyable =
#if ASH_CFG(COMPILER, CLANG)
    __is_trivially_copyable(T);
#else
    std::is_trivially_copyable_v<T>;
#endif

template <typename T>
concept TriviallyCopyConstructible =
#if ASH_CFG(COMPILER, CLANG)
    __is_trivially_constructible(T, T const &);
#else
    std::is_trivially_copy_constructible_v<T>;
#endif

template <typename T>
concept TriviallyMoveConstructible =
#if ASH_CFG(COMPILER, CLANG)
    __is_trivially_constructible(T, T &&);
#else
    std::is_trivially_copy_constructible_v<T>;
#endif

template <typename T, typename Base>
concept Derives =
#if ASH_CFG(COMPILER, CLANG)
    __is_base_of(Base, T);
#else
    std::is_base_of_v<Base, T>;
#endif

template <typename T>
concept Unsigned =
#if ASH_CFG(COMPILER, CLANG)
    __is_unsigned(T);
#else
    std::is_unsigned_v<T>;
#endif

template <typename T>
concept Signed =
#if ASH_CFG(COMPILER, CLANG)
    __is_signed(T);
#else
    std::is_signed_v<T>;
#endif

template <typename T>
concept FloatingPoint =
#if ASH_CFG(COMPILER, CLANG)
    __is_floating_point(T);
#else
    std::is_floating_point_v<T>;
#endif

template <typename F, typename... Args>
concept Callable = requires (F && f, Args &&... args) {
  { static_cast<F &&>(f)(static_cast<Args &&>(args)...) };
};

template <typename F, typename... Args>
using CallResult = decltype(std::declval<F>()((std::declval<Args>())...));

template <typename Fn, typename... Args>
concept Predicate = requires (Fn fn, Args... args) {
  { fn(static_cast<Args>(args)...) && true };
};

}        // namespace ash
