#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "stx/mem.h"

namespace stx {

template <typename Signature>
struct Fn {};

// NOTE that this is just an handle and doesn't manage lifetime.
// it is essentially a trivial struct. it is just a
// pointer (resource handle).
// this should never be initialized with a nullptr or invalid handle.
//
// copying and invoking copies of this function handle type could be dangerous
// if done across threads if the handle type mutates its' state upon call.
//
//
//
// this is just like std::span, it becomes invalid if and only if the handle
// becomes invalidated.
//
template <typename ReturnType, typename... Args>
struct Fn<ReturnType(Args...)> {
  using raw_func_type = ReturnType (*)(Args...);
  using func_type = ReturnType (*)(void*, raw_func_type, Args...);

  explicit constexpr Fn(func_type ifunc, raw_func_type iraw_func,
                        void* idata_addr)
      : func{ifunc}, raw_func{iraw_func}, data_addr{idata_addr} {}

  ReturnType operator()(Args... args) const {
    func(data_addr, raw_func, std::forward<Args>(args)...);
  }

 private:
  // the backing handle for this function object.
  // either is nullptr, both can not be nullptr
  func_type func = nullptr;
  raw_func_type raw_func = nullptr;
  void* data_addr = nullptr;
};

template <typename Signature>
using RcFn = Rc<Fn<Signature>>;

namespace impl {

template <typename T>
struct fn_decay_impl {
  using type = T;
};

template <typename ReturnType, typename... Args>
struct fn_decay_impl<ReturnType(Args...)> {
  using type = ReturnType (*)(Args...);
};

template <typename T>
using fn_decay = typename fn_decay_impl<T>::type;

template <typename T>
struct is_function_pointer_impl : public std::false_type {};

template <typename ReturnType, typename... Args>
struct is_function_pointer_impl<ReturnType(Args...)> : public std::true_type {};

template <typename ReturnType, typename... Args>
struct is_function_pointer_impl<ReturnType (*)(Args...)>
    : public std::true_type {};
}  // namespace impl

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
struct RawFnTraits<ReturnType(Args...)> {
  using handle = Fn<ReturnType(Args...)>;
  using signature = ReturnType(Args...);
  using ptr = ReturnType (*)(Args...);

  static constexpr auto func = [](typename handle::raw_func_type raw_func,
                                  void* data, Args... args) {
    raw_func(std::forward<Args>(args)...);
  };
};

template <typename ReturnType, typename... Args>
struct RawFnTraits<ReturnType (*)(Args...)>
    : public RawFnTraits<ReturnType(Args...)> {};

// wrapper for function objects.
// NOTE: we take ownership of the function object.
// this implies that the function object's only role is to be a function.
//
// this is typically used for lambda types. NOTE: a lambda's type is annonymous
// and not known at compile time
//
// we'd ideally lock this whilst it is being called but the user could also
// potentially already have safe data structures.
//
//
//
//-------------------

template <typename Type, typename ReturnType, typename... Args>
struct FnDispatcher {
  using handle = Fn<ReturnType(Args...)>;

  static constexpr auto func = [](typename handle::raw_func_type raw_func,
                                  void* data, Args... args) {
    *(reinterpret_cast<Type*>(data))(std::forward<Args>(args)...);
  };
};

template <class MemberFunctionSignature>
struct MemberFnTraits {};

// non-const member functions
template <class Type, typename ReturnType, typename... Args>
struct MemberFnTraits<ReturnType (Type::*)(Args...)> {
  using handle = Fn<ReturnType(Args...)>;
  using signature = ReturnType(Args...);
  using ptr = ReturnType (*)(Args...);
  using dispatcher = FnDispatcher<Type, ReturnType, Args...>;
};

// const member functions
template <class Type, typename ReturnType, typename... Args>
struct MemberFnTraits<ReturnType (Type::*)(Args...) const> {
  using handle = Fn<ReturnType(Args...)>;
  using signature = ReturnType(Args...);
  using ptr = ReturnType (*)(Args...);
  using dispatcher = FnDispatcher<Type, ReturnType, Args...>;
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
auto make_raw_functor_fn(Functor& functor) {
  static_assert(is_functor<Functor>);

  using traits = FunctorFnTraits<Functor>;
  using handle_type = typename traits::handle;
  using dispatcher_type = typename traits::dispatcher;

  return handle_type{dispatcher_type::func, nullptr, &functor};
}

template <typename RawFunctionType,
          std::enable_if_t<is_function_pointer<RawFunctionType>, int> = 0>
auto make_raw_ptr_fn(RawFunctionType function_pointer) {
  using traits = RawFnTraits<RawFunctionType>;
  using handle_type = typename traits::handle;
  using dispatcher_type = typename traits::dispatcher;

  return handle_type{dispatcher_type::func, function_pointer, nullptr};
}

template <typename StaticFunctor,
          std::enable_if_t<is_functor<StaticFunctor>, int> = 0>
auto make_raw_ptr_fn(StaticFunctor functor) {
  using traits = FunctorFnTraits<StaticFunctor>;
  using fn_ptr_type = typename traits::ptr;

  static_assert(std::is_convertible_v<StaticFunctor, fn_ptr_type>);

  auto fn_ptr = static_cast<fn_ptr_type>(functor);

  return make_raw_ptr_fn(fn_ptr);
}

// make_static_fn_handle
// TODO(lamarrr): what will happen when recursed?

template <typename FunctorType>
auto make_functor_fn(FunctorType&& fn) {
  Rc fn_rc = mem::make_rc(std::move(fn));
  auto fn_rc_handle = make_raw_functor_fn(*fn_rc.get());

  return stx::transmute(fn_rc_handle, std::move(fn_rc));
}

template <typename FunctorType>
void make_functor_fn(FunctorType& fn) = delete;

template <typename RawFunctionType,
          std::enable_if_t<is_function_pointer<RawFunctionType>, int> = 0>
auto make_static_fn(RawFunctionType function_pointer) {
  using traits = RawFnTraits<RawFunctionType>;
  using fn_ptr_type = typename traits::ptr;
  using handle_type = typename traits::handle;
  using rc_type = Rc<handle_type>;

  pmr::Manager manager{pmr::static_storage_manager_handle};
  manager.ref();

  return unsafe_make_rc(handle_type{function_pointer}, std::move(manager));
}

template <typename StaticFunctor,
          std::enable_if_t<is_functor<StaticFunctor>, int> = 0>
auto make_static_fn(StaticFunctor functor) {
  using traits = FunctorFnTraits<StaticFunctor>;
  using fn_ptr_type = typename traits::ptr;

  static_assert(std::is_convertible_v<StaticFunctor, fn_ptr_type>,
                "functor is inconvertible to function pointer");

  auto fn_ptr = static_cast<fn_ptr_type>(functor);

  return make_static_fn(fn_ptr);
}

}  // namespace stx
