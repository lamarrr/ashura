
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <type_traits>
#include <variant>

struct Void {};

struct voidable_invoker {
  template <typename Invocable>
  void invoke();

template<typename Invocable,
std::enable_if_t<std::is_invocable<Invocable, void>, int> = 0
>
  void invoke(   );
};

struct no_result_state {};

template <typename T>
using VoidStubbed = std::conditional_t<!std::is_void_v<T>, T, Void>;

template <typename T>
struct voiddable_rref_t {
  using type = T &&;
};

template <>
struct voiddable_rref_t<void> {
  using type = void;
};

template <typename T>
using voidable_rref = typename voiddable_rref_t<T>::type;

template <typename FirstReturnType, typename... ReturnTypes>
struct chain_state {
  std::variant<no_result_state, VoidStubbed<FirstReturnType>,
               VoidStubbed<ReturnTypes>...>
      last_result = no_result_state{};

  static constexpr uint64_t num_chains = uint64_t{1} + sizeof...(ReturnTypes);

  uint64_t next_executing_function = 0;

  constexpr bool has_executed() const { return next_executing_function != 0; }

  constexpr void mark_finished_executing_current() {
    next_executing_function++;
  }
};

template <typename Func, typename Arg>
struct ChainStep {
  static_assert(std::is_invocable_v<Func, voidable_rref<Arg>>);
  using result_type = std::invoke_result<Func, voidable_rref<Arg>>;
  using Lambda = Func;
};

template <uint64_t Index, typename Arg, typename Func, typename... Funcs>
struct ChainStage {
  // static_assert(std::is_invocable_v<Func, voidable_rref<Arg>>);
  using result = std::invoke_result<Func, voidable_rref<Arg>>;
  using next_stage_type = ChainStage<Index + 1, result, Funcs...>;
  using arg_type = Arg;
  using Lambda = Func;

  Lambda func;
  next_stage_type next_stage;

  template <typename Variant>
  void execute(Variant &variant, uint64_t index) {
    if (Index == index) {
      auto arg = std::move(std::get<arg_type>(variant));
      variant = func(std::move(arg));
    }
    // check suspension and return state if not suspended or canceled, return
    // service token? otherwise: continue executing
    next_stage.execute(variant, index + 1);
  }

  ChainStage(Func &&ifunc, Funcs &&...ifuncs)
      : func{std::move(ifunc)}, next_stage{std::move(ifuncs)...} {}
};

template <uint64_t Index, typename Arg, typename Func>
struct ChainStage<Index, Arg, Func> {
  // static_assert(std::is_invocable_v<Func, Arg &&>);
  using result = std::invoke_result<Func, Arg &&>;
  using arg_type = Arg;
  using Lambda = Func;

  Lambda func;

  ChainStage(Func &&ifunc) : func{std::move(ifunc)} {}
};

template <typename FirstFunc, typename... Func>
struct Chain {
  using stage_type = ChainStage<0, void, FirstFunc, Func...>;

  stage_type stages;

  Chain(FirstFunc &&func, Func &&...funcs)
      : stages{std::move(func), std::move(funcs)...} {}

  template <typename Variant>
  void execute_from(Variant &variant, uint64_t execution_start_index);
};

using x = Chain<int(), int(int), int(int)>;

int fn();
int fnx(int);

auto make() {
  return [stack = std::variant<int, std::nullopt_t>{std::nullopt},
          x = Chain{[]() -> int { return 0; },
                    [](int value) -> int { return value + 28; }}]() mutable {
    x.execute_from(stack, 0);
  };
}

/*
template <int64_t Index, typename Arg, typename FirstCallback,
          typename... TrailingCallbacks>
struct chain_step_result_t {
  static_assert(std::is_invocable_v<FirstCallback, Arg>,
                "Detected Incompatible Task Chain");
  using result =
      typename chain_step_result_t<Index,
                                   std::invoke_result_t<FirstCallback, Arg>,
                                   TrailingCallbacks...>::result;
};
*/
/*
// the 0 should be overriden to use 0 index.

template <int64_t Index, typename Arg, typename Callback>
struct chain_step_result_t<Index, Arg, Callback> {
  static_assert(std::is_invocable_v<Callback, Arg>,
                "Detected Incompatible Task Chain");
  using result = std::invoke_result_t<Callback, Arg>;
};


template <typename Arg, typename ...Callback>
struct chain_step_result_t<0, Arg, Callback...> {
  static_assert(std::is_invocable_v<Callback, Arg>,
                "Detected Incompatible Task Chain");
  using result = std::invoke_result_t<Callback, Arg>;
};

template <typename... InvocableType>
struct invocable_chain {
  std::tuple<InvocableType...> invocables;
};
*/

static_assert(chain_state<int, int, int, float>::num_chains == 4);