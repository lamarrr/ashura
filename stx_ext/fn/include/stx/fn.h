#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "stx/mem.h"

namespace stx {

template <typename Signature>
struct DynFn {};

// doesn't manage or control lifetime, just a plain handle
template <typename ReturnType, typename... Args>
struct DynFn<ReturnType(Args...)> {
  virtual ReturnType operator()(Args... args) = 0;
};

template <typename Signature>
struct DynFnHandle {};

// NOTE that this is just an handle and doesn't manage lifetime.
// it is essentially a trivial struct. it is just a
// pointer (resource handle).
// this should never be initialized with a nullptr or invalid handle.
//
// copying and invoking copies of this function handle type could be dangerous
// if done across threads if the handle type mutates its' state upon call.
//
//
// unlike std::function this does not copy the underlying object's state
// (accompanied with memory allocation)
//
//
//
// this is just like std::span, it becomes invalid if and only if the handle
// becomes invalidated.
//
//
// the handle is not allowed to be invalid
//
//
template <typename ReturnType, typename... Args>
struct DynFnHandle<ReturnType(Args...)> {
  explicit constexpr DynFnHandle(DynFn<ReturnType(Args...)>& ihandle)
      : handle{&ihandle}, raw_func_reserve{nullptr} {}

  explicit constexpr DynFnHandle(ReturnType (*raw_fn)(Args...))
      : handle{nullptr}, raw_func_reserve{raw_fn} {}

  ReturnType operator()(Args... args) const {
    if (raw_func_reserve != nullptr) {
      return raw_func_reserve(std::forward<Args>(args)...);
    } else {
      return handle->operator()(std::forward<Args>(args)...);
    }
  }

 private:
  // the backing handle for this function object.
  // either is nullptr, both can not be nullptr
  DynFn<ReturnType(Args...)>* handle = nullptr;
  ReturnType (*raw_func_reserve)(Args...) = nullptr;
};

template <typename Signature>
struct FnHandle {};

template <typename ReturnType, typename... Args>
struct FnHandle<ReturnType(Args...)> {
  explicit constexpr FnHandle(ReturnType (*ifunc)(Args...))
      : func{ifunc}, data{idata} {}

  ReturnType operator()(Args... args) const {
    func(data, std::forward<Arg>(args)...);
  }

 private:
  ReturnType (*func)(void*, Args...) = nullptr;
  // the backing handle for this function object.
  // either is nullptr, both can not be nullptr
  void* data = nullptr;
};

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
  using dyn_fn = DynFn<ReturnType(Args...)>;
  using handle = DynFnHandle<ReturnType(Args...)>;
  using signature = ReturnType(Args...);
  using ptr = ReturnType (*)(Args...);
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
template <typename Type, typename ReturnType, typename... Args>
struct FunctorFn : public DynFn<ReturnType(Args...)> {
  Type functor;

  explicit constexpr FunctorFn(Type&& ifunctor)
      : functor{std::move(ifunctor)} {}

  virtual ReturnType operator()(Args... args) final override {
    return functor(std::forward<Args>(args)...);
  }
};

template <class MemberFunctionSignature>
struct MemberFnTraits {};

// non-const member functions
template <class Type, typename ReturnType, typename... Args>
struct MemberFnTraits<ReturnType (Type::*)(Args...)> {
  using fn = FunctorFn<Type, ReturnType, Args...>;
  using dyn_fn = DynFn<ReturnType(Args...)>;
  using handle = DynFnHandle<ReturnType(Args...)>;
  using signature = ReturnType(Args...);
  using ptr = ReturnType (*)(Args...);
};

// const member functions
template <class Type, typename ReturnType, typename... Args>
struct MemberFnTraits<ReturnType (Type::*)(Args...) const> {
  using fn = FunctorFn<Type, ReturnType, Args...>;
  using dyn_fn = DynFn<ReturnType(Args...)>;
  using handle = DynFnHandle<ReturnType(Args...)>;
  using signature = ReturnType(Args...);
  using ptr = ReturnType (*)(Args...);
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

// TODO(lamarrr): what will happen when recursed?
template <typename FunctorType>
auto make_functor_fn(FunctorType&& fn) {
  static_assert(is_functor<FunctorType>);

  using traits = FunctorFnTraits<FunctorType>;
  using fn_type = typename traits::fn;
  using dyn_type = typename traits::dyn_fn;
  using handle_type = typename traits::handle;

  Rc fn_rc = mem::make_rc(fn_type{std::move(fn)});
  auto handle = handle_type{*fn_rc.get()};

  return stx::transmute(handle, std::move(fn_rc));
}

template <typename FunctorType>
void make_functor_fn(FunctorType& fn) = delete;

template <typename RawFunctionType,
          std::enable_if_t<is_function_pointer<RawFunctionType>, int> = 0>
auto make_static_fn(RawFunctionType function_pointer) {
  using traits = RawFnTraits<RawFunctionType>;
  using dyn_type = typename traits::dyn_fn;
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

  static_assert(std::is_convertible_v<StaticFunctor, fn_ptr_type>);

  auto fn_ptr = static_cast<fn_ptr_type>(functor);

  return make_static_fn(fn_ptr);
}

template <typename Signature>
using Fn = Rc<DynFnHandle<Signature>>;

}  // namespace stx
