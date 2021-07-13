#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "stx/option.h"
#include "stx/span.h"

namespace vlk {

namespace trace {
// causal tracing?

enum class EventType { Begin, End };

enum class EventUID : uint64_t {};

enum class TraceDiff { None, Some };

struct Event {
  // must reference a static string
  std::string_view category;

  // must reference a static string or string sourced from a shared_ptr and
  // whose lifetime is extended by `identifier_lifetime_extender`
  std::string_view identifier;

  std::chrono::steady_clock::time_point timepoint;
  EventType type = EventType::Begin;
  EventUID uid = EventUID{0};

  std::shared_ptr<void const> identifier_lifetime_extender;
};

struct EventRingBuffer {
  EventRingBuffer(uint64_t capacity) : capacity_{capacity} {
    VLK_ENSURE(capacity != 0);
    buffer_.resize(capacity);
  }

  void push(Event event) {
    buffer_[next_insert_pos_] = event;
    next_insert_pos_ = (next_insert_pos_ + 1) % capacity_;
    num_valid_insertions_ = std::min(num_valid_insertions_ + 1, capacity_);
    trailing_diff_ = TraceDiff::Some;
    leading_diff_ = TraceDiff::Some;
  }

  std::pair<stx::Span<Event const>, TraceDiff> get_trailing_fragment() {
    TraceDiff previous_diff = trailing_diff_;
    trailing_diff_ = TraceDiff::None;
    return std::make_pair(
        num_valid_insertions_ == capacity_
            ? stx::Span<Event const>(buffer_).subspan(next_insert_pos_)
            : stx::Span<Event const>(buffer_).subspan(0, num_valid_insertions_),
        previous_diff);
  }

  std::pair<stx::Span<Event const>, TraceDiff> get_leading_fragment() {
    TraceDiff previous_diff = leading_diff_;
    leading_diff_ = TraceDiff::None;
    return std::make_pair(
        num_valid_insertions_ == capacity_
            ? stx::Span<Event const>(buffer_).subspan(0, next_insert_pos_)
            : stx::Span<Event const>(),
        previous_diff);
  }

 private:
  std::vector<Event> buffer_;
  TraceDiff trailing_diff_ = TraceDiff::None;
  TraceDiff leading_diff_ = TraceDiff::None;
  uint64_t capacity_ = 0;
  uint64_t next_insert_pos_ = 0;
  uint64_t num_valid_insertions_ = 0;
};

constexpr uint64_t default_trace_entries_limit = 128;

struct SingleThreadContext {
  friend struct EventTrace;

  explicit SingleThreadContext(
      uint64_t entries_limit = default_trace_entries_limit)
      : buffer_{entries_limit} {}

  struct UIDProducer {
    uint64_t last_uid = 0;
    EventUID produce() {
      auto out_uid = last_uid;
      last_uid++;
      return EventUID{out_uid};
    }
  };

 private:
  void add_event(std::string_view category, std::string_view identifier,
                 EventType type) {
    buffer_.push(Event{category, identifier, std::chrono::steady_clock::now(),
                       type, uid_producer_.produce()});
  }

 private:
  EventRingBuffer buffer_;
  UIDProducer uid_producer_;
};

// for tracing events that begin and end on the same thread
// strings passed must live as long as context
struct EventTrace {
  EventTrace(SingleThreadContext& context, std::string_view category,
             std::string_view identifier) {
    context_ = &context;
    category_ = category;
    identifier_ = identifier;
    context_->add_event(category_, identifier_, EventType::Begin);
  }

  ~EventTrace() { context_->add_event(category_, identifier_, EventType::End); }

 private:
  std::string_view category_;
  std::string_view identifier_;
  SingleThreadContext* context_ = nullptr;
};

void trace_static();
void trace_shared();

// Requirements: we don't want to block progression of tasks whilst reading the
// info we can have an internal queue that replicates the content and
// immediately copies this info to it
// this will be a sort of mirror? we'd also need to know if anything changed
//

// each thread should have a fixed ring buffer for adding events to, we would
// therefore not need to synchronize access between the producer threads. but
// we'd have to syncrhonize acess to the consumer threads which would rarely
// run?
//
// we don't want to block progression of tasks on the thread that calls this
// function
//
//
// TODO(lamarrr): test all
struct MultiThreadContext {
  struct Context {
    SingleThreadContext context;
    // mutex doesn't have copy nor move constructors, thus can't be used
    // conveniently with std::vector without a unique_ptr
    std::unique_ptr<std::mutex> mutex;
  };

  MultiThreadContext(uint64_t num_threads, uint64_t entries_limit_per_thread =
                                               default_trace_entries_limit)
      : entries_limit_per_thread_{entries_limit_per_thread} {
    for (uint64_t i = 0; i < num_threads; i++) {
      thread_contexts_.push_back(
          Context{SingleThreadContext{entries_limit_per_thread},
                  std::unique_ptr<std::mutex>{new std::mutex{}}});
    }
    // resize accumulation
  }

  uint64_t get_num_threads() const { return thread_contexts_.size(); }

  uint64_t get_entries_limit_per_thread() const {
    return entries_limit_per_thread_;
  }

  auto get_trailing_fragment(uint64_t thread_index) {
    VLK_ENSURE(thread_index < thread_index);
    auto& acc = accumulation_[thread_index];
    return acc.get_trailing_fragment();
  }

  auto get_leading_fragment(uint64_t thread_index) {
    VLK_ENSURE(thread_index < thread_index);
    auto& acc = accumulation_[thread_index];
    return acc.get_leading_fragment();
  }

  // we can't block the UI thread
  // the accumulator should be internal to us so we can trace and have control
  // over it
  //
  //
  // should be called at short intervals to avoid blocking the worker threads
  //
  //
  void tick(std::chrono::nanoseconds interval) {
    for (auto& context_ : thread_contexts_) {
      if (context_.mutex->try_lock()) {
        //....
        // if different update

        context_.mutex->unlock();
      } else {
      }
    }
  }

  std::vector<Context> thread_contexts_;
  std::vector<EventRingBuffer> accumulation_;
  uint64_t entries_limit_per_thread_ = 0;
};

}  // namespace trace

}  // namespace vlk

// TODO(lamarrr): we don't need macro for this, we need a function that can
// create any type of required strtuct and dispatch appropriately
#define VLK_TRACE(context_identifier, category, trace_identifier) \
  vlk::trace::EventTrace { context_identifier, category, trace_identifier }
