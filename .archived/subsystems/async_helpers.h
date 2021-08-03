
// used for storing the results of async operations that are only launched in
// sequence.
//
//
// TODO(lamarrr): state that the user must use either of Seqasyncresult or
// parallelasyncresult to ensure proper behaviour in reference capture. or
// read-only ref captures
template <typename T>
struct SequentialAsyncSink {
  T copy() const { return value.value; }

  T move() { return std::move(value.value); }

  // should only be called from the async function
  void write(T&& new_value) { value.value = std::move(new_value); }

  CacheLineAligned<T> value;
};

namespace impl {
template <typename T>
struct ParallelAsyncSinkStorage {
  T value;
  std::mutex mutex;
};
};  // namespace impl

// used for storing the results of async operations that could be overlapping or
// possibly submitted in parallel. i.e. re-submitted async tasks.
//
// use this for collecting results of parallel tasks that could possibly overlap
// or execute at the same time.
//

template <typename T>
struct ParallelAsyncSink {
  ParallelAsyncSink(T initial_value) {}

  T copy() const {
    // problem: blocks
    std::lock_guard guard{storage_.value.mutex};
    T new_copy{storage_.value.value};
    return new_copy;
  }

  T move() {
    // problem: blocks
    std::lock_guard guard{storage_.value.mutex};
    T out{std::move(storage_.value.value)};
    return std::move(out);
  }

  // should only be called from the async function
  void write(T&& new_value) {
    // problem: blocks
    std::lock_guard guard{storage_.value.mutex};
    storage_.value.value = std::move(new_value);
  }

 private:
  CacheLineAligned<impl::ParallelAsyncSinkStorage<T>> storage_;
};

  // this is aligned to the cache line size via heap allocation which should
  // make allocations chunked to std::max_align_t granularity. so we shouldn't
  // have cache coherence issues.