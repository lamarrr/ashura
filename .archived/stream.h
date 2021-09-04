
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

namespace stx {

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
using StreamChunkHandle = RefCntOperation<DeallocateObject<StreamChunk<T>>>;

// the code can continue reading from this chunk without communicating with the
// the stream future????
//
//
// NOTE: chunk can be as large as a cacheline (24 bytes minimum)
//
template <typename T>
struct StreamChunk {
  STX_MAKE_PINNED(StreamChunk)

  template <typename... Args>
  explicit StreamChunk(uint64_t iindex, Args &&... args)
      : index{iindex}, data{std::forward<Args>(args)...} {}

  // used for sorting ordered and sequential streams.
  // used for getting data from the streams using indices.
  uint64_t index;

  // must always be null until added into the stream.
  // points to the next added element in the stream it belongs to (if any).
  std::atomic<StreamChunkHandle<T> *> next{nullptr};

  T data;

  SpinLock data_lock;
};

template <typename T>
struct StreamData {
  // .copy() -> T
  // .move() -> T
  // .map() -> U
  Rc<StreamChunk<T> *> chunk;
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

  // if an executor wants to close off a stream and prevent further entries, the
  // yield_router pointer is adjusted to the residual_slot
  //
  std::atomic<StreamChunkHandle<T> *> yield_slot{nullptr};

  // this just serves as a hint to the observer, used to signify when new data
  // is added to the slots it is technically just a counter and doesn't reflect
  // the actual iteration indices of the stream.
  //
  std::atomic<uint64_t> num_chunks{0};

  std::atomic<StreamChunkHandle<T> *> residual_slot{nullptr};

  std::atomic<std::atomic<StreamChunkHandle<T> *> *> yield_router{&yield_slot};

  // a multi-observer model where each observer tracks its last read position
  //
  // This should probably not be used too often as it starts all over from the
  // beginning of the chunks.
  //
  Option<Rc<StreamChunk<T> *>> user____poll_index(uint64_t index) const {
    StreamChunkHandle<T> *iter = yield_slot.load(std::memory_order_relaxed);

    while (iter != nullptr) {
      if (index == iter->operation.object.index) {
        // share ownership of the stream chunk
        iter->ref();
        return Some(
            Rc<StreamChunk<T> *>{&iter->operation.object, Manager{*iter}});
      }

      iter = iter->operation.object.next.load(std::memory_order_relaxed);
    }

    return None;
  }

  uint64_t user____fetch_num_chunks() const {
    return num_chunks.load(std::memory_order_relaxed);
  }

  // REQUIREMENTS:
  //
  // - chunk_handle must be initialized with a ref count of 1
  // and the source streams must agree on the indexes of the streams, the
  // streams indices should be unique to function with sequential processing or
  // ordered streams.
  //
  //
  // if any executor yields before the close request is serviced, they will
  // still be able to yield to the stream.
  //
  //

  /// TODO(lamarr): is there a way we can use a single ref-count for all the
  /// chunks??

  void generator____yield(StreamChunkHandle<T> *chunk_handle, bool close) {
    // we need close to be consistent, so no other read/writes to the stream
    // will occur.
    //
    // we need a way to stop the executor from further updating the state once
    // close is requested.
    std::atomic<StreamChunkHandle<T> *> *slot = nullptr;

    // NOTE: even if another source has closed the stream, we are still routed
    // to the residual slot.
    //
    // satisfies: consistent multi-stream closing requirement
    //
    if (close) {
      slot = generator____close();
    } else {
      slot = yield_router.load(std::memory_order_relaxed);
    }

    // NOTE: even if another thread is adding to the slots whilst we are adding
    // to it, the behaviour is still sane and we add to the proper end of the
    // list.
    //
    // satisfies: sanity of the list.
    // satisfies: single-source consistency/ordering requirement.
    //
    std::atomic<StreamChunkHandle<T> *> *iter = slot;
    StreamChunkHandle<T> *expected = nullptr;
    StreamChunkHandle<T> *const target = chunk_handle;

    // NOTE: the list could be added to whilst we are modifying it, but we are
    // resilient to that as insertion is atomic and our writes are atomic.
    //
    // yielding never fails.
    //
    while (!iter->compare_exchange_strong(expected, target,
                                          std::memory_order_relaxed,
                                          std::memory_order_relaxed)) {
      // chase until we are able to add to the end of the stream.
      //
      // advance to the next chunk in the stream.
      iter = &expected->operation.object.next;
      expected = nullptr;
    }

    // notify that we have added new entries to the yield slot.
    // not notified until insertion is actually performed.
    if (slot == &yield_slot) {
      num_chunks.fetch_add(1, std::memory_order_relaxed);
    }
  }

  std::atomic<StreamChunkHandle<T> *> *generator____close() {
    return yield_router.exchange(&residual_slot, std::memory_order_relaxed);
  }

 private:
  // we unref the entries in a bottom-up order (inwards-outwards).
  // NOTE: we don't begin unref-ing the entries until we reach the end of
  // the chunks. since the the top-most element refers to the next one,
  // otherwise we'd risk a use-after-unref (use-after-free).
  //
  void unref_pass(StreamChunkHandle<T> *chunk_handle) const {
    if (chunk_handle != nullptr) {
      unref_pass(
          chunk_handle->operation.object.next.load(std::memory_order_relaxed));

      // release shared ownership
      chunk_handle->unref();
    }
  }

  void unref_slots() const {
    unref_pass(yield_slot.load(std::memory_order_relaxed));
    unref_pass(residual_slot.load(std::memory_order_relaxed));
  }

 public:
  // guaranteed to not happen along or before the operations possible on the
  // streams.
  ~StreamState() { unref_slots(); }
};

template <typename T>
struct Generator {
  explicit Generator(Rc<StreamState<T> *> &&istate)
      : state{std::move(istate)} {}

  // void yield();

  Rc<StreamState<T> *> state;
};

// this is just a simple indexed stream.
template <typename T>
struct Stream {
  explicit Stream(Rc<StreamState<T> *> &&istate) : state{std::move(istate)} {}

  // void iter();
  // poll

  Rc<StreamState<T> *> state;
};

// map (fast)
// filter
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

enum class StreamTag : uint8_t {
  None = 0,
  Ordered = 0b001,
  Unordered = 0b010,
  Gapped = 0b100
};

STX_DEFINE_ENUM_BIT_OPS(StreamTag)

// or
struct StreamAttributes {
  enum class Ordering {} ordering;
  enum class Gapping {} gapping;
};

// limitations: entries are retained even when not needed
// Stream<Stream<int>>???
// this is because of the deferred guarantee
//
//
//

}  // namespace stx
