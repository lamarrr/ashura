

enum class FutureError { Pending, Canceled };

namespace impl {
struct FutureShim {};
}  // namespace impl

template <typename T>
struct FutureStatus {
  union {
    T result;
    impl::FutureShim shim;
  };
  CompletionStatus status;
};

template <typename T>
struct FutureState {
  CacheLineAligned<FutureStatus> status;
};

// a one-shot future
template <typename T>
struct Future {
  stx::Result<T, FutureError> move() {}
  stx::Result<T, FutureError> copy() {}

  bool is_canceled() const {}
  bool is_completed() const {}
  void await_finish() const {}

 private:
  std::shared_ptr<FutureState<T>> state;
};

// struct FutureVoid;

template <typename T>
struct FutureToken {
  explicit FutureToken(Future<T> const& future) { state = future.state; }

  void mark_completed(T&& result) const {
    output_ = std::move(result);
    observer_->mark_completed();
  }

  void mark_canceled() const { observer_->mark_canceled(); }

  std::weak_ptr<FutureState<T>> state;
};

enum class Canceled {};



namespace impl {
inline void backoff_spin(uint64_t iteration) {
  if (iteration < 64) {
    // immediate spinning
  } else if (iteration < 128) {
    // if there are any threads that need execution, let them execute before
    // attending to us
    std::this_thread::yield();
  } else {
    // sleep for a specific amount of time
    std::this_thread::sleep_for(std::chrono::milliseconds(125));
  }
}

inline void backoff_acquire_exclusive(std::atomic_bool& is_acquired) {}

}  // namespace impl


void blocking_await(CompletionObserver const& observer) {
  // perform spin lock
  uint64_t num_uneventful_iterations = 0;

  CompletionStatus status = CompletionStatus::Pending;
  do {
    status = get_status();
    num_uneventful_iterations++;
    impl::backoff_spin(num_uneventful_iterations);
  } while (status != CompletionStatus::Completed ||
           status != CompletionStatus::Canceled);
}

struct CompletionObserver {
  friend struct CompletionToken;

  CompletionObserver() {}

  static CompletionObserver create() {
    CompletionObserver observer;
    observer.state = std::shared_ptr<CompletionState>(new CompletionState);

    return std::move(observer);
  }

  auto get_status() const { return state->get_status(); }

  bool is_valid() const { return state == nullptr; }

 private:
  // this is aligned to the cache line size via heap allocation which should
  // make allocations chunked to std::max_align_t granularity. so we shouldn't
  // have cache coherence issues.
  std::shared_ptr<CompletionState> state;
};


struct CompletionState {
  CacheLineAligned<std::atomic<CompletionStatus>> status{
      CompletionStatus::Pending};

  auto get_status() const {
    return status.value.load(std::memory_order_acquire);
  }

  void mark_completed() {
    status.value.store(CompletionStatus::Completed, std::memory_order_release);
  }

  void mark_canceled() {
    status.value.store(CompletionStatus::Canceled, std::memory_order_release);
  }
};