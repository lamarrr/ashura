
/*
template <typename Output>
Future<Output> schedule(RcFn<Output()> task, TaskPriority priority,
                        TaskTraceInfo trace_info) {
  Promise promise = stx::make_promise<Output>();
  Future future = Future<Output>(promise);

  auto packaged_task =
      stx::make_functor_fn([task___ = std::move(task), promise___ = promise]() {
        if constexpr (!std::is_void_v<Output>) {
          promise___.notify_completed(task___.get()());
        } else {
          task___.get()();
          promise___.notify_completed();
        }
      });

  Task entry{std::move(packaged_task),
             PromiseAny{promise},
             priority,
             std::move(trace_info),
             TaskEntryState ::Scheduled,
             stx::make_static_fn([]() { return true; }),
             FutureAny{future}};

  entries.emplace_back(std::move(entry));

  return future;
}

template <typename... ChainArgs>
auto schedule(Chain<ChainArgs...> &&chain, TaskPriority priority,
              TaskTraceInfo trace_info) {
  using chain_type = std::remove_reference_t<decltype(chain)>;

  using stack_type = typename chain_type::stack_type;

  stack_type stack = Void{};

  using chain_result_type =
      typename chain_type::stages_type::last_stage_result_type;

  Promise promise = stx::make_promise<chain_result_type>();
  /////////////////////////////////////////////// this shouldn't compile
  Future future = Future<chain_result_type>(promise);

  auto packaged_task = [chain___ = std::move(chain),
                        stack___ = std::move(stack), promise___ = promise,
                        state___ = stx::ChainState{}]() mutable {
    stx::RequestProxy proxy{promise___};
    chain___.resume(stack___, state___, proxy);

    // task was suspended or canceled.
    // notify scheduler and user of which was
    if (state___.next_phase_index < chain_type::num_stages) {
      // canceled
      if (state___.service_token.type == stx::RequestType::Cancel) {
        if (state___.service_token.source == stx::RequestSource::User) {
          promise___.notify_user_canceled();
        } else {
          promise___.notify_force_canceled();
        }
      } else {
        // suspended
        if (state___.service_token.source == stx::RequestSource::User) {
          promise___.notify_user_suspended();
        } else {
          promise___.notify_force_suspended();
        }
      }
    } else {
      // tasks completed
      promise___.notify_completed(
          std::move(std::get<chain_result_type>(stack___)));
    }
  };

  auto packaged_task_fn = stx::make_functor_fn(std::move(packaged_task));

  // TODO(lamarrr): suspension and cancelation management and tracking.

  raw_schedule(Task{packaged_task_fn, PromiseAny{promise}, priority, trace_info,
                    TaskEntryState::Scheduled,
                    stx::make_static_fn([]() { return true; }),
                    FutureAny{future}});

  return future;
}
*/