#pragma once

namespace ash
{

/// NOTE that this is just an handle and doesn't manage any lifetime.
///
/// it is essentially a trivial struct. it is just contains pointers (resource
/// handles).
///
template <typename Signature>
struct Fn;

template <typename ReturnType, typename... Args>
struct Fn<ReturnType(Args...)>
{
  using Dispatcher = ReturnType (*)(void *, Args...);

  constexpr ReturnType operator()(Args... args) const
  {
    return dispatcher(data, static_cast<Args &&>(args)...);
  }

  Dispatcher dispatcher = nullptr;
  void      *data       = nullptr;
};

template <typename ReturnType, typename... Args>
struct RawFunctionDispatcher
{
  static constexpr ReturnType dispatch(void *data, Args... args)
  {
    using Ptr = ReturnType (*)(Args...);

    Ptr function_ptr = reinterpret_cast<Ptr>(data);

    return function_ptr(static_cast<Args &&>(args)...);
  }
};

template <typename RawFunctionType>
struct RawFnTraits
{
};

template <typename ReturnType, typename... Args>
struct RawFnTraits<ReturnType(Args...)>
{
  using ptr         = ReturnType (*)(Args...);
  using signature   = ReturnType(Args...);
  using fn          = Fn<signature>;
  using dispatcher  = RawFunctionDispatcher<ReturnType, Args...>;
  using return_type = ReturnType;
};

template <typename ReturnType, typename... Args>
struct RawFnTraits<ReturnType (*)(Args...)>
    : public RawFnTraits<ReturnType(Args...)>
{
};

template <typename Type, typename ReturnType, typename... Args>
struct FunctorDispatcher
{
  static constexpr ReturnType dispatch(void *data, Args... args)
  {
    return (*(reinterpret_cast<Type *>(data)))(static_cast<Args &&>(args)...);
  }
};

template <class MemberFunctionSignature>
struct MemberFnTraits
{
};

// non-const member functions
template <class Type, typename ReturnType, typename... Args>
struct MemberFnTraits<ReturnType (Type::*)(Args...)>
{
  using ptr         = ReturnType (*)(Args...);
  using signature   = ReturnType(Args...);
  using fn          = Fn<signature>;
  using type        = Type;
  using dispatcher  = FunctorDispatcher<type, ReturnType, Args...>;
  using return_type = ReturnType;
};

// const member functions
template <class Type, typename ReturnType, typename... Args>
struct MemberFnTraits<ReturnType (Type::*)(Args...) const>
{
  using ptr         = ReturnType (*)(Args...);
  using signature   = ReturnType(Args...);
  using fn          = Fn<signature>;
  using type        = Type const;
  using dispatcher  = FunctorDispatcher<type, ReturnType, Args...>;
  using return_type = ReturnType;
};

template <class Type>
struct FunctorFnTraits : public MemberFnTraits<decltype(&Type::operator())>
{
};

// make a function view from a functor reference. Functor should outlive the Fn
template <typename Functor>
auto make_functor_fn(Functor &functor)
{
  using traits     = FunctorFnTraits<Functor>;
  using fn         = typename traits::fn;
  using dispatcher = typename traits::dispatcher;

  return fn{&dispatcher::dispatch,
            const_cast<void *>(reinterpret_cast<void const *>(&functor))};
}

// make a function view from a raw function pointer.
template <typename RawFunctionType>
auto make_fn(RawFunctionType *function_pointer)
{
  using traits     = RawFnTraits<RawFunctionType>;
  using fn         = typename traits::fn;
  using dispatcher = typename traits::dispatcher;

  return fn{&dispatcher::dispatch, reinterpret_cast<void *>(function_pointer)};
}

// make a function view from a data-less functor (i.e. lambda's without data)
template <typename StaticFunctor>
auto make_static_functor_fn(StaticFunctor functor)
{
  using traits = FunctorFnTraits<StaticFunctor>;
  using ptr    = typename traits::ptr;

  ptr function_pointer = static_cast<ptr>(functor);

  return make_fn(function_pointer);
}

}        // namespace ash
