#pragma once

#include <cinttypes>
#include <functional>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "stx/async.h"
#include "stx/fn.h"
#include "stx/mem.h"
#include "stx/option.h"
#include "stx/task/priority.h"
#include "vlk/subsystem/impl.h"
#include "vlk/utils.h"

namespace vlk {

using namespace std::chrono_literals;

struct TaskTraceInfo {
  stx::Rc<std::string_view> context =
      stx::mem::make_static_string_rc("Unnamed Context");
  stx::Rc<std::string_view> purpose =
      stx::mem::make_static_string_rc("Unspecified Purpose");
  stx::Rc<std::string_view> additional_context =
      stx::mem::make_static_string_rc("");
};

struct TaskDispatcher {
  explicit TaskDispatcher(uint64_t) {}

  virtual void dispatch(std::function<void()> &&task,
                        TaskTraceInfo &&trace_info,
                        uint64_t physical_unit_index) = 0;

 private:
  uint64_t num_allocated_physical_units_ = 0;
};

struct ThreadPool : public SubsystemImpl {};

/*
template <int64_t Index, typename Arg, typename FirstCallback,
          typename... TrailingCallbacks>
struct chain_step_result_t {
  static_assert(std::is_invocable_v<FirstCallback, Arg>,
                "Detected Incompatible Task Chain");
  using result =
      typename chain_step_result_t<std::invoke_result_t<FirstCallback, Arg>,
                                   TrailingCallbacks...>::result;

  using variant = std::variant<>;
};

// the 0 should be overriden to use 0 index.

template <int64_t Index, typename Arg, typename Callback>
struct chain_step_result_t<Index, Arg, Callback> {
  static_assert(std::is_invocable_v<Callback, Arg>,
                "Detected Incompatible Task Chain");
  using result = std::invoke_result_t<Callback, Arg>;
};

template <typename FirstReturnType, typename... ReturnTypes>
struct chain_state {
  std::variant<FirstReturnType, ReturnTypes...> state;
  static constexpr num_chains = 1 + sizeof(ReturnTypes);
  int64_t executing_function_index = -1;
};

// automatically generates suspension and cancelation points
//
//
//
template <typename Callback1, typename... Callbacks>
struct chain_result_t {};

template <int64_t Index, typename T>
struct Indexed {
  T value;
};

template <typename Invocable1, typename... ChainableInvocables>
std::function<void(stx::RequestProxy const&)> make_chained(
    Invocable1&& invocable, ChainableInvocables&&... invocables) {
  static_assert(std::is_invocable_v<invocable> &&
                std::invoke_result_t<invocable>);

  [executing_function_index = int64_t{0},
   last_result =
       std::variant<stx::NoneType, Indexed<32, ChainableInvocables>...>{
           stx::None}](stx::RequestProxy const& proxy)
      -> stx::Result<std::invoke_result_t<Invocable1>, stx::ServiceToken> {
    // invoke callback
    last_result = ...;
    executing_function_index++;
  };
}
*/

struct Void {};

template <typename T, typename... Ts>
struct filter_duplicates {
  using type = T;
};

template <template <typename...> class C, typename... Ts, typename U,
          typename... Us>
struct filter_duplicates<C<Ts...>, U, Us...>
    : std::conditional_t<(std::is_same_v<U, Ts> || ...),
                         filter_duplicates<C<Ts...>, Us...>,
                         filter_duplicates<C<Ts..., U>, Us...>> {};

template <typename T>
struct unique_variant_impl;

template <typename... Ts>
struct unique_variant_impl<std::variant<Ts...>>
    : filter_duplicates<std::variant<>, Ts...> {};

template <typename T>
using unique_variant = typename unique_variant_impl<T>::type;

/*
template <typename FirstReturnType, typename... ReturnTypes>
struct chain_state {
  std::variant<no_result_state, FirstReturnType, ReturnTypes...> last_result =
      no_result_state{};

  static constexpr uint8_t num_chains = uint8_t{1} + sizeof...(ReturnTypes);

  uint8_t next_executing_function = 0;

  constexpr bool has_executed() const { return next_executing_function != 0; }

  constexpr void mark_finished_executing_current() {
    next_executing_function++;
  }
};
*/

/*
template <typename Fn, typename Arg>
struct ChainStep {
  static_assert(std::is_invocable_v<Fn, Arg &&>);
  using result_type = std::invoke_result<Fn, Arg &&>;
  using fn = Fn;
};
*/

struct ChainState {
  // only valid if next_stage_index != (num_chains)
  stx::ServiceToken service_token{};
  // once next_stage_index == num_chains then the chain has reached completion,
  // otherwise, it is assumed to be in a suspended state
  uint8_t next_stage_index = 0;
};

template <uint8_t Index, typename Arg, typename Fn, typename... OtherFns>
struct ChainStage {
  using fn_type = stx::impl::fn_decay<Fn>;
  using arg_type = Arg;

  static_assert(!std::is_void_v<arg_type>);

  using result_type = std::invoke_result_t<fn_type, arg_type &&>;

  static_assert(!std::is_void_v<result_type>);

  static_assert(std::is_invocable_v<fn_type, arg_type &&>);

  using next_stage_type = ChainStage<Index + 1, result_type, OtherFns...>;

  fn_type fn;
  next_stage_type next_stage;

  using last_stage_result_type =
      typename next_stage_type::last_stage_result_type;

  template <typename Variant>
  void resume(Variant &variant, ChainState &state, stx::RequestProxy &proxy) {
    // is this fn the intended resumption point? then start execution from
    // here, otherwise skip this fn and pass on to the desired lamda
    if (Index == state.next_stage_index) {
      arg_type &&arg = std::move(std::get<arg_type>(variant));
      variant = fn(std::move(arg));
      state.next_stage_index++;

      // check suspension and cancelation requests before passing on to the next
      // fn otherwise: continue executing
      stx::CancelRequest const cancel_request = proxy.fetch_cancel_request();
      stx::SuspendRequest const suspend_request = proxy.fetch_suspend_request();

      if (cancel_request.state == stx::RequestedCancelState::Canceled) {
        state.service_token = stx::ServiceToken{cancel_request};
        return;
      }

      if (suspend_request.state == stx::RequestedSuspendState::Suspended) {
        state.service_token = stx::ServiceToken{suspend_request};
        return;
      }

      next_stage.resume(variant, state, proxy);
    } else {
      next_stage.resume(variant, state, proxy);
    }
  }

  explicit ChainStage(Fn &&ifn, OtherFns &&... iothers)
      : fn{std::move(ifn)}, next_stage{std::move(iothers)...} {}
};

template <uint8_t Index, typename Arg, typename Fn>
struct ChainStage<Index, Arg, Fn> {
  using fn_type = stx::impl::fn_decay<Fn>;
  using arg_type = Arg;

  static_assert(!std::is_void_v<arg_type>);

  using result_type = std::invoke_result_t<fn_type, arg_type &&>;

  static_assert(!std::is_void_v<result_type>);

  static_assert(std::is_invocable_v<fn_type, arg_type &&>);

  using last_stage_result_type = result_type;

  fn_type fn;

  explicit ChainStage(fn_type &&ifn) : fn{std::move(ifn)} {}

  template <typename Variant>
  void resume(Variant &variant, ChainState &state, stx::RequestProxy &) {
    if (Index == state.next_stage_index) {
      arg_type &&arg = std::move(std::get<arg_type>(variant));
      variant = fn(std::move(arg));
      state.next_stage_index++;
      return;
    }
  }
};

template <typename Fn, typename... OtherFns>
struct Chain {
  static_assert((1 + sizeof...(OtherFns)) <= (stx::u8_max - 1));

  using stages_type = ChainStage<0, Void, Fn, OtherFns...>;

  static constexpr uint8_t num_stages = (1 + sizeof...(OtherFns));

  stages_type stages;

  explicit Chain(Fn &&fn, OtherFns &&... others)
      : stages{std::move(fn), std::move(others)...} {}

  template <typename Variant>
  void resume(Variant &variant, ChainState &state, stx::RequestProxy &proxy) {
    stages.resume(variant, state, proxy);
  }
};

void fhdg() {
  Chain chain{[](Void) -> int {
                VLK_LOG("executing 1 ...");
                std::this_thread::sleep_for(1s);
                return 0;
              },
              [](int x) -> int {
                VLK_LOG("executing 2 ...");
                std::this_thread::sleep_for(1s);
                return x + 1;
              },
              [](int x) -> float {
                VLK_LOG("executing 3 ...");
                std::this_thread::sleep_for(1s);
                return x + 2;
              },
              [](float y) -> int {
                VLK_LOG("executing 4 ...");
                std::this_thread::sleep_for(1s);
                return y + 5;
              }};

  using chain_type = decltype(chain);

  std::variant<Void, int, float> var = Void{};

  using chain_result_type =
      typename chain_type::stages_type::last_stage_result_type;
  auto [future, promise] = stx::make_future<chain_result_type>();

  stx::Rc dyn_fn = stx::make_functor_dynfn(
      [chain_x = std::move(chain), var_x = std::move(var),
       promise_x = std::move(promise), state = vlk::ChainState{}]() mutable {
        stx::RequestProxy proxy{promise_x};
        chain_x.resume(var_x, state, proxy);

        if (state.next_stage_index < chain_type::num_stages) {
          // task was suspended or canceled
          // notify scheduler of which
          if (state.service_token.type == stx::RequestType::Cancel) {
            VLK_LOG("task canceled");
          } else {
            VLK_LOG("task suspended");
          }
        } else {
          // tasks completed
          promise_x.notify_completed(
              std::move(std::get<chain_result_type>(var_x)));
          VLK_LOG("tasks completed");
        }
      });

  // future.request_suspend();
  dyn_fn.get()();
}

/*
struct TaskScheduler : public SubsystemImpl {
  explicit TaskScheduler(stx::mem::Rc<TaskDispatcher> dispatcher) {
    VLK_ENSURE(dispatcher.get()->num_allocated_physical_units > 0);
  }

  // TODO(lamarrr): how is suspension and cancelation affected by this
  // mechanism???
  //
  //
  //
  //
  //
  //
  // void overloads
  //
  //
  //
  //
  // TODO(lamarrr): taking in futures of any type?
  //
  //
  //
  template <typename... Inputs, typename Output>
  stx::Future<Output> await(
      std::function<stx::Result<Output, stx::ServiceToken>(
          stx::RequestProxy const &, stx::Future<Inputs> &&...)> &&task,
      stx::TaskPriority priority, TaskTraceInfo trace_info,
      stx::Future<Inputs>... inputs);

  template <typename... Inputs>
  stx::Future<void> await(
      std::function<stx::Result<Void, stx::ServiceToken>(
          stx::RequestProxy const &, stx::Future<Inputs> &&...)> &&task,
      stx::TaskPriority priority, TaskTraceInfo trace_info,
      stx::Future<Inputs>... inputs);

  template <typename... Inputs, typename Output>
  stx::Future<Output> await(
      std::function<Output(stx::Future<Inputs> &&...)> &&task,
      stx::TaskPriority priority, TaskTraceInfo trace_info,
      stx::Future<Inputs>... inputs);

  //

  template <typename Output>
  stx::Future<Output> schedule(
      std::function<stx::Result<Output, stx::ServiceToken>(
          stx::RequestProxy const &)> &&task,
      stx::TaskPriority priority, TaskTraceInfo trace_info);

  stx::Future<void> schedule(std::function<stx::Result<Void, stx::ServiceToken>(
                                 stx::RequestProxy const &)> &&task,
                             stx::TaskPriority priority,
                             TaskTraceInfo trace_info);

  template <typename Output>
  stx::Future<Output> schedule(std::function<Output()> &&task,
                               stx::TaskPriority priority,
                               TaskTraceInfo trace_info);

  stx::FutureAny get_future() final {
    return stx::FutureAny{future.take().unwrap()};
  }

  void link(SubsystemsContext const &) final {}
  void tick(std::chrono::nanoseconds) final {
    // cancel requested for scheduler?
  }

  enum class TaskState { Scheduled, Awaiting, Running, Preempted };

  // we need stable references? once preemption is attended to we need to update
  // this struct
  struct Entry {
    TaskState state = TaskState::Scheduled;
    std::vector<stx::FutureAny> await_futures;
    std::function<void()> task;
    stx::TaskPriority priority = stx::TaskPriority::Background;
    TaskTraceInfo trace_info;

    bool operator==(Entry const &other) const;
    bool operator!=(Entry const &other) const;
    bool operator<(Entry const &other) const;
    bool operator>(Entry const &other) const;
    bool operator<=(Entry const &other) const;
    bool operator>=(Entry const &other) const;
  };

  struct ExecutionInfo {
    STX_MAKE_PINNED(ExecutionInfo)
    // once this is less than the number of physical units, send more tasks
    STX_CACHELINE_ALIGNED std::atomic<int64_t> num_executing{0};
  };

  stx::Promise<void> promise;
  stx::Option<stx::Future<void>> future;
  stx::mem::Rc<ExecutionInfo> execution_info =
      stx::mem::make_rc_inplace<ExecutionInfo>();
};
*/
}  // namespace vlk
