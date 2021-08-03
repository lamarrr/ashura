
#include <atomic>

#include "stx/span.h"
// not movable nor copyable?

template <typename T>
struct SpScRingBufferAdapter {
  // we need to either launder or require default construction to use aligned
  // storage

  SpScRingBufferAdapter(stx::Span<T> output) {
    // check size is power of 2 and greater than 0
  }

  void write(T const&) {
    auto write_pos = advance_read_barrier_by(1) & managed.size();
    advance_written_index_by(1);
  }

  // void write_range(uint64_t dest_begin,  stx::Span<T const> values){}

  // void read_range(uint64_t dest_begin,  stx::Span<T> values)
  // returns previous size
  // no
  uint64_t advance_read_barrier_by(uint64_t size) {
    return read_index_.fetch_add(size, std::memory_order_release);
  }

  // returns previous index
  uint64_t advance_write_beacon_by(uint64_t size) {
    return write_index_.fetch_add(size, std::memory_order_release);
  }

  std::atomic_uint64_t read_index_ = 0;
  std::atomic_uint64_t write_index_ = 0;

  stx::Span<T> managed;
};
