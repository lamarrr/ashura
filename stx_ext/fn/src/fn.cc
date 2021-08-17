#include "stx/fn.h"

/*

struct dx {};

static_assert(!stx::is_functor<dx>);

struct ff {
  void operator()(int) {}
  void f();
};

static_assert(stx::is_functor<ff>);

__attribute__((noinline)) void clobber(void*);

// allocate memory for this struct but not know what exactly it allocated for
// and still be able to destroy it
//
//
//
//

struct Entry {
  // letting the user give us an Rc<DynFnHandle> will be extremely problematic
  stx::Rc<stx::DynFnHandle<void()>> fn;
};

//
//
// this is just as hazardrous as std::shared_ptr<std::function<void()>>
//
//
template <typename Signature>
using RcFn = stx::Rc<stx::DynFnHandle<Signature>>;

int main() {
  stx::RawFn<int()> f{main};
  auto fx = stx::make_functor_dynfn([](int x) mutable -> float {});
  fx.get()(4);

  //
  // we can: make rc (performs deletion type erasure)
  // statically allocates on stack.
  //
  //
  // singly allocate a block of args.
  // type erase the function in it
  //
  //
  //
  //
  //
  //
  //
  // make_fn(main);
  // std::function{main};
  // std::function{&main};
  // std::function{&ff::f};
  // std::function{ff{}};
  //
  // using xg = RawFnTraits<decltype(&main)>::fn_type;
  //
  // std::function{[](int) { return 0; }};
  stx::make_functor_dynfn(ff{});

  auto g = []() mutable {};

  // static_assert(is_functor<decltype(g)>);

  stx::make_functor_dynfn([](int) -> int { return 0; });
  auto fn = stx::make_functor_dynfn([]() {});
  clobber(&fn);
  fn.get()();

  using mem_fn_sig = decltype(&decltype(g)::operator());
  using fn_sig = decltype(main);
}
*/