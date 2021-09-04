
#include <atomic>
#include <cinttypes>
#include <utility>

#include "stx/allocator.h"
#include "stx/async.h"
#include "stx/manager.h"
#include "stx/mem.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/spinlock.h"

// THIS is just an annotation really.
//
// RULES:
//
// - must not execute user code, i.e. generic types
// - operations performed must take constant time and must be extremely
// short-lived. you either be able to say specifically what amount of time it
// takes or not.
//
//
#define CRITICAL_SECTION(...) \
  do                          \
    __VA_ARGS__               \
  while (false)

namespace stx {

template <typename T, typename Index = size_t>
struct Enumerated {
  template <typename... Args>
  explicit constexpr Enumerated(Index iindex, Args &&... args)
      : index{iindex}, value{std::forward<Args>(args)...} {}

  Index index = 0;
  T value;
};

template <typename T, typename I>
Enumerated(I, T)->Enumerated<T, I>;

enum class YieldError : uint8_t { MemoryFull };
enum class StreamError : uint8_t { Pending, Closed };

template <uint64_t N>
struct SpanLock {
  STX_MAKE_PINNED(SpanLock)

  static constexpr BUFFER_LENGTH = N < 64 ? 1 : ((N + 63) / 64);

  SpinLock lock;
  // fill all with zero, meaning none of the associated operations is in flight
  uint64_t buffer[BUFFER_LENGTH]{};

  void lock_all() {
    for (uint64_t accessed_index = 0; accessed_index < N; accessed_index++) {
      uint64_t pack_pos = accessed_index / 64U;
      uint64_t pack_index = accessed_index % 64U;
      uint64_t pack_bit = static_cast<uint64_t>(1) << (pack_index);

      CRITICAL_SECTION({
        bool got_access = false;
        do {
          LockGuard guard{lock};
          got_access = (buffer[pack_pos] & pack_bit) == 0;
          if (got_access) {
            buffer[pack_pos] |= pack_bit;
          }
        } while (!got_access);

        accessed_index++;
      });
    }
  }


  void unlock_all() {
    for (uint64_t accessed_index = 0; accessed_index < N; accessed_index++) {
      uint64_t pack_pos = accessed_index / 64U;
      uint64_t pack_index = accessed_index % 64U;
      uint64_t pack_bit = static_cast<uint64_t>(1) << (pack_index);

      CRITICAL_SECTION({
        bool got_access = false;
        do {
          LockGuard guard{lock};
          got_access = (buffer[pack_pos] & pack_bit) == 0;
          if (got_access) {
            buffer[pack_pos] |= pack_bit;
          }
        } while (!got_access);

        accessed_index++;
      });
    }
  }

  template <uint64_t I>
  void guard____lock() {
    static_assert(I < N);

    constexpr uint64_t PACK_POS = I / 64U;
    constexpr uint64_t PACK_INDEX = I % 64U;
    constexpr uint64_t PACK_BIT = static_cast<uint64_t>(1) << (PACK_INDEX);

    CRITICAL_SECTION({
      bool got_access = false;

      do {
        LockGuard guard{lock};
        got_access = (buffer[PACK_POS] & PACK_BIT) == 0;
        if (got_access) {
          buffer[PACK_POS] |= PACK_BIT;
        }

      } while (!got_access);
    });
  }

  template <uint64_t I>
  void guard____unlock() {
    static_assert(I < N);

    constexpr uint64_t PACK_POS = I / 64U;
    constexpr uint64_t PACK_INDEX = I % 64;
    constexpr uint64_t PACK_BIT = static_cast<uint64_t>(1) << (PACK_INDEX);

    CRITICAL_SECTION({
      LockGuard guard{lock};
      buffer[PACK_POS] &= ~PACK_BIT;
    });
  }
};

struct DualSpinLock {
  static constexpr uint8_t FIRST = 0xb00001111;
  static constexpr uint8_t SECOND = ~FIRST;
  static constexpr uint8_t BOTH = FIRST | SECOND;
  static constexpr uint8_t NEITHER = FIRST & SECOND;

  STX_MAKE_PINNED(DualSpinLock)

  template <uint8_t Partition>
  void guard____lock_op() {
    static_assert(Partition == FIRST || Partition == SECOND ||
                  Partition == BOTH);
    bool partition_is_in_use = true;

    do {
      CRITICAL_SECTION({
        LockGuard guard{____lock};
        if ((____status & Partition) == Partition) {
          partition_is_in_use = true;
        } else {
          partition_is_in_use = false;
          ____status |= Partition;
        }
      });
    } while (partition_is_in_use);
  }

  template <uint8_t Partition>
  void guard____unlock_op() {
    static_assert(Partition == FIRST || Partition == SECOND);
    CRITICAL_SECTION({
      LockGuard guard{____lock};
      ____status &= ~Partition;
    });
  }

  void guard____lock_both() {
    bool partition_is_in_use = true;

    do {
      CRITICAL_SECTION({
        LockGuard guard{____lock};
        if (____status != 0) {
          partition_is_in_use = true;
        } else {
          partition_is_in_use = false;
          ____status = FIRST | SECOND;
        }
      });
    } while (partition_is_in_use);
  }

  void guard____unlock_both() {
    CRITICAL_SECTION({
      LockGuard guard{____lock};
      ____status = 0;
    });
  }

  SpinLock ____lock;
  uint8_t ____status = 0;
};

//
//
//
// # Design Problems
//
// - The stream's memory is never released or re-used when done with. we need a
// notion of unique streams. such that copying onto other streams will be
// explicit and once a stream chunk is processed it is released.
// - This also means we need async managing of the list, preferrably O(1) locked
// or lock-free.
//
// - We want to be able to maintain the indices of the generated data, we'll
// thus need some methods or data member book-keeping to ensure ordering of the
// streams.
//
//
//
//
//
//
//
//
//
//
//

template <typename T>
struct StreamChunk;

// how do we get memory for the stream and its containing data,
// whilst having decent perf?

// # Sharing
//
//
// ## Lifetime Management
//
// how is lifetime managed?
// the stream manages its lifetime via a ref-counted state. the chunks
// individually have different lifetimes and are also ref-counted as they will
// all be shared across executors, filtered, mapped, etc.
// the stream shares the chunks with the executors and observers.
//
//
// ## Cacheline Packing
//
// The streams are unlikely to be processed on the same thread they were
// generated from so cache locality here is probably not a high priority and we
// often allocate the chunks individually over time. though we could allocate
// them at once if the bound is known but that would give little to no benefit
// for non-sequentially processed streams.
//
// Also: the data contained in streams are typically quite heavy, i.e. vectors,
// buffers, arrays, and they will often fit a cacheline.
//
//
// # Locking
//
// The stream is lock-free but its chunks' data are locked via a spinlock since
// we intend to distribute processing across threads and we thus need sharing.
// we use a cheap and fast spinlock since the operations performed on the shared
// data are usually very short-lived compared to the rest of the pipeline,
// ideally nanoseconds. i.e. copy, move, map.
//
template <typename T>
struct StreamChunk {
  STX_MAKE_PINNED(StreamChunk)

  template <typename... Args>
  explicit StreamChunk(Manager imanager, Args &&... iargs)
      : manager{imanager}, data{std::forward<Args>(iargs)...} {}

  // used for sorting ordered and sequential streams.
  // used for getting data from the streams using indices.
  // uint64_t index;

  T data;
  Manager manager;

  // points to the next added element in the stream it belongs to (if any).
  // must always be nullptr until added into the stream.
  StreamChunk<T> *next{nullptr};
};

// a sink that schedules tasks once data from a stream is available
//
// how will the future be awaited? Stream<Map<T>>
//
//
// guaranteeing cacheline packing of streamed data will be in chunks.
// which means if many allocations happen to occur in-between the chunks, there
// will be a lot of cacheline misses when moving from chunk to chunk. but that's
// not important nor a concern since the Stream will be observed by the sink in
// non-deterministic patterns anyway (depending on the number of tasks on the
// executor and their priorities).
//
//
// # Sources and Sinks
// - Streams can get data from multiple sources and yielded-to or streamed
// across multiple threads. (multi-source multi-sink)
// - chunks enter the stream in the order they were inserted.
//
//
// # Responsibilities Delegation
//
// ## Error Handling and Interruption
//
// The generator is left to determine how to handle and report errors to the
// stream and future. i.e. if we run out of memory whilst processing a video
// stream, do we close the stream and return an error via the future or do we
// swallow the error and try again?.
//    Also, some streams have non-fatal errors that don't terminate the whole
// stream but only the individual chunks, i.e. packet processing and
// streaming, if a data packet is sent and it timed-out, it is non-fatal and
// okay to try again or ignore, report error and continue.
//    Some might even have heuristics. i.e. after 20s of packet transmission
// failure, close the stream and complete the future with an error.
//
// ## Stream Ordering across streams.
//
// i.e. if we need a stream of data and want to process them and then perform
// actions on them in the order they appeared from the root stream.
// i.e. read file in stream sequentially with the indices but spread the
// processing of the streams in any order, process each chunk and then
// re-organize them by indices into an output stream that needs to write them
// out in the order they were received. i.e. read file in stream, spread
// processing across cores, and
// then....................................................
//
//
// We use the indices of the streams. and each operation carries over the
// previous operation's indices if they are linear.
//
//
// TODO(lamarrr): reduce will try to use indices, how do we do this and remove
// the indices, do we store a tag to notify that the stream is unordered from
// the root???
//
//
//==============
// can be a single-source or multi-source stream. for a multi-source stream
// events are gotten into the stream in no specific order between different
// executors.
//================
//
// and the source streams must agree on the indexes of the streams, the
// streams indices should be unique to function with sequential processing or
// ordered streams.
//======================
//
//
// guarantees consistency from point of close
//
//
// Supporting the most parallel and distrubitive of workloads
//
//
// cancelation? doesn't need to be attended to at all or even attended to on
// time. once you request for cancelation, you don't need to wait. proceed with
// what you are doing.
//
//
//
//
// The generator is expected to coordinate itself. i.e. completing the future
// after closing the stream across threads.
//
// The generator is also expected to report errors and decide to handle, retry,
// or continue the stream.
//
//
//
// Consistency Guarantees:
// - closing of the stream is guaranteed to be consistent across streams.
// this means if one stream successfully closes the stream, more data will not
// enter the stream, therefore ensuring consistency of the chunks. i.e. chunk
// inserted whilst closing the stream will always be the last observed chunk.
//
//
// IMPORTANT:
// - we can't panic on the executor thread.
// - we need it to be lock-free so we can't ask for a vector as it requires
// locking and mutual exclusion, and even though insertion is armortized we
// can't afford the scenario where it is as expensive as O(n).
//
//
template <typename T>
struct StreamState {
  static_assert(!std::is_void_v<T>);

  STX_MAKE_PINNED(StreamState)

  StreamState() {}

  SpinLock lock;
  bool closed = false;
  StreamChunk<T> *pop_it = nullptr;
  StreamChunk<T> *yield_last = nullptr;

  // yield is O(1)
  // contention is O(1) and not proportional to the contained object nor
  // management of the chunks.
  //
  // yielding never fails.
  //
  // REQUIREMENTS:
  //
  // - chunk_handle must be initialized with a ref count of 1
  //
  // if any executor yields before the close request is serviced, they will
  // still be able to yield to the stream.
  //
  void generator____yield(StreamChunk<T> *chunk_handle, bool should_close) {
    bool was_added = false;

    CRITICAL_SECTION({
      LockGuard guard{lock};

      if (closed) {
        was_added = false;
        break;
      }

      // yield_last == nullptr ?: we haven't yielded anything yet
      // pop_it == nullptr?: popping has caught up to yielding and released all
      // the previous handles. so, no handle is valid now.
      //
      if (yield_last == nullptr || pop_it == nullptr) {
        yield_last = chunk_handle;
      } else {
        yield_last->next = chunk_handle;
        yield_last = chunk_handle;
      }

      // popping has previously caught up with yielding, we need to update the
      // popping iterator (to notify that new data has been added)
      if (pop_it == nullptr) {
        pop_it = yield_last;
      }

      closed = should_close;
      was_added = true;
    });

    if (!was_added) {
      chunk_handle->manager.unref();
    }
  }

  void generator____close() {
    CRITICAL_SECTION({
      LockGuard guard{lock};
      closed = true;
    });
  }

  // NOTE that it might still have items in the stream
  bool stream____is_closed() {
    CRITICAL_SECTION({
      LockGuard guard{lock};
      return closed;
    });
  }

  // yield is O(1)
  // contention is O(1) and not proportional to the contained object nor
  // management of the chunks.
  //
  Result<T, StreamError> stream____pop() {
    StreamChunk<T> *chunk = nullptr;

    CRITICAL_SECTION({
      LockGuard guard{lock};

      if (pop_it == nullptr) break;

      chunk = pop_it;

      pop_it = pop_it->next;
    });

    if (chunk == nullptr) {
      if (closed) {
        return Err(StreamError::Closed);
      } else {
        return Err(StreamError::Pending);
      }
    } else {
      T item{std::move(chunk->data)};

      // release the chunk
      chunk->manager.unref();

      return Ok(std::move(item));
    }
  }

 private:
  // we unref the entries in a bottom-up order (inwards-outwards).
  // NOTE: we don't begin unref-ing the entries until we reach the end of
  // the chunks. since the the top-most element refers to the next one,
  // otherwise we'd risk a use-after-unref (use-after-free).
  //
  void unref_pass(StreamChunk<T> *chunk_handle) const {
    if (chunk_handle != nullptr) {
      unref_pass(chunk_handle->next);

      // release the chunk
      chunk_handle->manager.unref();
    }
  }

  void unref_items() const { unref_pass(pop_it); }

 public:
  // guaranteed to not happen along or before the operations possible on the
  // streams.
  ~StreamState() { unref_items(); }
};

static constexpr uint64_t get_next_ring_release_pos(uint64_t N,
                                                    uint64_t next_chunk_index,
                                                    uint64_t num_in_use) {
  return next_chunk_index >= num_in_use ? (next_chunk_index - num_in_use)
                                        : ((N - next_chunk_index) - num_in_use);
}

// essentially a ring-buffer-memory for the stream.
//
// deallocation needs to happen on another thread
//
// belongs to a single generator.
//
//
// NOTE: streams can use fixed-size ring buffers because they are popped in the
// order they were added (FIFO).
// This is the primary contract that allows this optimization.
//
//
template <typename T, uint64_t N>
struct GeneratorRingMemory final : private ManagerHandle {
  static_assert(N != 0);

  STX_MAKE_PINNED(GeneratorRingMemory);

  // the dual spinlock enables us to have multiple memory allocations in-flight
  // whilst destruction is happening. this means destruction can be performed
  // without blocking allocation. construction happens independent of the
  // critical sections.

  DualSpinLock lock;
  union {
    StreamChunk<T> memory_chunks[N];
  };

  uint64_t next_chunk_index = 0;
  uint64_t num_in_use = 0;
  uint64_t num_destroyed = 0;

  GeneratorRingMemory() {}
  // doesn't destroy anything
  ~GeneratorRingMemory() {}

  // None: memory is presently not available but could be available later in
  // time once an item is popped from the associated stream.
  //
  Result<StreamChunk<T> *, AllocError> generator____allocate(T &&value) {
    void *memory = nullptr;

    CRITICAL_SECTION({
      lock.guard____lock_op<DualSpinLock::FIRST>();

      if (num_in_use == N) break;

      memory = memory_chunks + next_chunk_index;

      next_chunk_index = (next_chunk_index + 1) % N;
      num_in_use++;

      lock.guard____unlock_op<DualSpinLock::SECOND>();
    });

    if (memory == nullptr) return Err(AllocError::NoMemory);

    StreamChunk<T> *chunk =
        new (memory) StreamChunk<T>{Manager{*this}, std::move(value)};

    return Ok(static_cast<StreamChunk<T> *>(chunk));
  }

  void manager____deallocate() {
    // contention can happen if `T`'s destructor is run and it is non-trivial,
    // takes a long
    // time.
    //
    // we don't need to hold the lock whilst destroying the stream chunk.
    //
    //
    //
    //
    //
    CRITICAL_SECTION({
      StreamChunk<T> *to_destroy_chunk = nullptr;

      CRITICAL_SECTION({
        // lock memory and destruction of objects
        lock.guard____lock_both();

        if constexpr (!std::is_trivially_destructible_v<T>) {
          // ....
          //

          uint64_t destroy_pos =
              get_next_ring_release_pos(N, next_chunk_index, num_in_use);

          to_destroy_chunk =
              reinterpret_cast<StreamChunk<T> *>(&memory_chunks[destroy_pos]);

          // unlock only allocation
          lock.guard____unlock_op<DualSpinLock::FIRST>();
        } else {
          // unlock both, trivially destructible data types don't need to run
          // destructors
          num_in_use--;
          lock.guard____unlock_both();
        }
      });

      // NOTE: how we've released the lock but began destroying the element.
      // this is based on the guarantee that the oldest element in the stream
      // is always destroyed first before the others. and we use a
      // `num_in_use` counter instead of the actual position of the element to
      // be deleted.
      //
      // so if another request for deletion comes in whilst we are.
      //
      //
      // but: we can't allocate whilst this is being deleted.
      //
      if constexpr (!std::is_trivially_destructible_v<T>) {
        to_destroy_chunk->~StreamChunk<T>();
        lock.guard____unlock_op<DualSpinLock::SECOND>();
      }
    });

    CRITICAL_SECTION({
      lock.lock_op<DualSpinLock::SECOND>();
      LockGuard guard{lock};
    });

    else {
      LockGuard guard{lock};
      // if (num_in_use == 0) return;
      //   next_chunk_index - N

      StreamChunk<T> *chunk = &memory_chunks[release_pos];

      // can't release lock before calling destructor since the address is in
      // use.
      chunk->~StreamChunk<T>();
      num_in_use--;
    }
  }

 private:
  virtual void ref() override {}
  virtual void unref() override { manager____deallocate(); }
};

template <typename T>
struct Generator {
  STX_DEFAULT_MOVE(Generator)
  STX_DISABLE_COPY(Generator)

  explicit Generator(Rc<StreamState<T> *> &&istate)
      : state{std::move(istate)} {}

  Result<Void, AllocError> yield(Allocator allocator, T &&value,
                                 bool should_close = false) const {
    //
  }

  // template <size_t N>
  // Result<Void, YieldError> unsafe____yield(GeneratorMemory<T, N> const
  // &memory, T &&value,
  //                            bool should_close_ = false) const {
  // memory.handle->
  // TRY_OK(chunk,  memory.handle->try_allocate(std::move(value) ));
  // }

  void close() const { state.handle->generator____close(); }

  Generator fork() const { return Generator{state.share()}; }

  Rc<StreamState<T> *> state;
};

// packed so that the memory is not released before the generator is destroyed.
// pinned to the address since we need to access the memory for the lifetime of
// the generator.
template <typename T, uint64_t N>
struct MemoryBackedGenerator {
  STX_MAKE_PINNED(MemoryBackedGenerator)
  Generator<T> generator;
  GeneratorRingMemory<T, N> memory;
};

template <typename T>
struct Stream {
  STX_DEFAULT_MOVE(Stream)
  STX_DISABLE_COPY(Stream)

  explicit Stream(Rc<StreamState<T> *> &&istate) : state{std::move(istate)} {}

  Result<T, StreamError> pop() const;

  Stream fork() const { return Stream{state.share()}; }

  Rc<StreamState<T> *> state;
};

// map (fast)
// filter
// enumerate
// seq?
//
// map_seq (slow, needs to be processed one by one to ensure sequential
// execution across threads)
//
// problem now is that how do we know the stream is ordered or not.
//
// i.e. after a filter, it is still sequential but has omitted elements.
//
//
//
// await
//
//

//
//
// we shouldn't support filtering or reducing, the user should handle those
// manually. filtering could be potentially expensive
//
// filter (needs to return index along with data?) -> gapped (for sequential
// processing preceding this we need to interleave their processing)
//

//
// if marked as ordered-source, ordering requirements don't need to wait and
// thus process immediately
//
// if marked as unordered, stream sinks need to wait
// for all of the stream to complete????
//
//
//
// ordered and sequentially processed
// unordered and ....
//
//
//
//
// gapped tag. i.e. filter in which it has to be waited to complete in some
// cases
//
//
//
// combinations of these will consume too much memory
//

/*
enum class StreamTag : uint8_t {
  None = 0,
  Ordered = 0b001,
  Unordered = 0b010,
  Gapped = 0b100
};
*/

/* STX_DEFINE_ENUM_BIT_OPS(StreamTag) */

// or
/* struct StreamAttributes {
  enum class Ordering {} ordering;
  enum class Gapping {} gapping;
};
*/
// limitations: entries are retained even when not needed
// Stream<Stream<int>>???
// this is because of the deferred guarantee
//
//
//

}  // namespace stx
