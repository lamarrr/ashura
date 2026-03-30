/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.hpp"

namespace ash
{

template <typename Sig>
struct PFnThunk;

template <typename R, typename... Args>
struct PFnThunk<R(Args...)>
{
    static constexpr R thunk(void * data, Args... args)
    {
        using PFn = R (*)(Args...);

        PFn pfn = reinterpret_cast<PFn>(data);

        return pfn(static_cast<Args &&>(args)...);
    }
};

template <typename T, typename Sig>
struct FunctorThunk;

template <typename T, typename R, typename... Args>
struct FunctorThunk<T, R(Args...)>
{
    static constexpr R thunk(void * data, Args... args)
    {
        return (*(reinterpret_cast<T *>(data)))(static_cast<Args &&>(args)...);
    }
};

template <typename Sig>
struct PFnTraits;

template <typename R, typename... Args>
struct PFnTraits<R(Args...)>
{
    using Ptr    = R (*)(Args...);
    using Return = R;
    using Thunk  = PFnThunk<R(Args...)>;
};

template <typename R, typename... Args>
struct PFnTraits<R (*)(Args...)> : PFnTraits<R(Args...)>
{
};

template <typename Sig>
struct MethodTraits;

/// @brief Non-const method traits
template <typename T, typename R, typename... Args>
struct MethodTraits<R (T::*)(Args...)>
{
    using Ptr    = R (*)(Args...);
    using Type   = T;
    using Return = R;
    using Thunk  = FunctorThunk<T, R(Args...)>;
};

/// @brief Const method traits
template <typename T, typename R, typename... Args>
struct MethodTraits<R (T::*)(Args...) const>
{
    using Ptr    = R (*)(Args...);
    using Type   = T const;
    using Return = R;
    using Thunk  = FunctorThunk<T const, R(Args...)>;
};

template <typename F>
struct FunctorTraits : MethodTraits<decltype(&F::operator())>
{
};

template <typename F, typename R, typename... Args>
concept CallableOf = requires (F f, Args... args) {
    { f(static_cast<Args &&>(args)...) } -> Same<R>;
};

template <typename Sig>
struct Fn;

/// @brief Fn is a type-erased function containing a callback and a pointer. Fn
/// is a reference to both the function to be called and its associated data, it
/// doesn't manage any lifetime.
/// @param thunk function/callback to be invoked. typically a
/// dispatcher/thunk.
/// @param data associated data/context for the thunk to operate on.
template <typename R, typename... Args>
struct Fn<R(Args...)>
{
    using Thunk = R (*)(void *, Args...);

    void * data;

    Thunk thunk;

    constexpr Fn(Fn const &)             = default;
    constexpr Fn(Fn &&)                  = default;
    constexpr Fn & operator=(Fn const &) = default;
    constexpr Fn & operator=(Fn &&)      = default;
    constexpr ~Fn()                      = default;

    constexpr Fn(void * data, Thunk thunk) : data{data}, thunk{thunk}
    {
    }

    static constexpr Fn null()
    {
        return Fn{nullptr, nullptr};
    }

    Fn(R (*pfn)(Args...)) :
      data{reinterpret_cast<void *>(pfn)},
      thunk{&PFnThunk<R(Args...)>::thunk}
    {
    }

    template <typename PFn>
    requires (Convertible<PFn, R (*)(Args...)>)
    Fn(PFn pfn) : Fn{static_cast<R (*)(Args...)>(pfn)}
    {
    }

    /// @brief Create a function view from an object reference and a function
    /// thunk to execute using the object reference as its first argument.
    template <typename T>
    Fn(T * data, R (*thunk)(T *, Args...)) :
      data{const_cast<void *>(reinterpret_cast<void const *>(data))},
      thunk{reinterpret_cast<Thunk>(thunk)}
    {
    }

    template <typename T, typename PFn>
    requires (Convertible<PFn, R (*)(T *, Args...)>)
    Fn(T * data, PFn thunk) : Fn{data, static_cast<R (*)(T *, Args...)>(thunk)}
    {
    }

    /// @brief Make a function view from a functor reference. Functor should
    /// outlive the Fn
    template <typename F>
    requires (CallableOf<F, R, Args...>)
    Fn(F * functor) : Fn{(void *) (functor), &FunctorThunk<F, R(Args...)>::thunk}
    {
    }

    constexpr R operator()(Args... args) const
    {
        return thunk(data, static_cast<Args &&>(args)...);
    }
};

template <typename R, typename... Args>
Fn(R (*)(Args...)) -> Fn<R(Args...)>;

template <typename T, typename R, typename... Args>
Fn(T *, R (*)(T *, Args...)) -> Fn<R(Args...)>;

template <typename Tag, typename Sig>
struct TagFn;

template <typename Tag, typename R, typename... Args>
struct TagFn<Tag, R(Args...)>
{
    using Thunk = R (*)(Tag, Args...);

    Tag tag;

    Thunk thunk;

    constexpr TagFn(TagFn const &)             = default;
    constexpr TagFn(TagFn &&)                  = default;
    constexpr TagFn & operator=(TagFn const &) = default;
    constexpr TagFn & operator=(TagFn &&)      = default;
    constexpr ~TagFn()                         = default;

    constexpr TagFn(Tag tag, Thunk thunk) : tag{tag}, thunk{thunk}
    {
    }

    constexpr R operator()(Args... args) const
    {
        return thunk(tag, static_cast<Args &&>(args)...);
    }
};

template <typename R, typename... Args>
struct Consume<Fn<R(Args...)>>
{
    static constexpr auto consume(Fn<R(Args...)> & v)
    {
        auto out = v;
        v        = Fn<R(Args...)>::null();
        return out;
    }
};

}    // namespace ash
