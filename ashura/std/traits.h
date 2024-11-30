/// SPDX-License-Identifier: MIT
#pragma once
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
concept Same = is_same_impl<T, U>::value;

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
concept Const = is_const_impl<T>::value;

template <typename T>
concept NonConst = !is_const_impl<T>::value;

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

template <typename T>
concept Unsigned = std::is_unsigned_v<T>;

template <typename T>
concept Signed = std::is_signed_v<T>;

template <typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

template <typename F, typename... Args>
concept Callable = requires (F && f, Args &&... args) {
  { static_cast<F &&>(f)(static_cast<Args &&>(args)...) };
};

template <typename F, typename... Args>
using CallResult = decltype(std::declval<F>()((std::declval<Args>())...));

template <typename T>
struct is_pfn_impl
{
  static constexpr bool value = false;
};

template <typename R, typename... Args>
struct is_pfn_impl<R (*)(Args...)>
{
  static constexpr bool value = true;
};

template <typename F>
concept AnyPFn = is_pfn_impl<F>::value;

template <typename F>
concept AnyFunctor = requires () {
  { &F::operator() };
};

}        // namespace ash
