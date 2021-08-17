#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "stx/mem.h"

namespace stx {

template <typename Signature>
struct Fn {};

// NOTE that this is just an handle and doesn't manage any lifetime.
//
//
// it is essentially a trivial struct. it is just contains pointers (resource
// handle).
//
// func must never be initialized with a nullptr or invalid handle.
//
// invoking copies of this function handle across threads could be dangerous
// if done across threads if the function type mutates its' internal state upon
// call as in the case of a mutable lambda with captures and classes/structs
// with members.
//
//
// Fn is to function pointers and functors as std::span is to std::vector and
// std::array (a view).
//
// we'd ideally lock this whilst it is being called but the user could
// potentially already have internally thread-safe data structures.
//
//
template <typename ReturnType, typename... Args>
struct Fn<ReturnType(Args...)> {
  using func_type = ReturnType (*)(void*, Args...);

  explicit constexpr Fn(func_type idispatcher, void* idata_addr)
      : dispatcher{idispatcher}, data_addr{idata_addr} {}

  constexpr ReturnType operator()(Args... args) const {
    return dispatcher(data_addr, std::forward<Args>(args)...);
  }

  func_type dispatcher = nullptr;
  void* data_addr = nullptr;
};

template <typename Signature>
using RcFn = Rc<Fn<Signature>>;

namespace impl {

template <typename T>
struct raw_function_decay_impl {
  using type = T;
};

template <typename ReturnType, typename... Args>
struct raw_function_decay_impl<ReturnType(Args...)> {
  using type = ReturnType (*)(Args...);
};

template <typename T>
struct is_function_pointer_impl : public std::false_type {};

template <typename ReturnType, typename... Args>
struct is_function_pointer_impl<ReturnType(Args...)> : public std::true_type {};

template <typename ReturnType, typename... Args>
struct is_function_pointer_impl<ReturnType (*)(Args...)>
    : public std::true_type {};

}  // namespace impl

template <typename T>
using raw_function_decay = typename impl::raw_function_decay_impl<T>::type;

template <typename T>
constexpr bool is_function_pointer = impl::is_function_pointer_impl<T>::value;

namespace impl {

template <typename Signature>
struct raw_fn_impl {};

template <typename ReturnType, typename... Args>
struct raw_fn_impl<ReturnType(Args...)> {
  using type = ReturnType (*)(Args...);
};

template <typename ReturnType, typename... Args>
struct raw_fn_impl<ReturnType (*)(Args...)> {
  using type = ReturnType (*)(Args...);
};

}  // namespace impl

template <typename Type>
using RawFn = typename impl::raw_fn_impl<Type>::type;

template <typename RawFunctionType>
struct RawFnTraits {};

template <typename ReturnType, typename... Args>
struct RawFunctionDispatcher {
  static constexpr ReturnType dispatch(void* data_addr, Args... args) {
    using ptr = ReturnType (*)(Args...);

    return reinterpret_cast<ptr>(data_addr)(std::forward<Args>(args)...);
  }
};

template <typename ReturnType, typename... Args>
struct RawFnTraits<ReturnType(Args...)> {
  using ptr = ReturnType (*)(Args...);
  using signature = ReturnType(Args...);
  using fn = Fn<signature>;
  using dispatcher = RawFunctionDispatcher<ReturnType, Args...>;
};

template <typename ReturnType, typename... Args>
struct RawFnTraits<ReturnType (*)(Args...)>
    : public RawFnTraits<ReturnType(Args...)> {};

template <typename Type, typename ReturnType, typename... Args>
struct FunctorDispatcher {
  static constexpr ReturnType dispatch(void* data_addr, Args... args) {
    return (*(reinterpret_cast<Type*>(data_addr)))(std::forward<Args>(args)...);
  }
};

template <class MemberFunctionSignature>
struct MemberFnTraits {};

// non-const member functions
template <class Type, typename ReturnType, typename... Args>
struct MemberFnTraits<ReturnType (Type::*)(Args...)> {
  using ptr = ReturnType (*)(Args...);
  using signature = ReturnType(Args...);
  using fn = Fn<signature>;
  using type = Type;
  using dispatcher = FunctorDispatcher<type, ReturnType, Args...>;
};

// const member functions
template <class Type, typename ReturnType, typename... Args>
struct MemberFnTraits<ReturnType (Type::*)(Args...) const> {
  using ptr = ReturnType (*)(Args...);
  using signature = ReturnType(Args...);
  using fn = Fn<signature>;
  using type = Type const;
  using dispatcher = FunctorDispatcher<type, ReturnType, Args...>;
};

template <class Type>
struct FunctorFnTraits : public MemberFnTraits<decltype(&Type::operator())> {};

namespace impl {

template <typename T, typename Stub = void>
struct is_functor_impl : public std::false_type {};

template <typename T>
struct is_functor_impl<T, decltype(&T::operator(), (void)0)>
    : public std::true_type {};

}  // namespace impl

template <typename T>
constexpr bool is_functor = impl::is_functor_impl<T>::value;

template <typename Functor>
auto make_functor_fn_raw(Functor& functor) {
  static_assert(is_functor<Functor>);

  using traits = FunctorFnTraits<Functor>;
  using fn = typename traits::fn;
  using dispatcher = typename traits::dispatcher;

  return fn{&dispatcher::dispatch, &functor};
}

template <typename RawFunctionType,
          std::enable_if_t<is_function_pointer<RawFunctionType>, int> = 0>
auto make_ptr_fn_raw(RawFunctionType* function_pointer) {
  using traits = RawFnTraits<RawFunctionType>;
  using fn = typename traits::fn;
  using dispatcher = typename traits::dispatcher;

  return fn{&dispatcher::dispatch, function_pointer};
}

template <typename StaticFunctor,
          std::enable_if_t<is_functor<StaticFunctor>, int> = 0>
auto make_ptr_fn_raw(StaticFunctor functor) {
  using traits = FunctorFnTraits<StaticFunctor>;
  using ptr = typename traits::ptr;

  static_assert(std::is_convertible_v<StaticFunctor, ptr>);

  ptr function_pointer = static_cast<ptr>(functor);

  return make_ptr_fn_raw(function_pointer);
}

template <typename FunctorType>
auto make_functor_fn(FunctorType&& functor) {
  Rc fn_rc = mem::make_rc(std::move(functor));

  Fn fn = make_functor_fn_raw(*fn_rc.get());

  return stx::transmute(fn, std::move(fn_rc));
}

template <typename FunctorType>
void make_functor_fn(FunctorType& fn) = delete;

template <typename RawFunctionType,
          std::enable_if_t<is_function_pointer<RawFunctionType>, int> = 0>
auto make_static_fn(RawFunctionType* function_pointer) {
  using traits = RawFnTraits<RawFunctionType>;
  using fn = typename traits::fn;
  using dispatcher = typename traits::dispatcher;

  Manager manager{static_storage_manager_handle};
  manager.ref();

  return unsafe_make_rc(
      fn{&dispatcher::dispatch, reinterpret_cast<void*>(function_pointer)},
      std::move(manager));
}

template <typename StaticFunctor,
          std::enable_if_t<is_functor<StaticFunctor>, int> = 0>
auto make_static_fn(StaticFunctor functor) {
  using traits = FunctorFnTraits<StaticFunctor>;
  using ptr = typename traits::ptr;

  static_assert(std::is_convertible_v<StaticFunctor, ptr>,
                "functor is not convertible to function pointer");

  ptr function_pointer = static_cast<ptr>(functor);

  return make_static_fn(function_pointer);
}

}  // namespace stx
