
#include <atomic>
#include <cinttypes>
#include <utility>

#include "stx/allocator.h"
#include "stx/async.h"
#include "stx/manager.h"
#include "stx/mem.h"
#include "stx/option.h"
#include "stx/rc.h"

namespace stx {

// map
// reduce
// filter
// count
// count_if
// find
// find_if
// combinations of these will consume too much memory
//

// we either perform allocation upfront or perform it in the stream.
//
// we can't panic on the thread!!!!!!!!!!!!!!!
//
// we need it to be lock-free so we can't ask for a vector
//
//
enum class StreamError {
  // if the provided allocator runs out of memory
  NoMemory
};

template <typename T>
struct StreamChunk;

// how is lifetime managed??
//
// how do we get memory for the stream and its containing data???
// whilst being perf good
//
// stream of requests
//
template <typename T>
using StreamChunkHandle = RefCntOperation<DeallocateObject<StreamChunk<T>>>;

// needs iterator
// should we help lock the chunk???
// since it is shared by default.
// it is not really in the hot path.
//
// the code can continue reading from this chunk without communicating with the
// the stream future????
template <typename T>
struct StreamChunk {
  STX_MAKE_PINNED(StreamChunk)

  template <typename... Args>
  explicit StreamChunk(Args &&... args) : data{std::forward<Args>(args)...} {}

  T data;

  // must always be null until added into the stream
  std::atomic<StreamChunkHandle<T> *> next{nullptr};
};

template <typename T>
struct StreamData {
  Rc<StreamChunk<T> *> chunk;
};

// how do we destroy???
// require allocator???
//
// a sink that schedules tasks once data from a stream is available
//
// how will the future be awaited? Stream<Map<T>>
//
// how to prevent user from adding additional entries once marked as
// completed???
//
// guaranteeing cacheline packing of streamed data will be in chunks.
// which means if many allocations happen to occur in-between the chunks, there
// will be a lot of cacheline misses when moving from chunk to chunk. but that's
// not important since the Stream will be observed by the sink in
// non-deterministic patterns anyway (depending on the number of tasks on the
// executor and their priorities).
//
//
// multi-source multi-sink
//
//
// stream must request a thread-safe allocator for the chunks
//
//
//
//
// can be a single-source or multi-source stream. for a multi-source stream
// events are gotten into the stream in no specific order between different
// executors.
// closing of the stream is however guaranteed to be consistent across streams.
// this means if one stream successfully closes the stream, more data will not
// enter the stream, therefore ensuring consistency of the chunks.
//
// stream_to()
//
//
//
// NOTE: if a bound/estimate is provided for the number of iterations upfront,
// we can perform the allocation upfront before entering the routine. otherwise,
// we'd have to forward allocation errors. and then return the error code of the
// allocation.
//
// guarantees consistency from point of close
//
//
//
//
// What to stream? Try to make sure your stream chunks contain large amounts of
// data to get the benefits.
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
template <typename T>
struct StreamState {
  static_assert(!std::is_void_v<T>);

  STX_MAKE_PINNED(StreamState)

  StreamState() {}

  // disable move, disable copy
  //

  // if an executor wants to close off a stream and prevent further entries, the
  // yield_router pointer is adjusted to the residual_slot
  //
  std::atomic<StreamChunkHandle<T> *> yield_slot{nullptr};
  std::atomic<StreamChunkHandle<T> *> residual_slot{nullptr};
  std::atomic<std::atomic<StreamChunkHandle<T> *> *> yield_router{&yield_slot};

  // we perform a ref/unref at the end of the stream??? so ones that haven't
  // been observed can be freed??? other futures or waiters might be late to the
  // party. some might want to observe in whole.
  //
  //
  // this means once the stream is completed we increase the ref count of all
  // the handles.
  //
  // we need a ref count of one since each stream will need to ....
  //
  // initialized with a refcount of 1
  //
  //
  // how do we lock the stream when it is completed??
  // its completion
  //
  //
  // not actually to be used?
  //
  //
  //
  // can rewind
  //
  //
  //
  // a multi-observer model where each one tracks its last read position
  // should propably return a result (invalid index, )
  Option<Rc<StreamChunk<T> *>> user____poll(uint64_t index) {
    // iterate recursively until nullptr is reached and then return from there.
    //
    StreamChunkHandle<T> *iter = yield_slot.load(std::memory_order_relaxed);
    uint64_t iter_index = 0;

    while (iter != nullptr && iter_index <= index) {
      if (iter_index == index) {
        // share ownership of the stream chunk
        iter->ref();
        return Some(
            Rc<StreamChunk<T> *>{&iter->operation.object, Manager{*iter}});
      }

      iter = iter->operation.object.next.load(std::memory_order_relaxed);
      iter_index++;
    }

    return None;
  }

  // how will allocation happen? what strategy makes sense?
  // handling allocation errors?
  // we should probably forward this to the main scheduler thread where it can
  // be more easily handled
  //
  //
  // chunk_handle must be initialized with a ref count of 1!
  //
  // terminal status must be set after this.
  //
  //
  // further yields will be ignored. closing is only a hint, other executors can
  // add more chunks as long as they have the memory for it.
  //
  //
  // if any executor yields before this close request, they will still observe
  // the effects.
  //
  // but you know the observed state might become inconsistent across shared
  // streams.
  //
  // NOTE: this does not affect the state of the future.
  //
  // NOTE: close is just a hint, if another thread requests the stream to close
  // before the calling thread we do not require the close order to be ordered
  //
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
      slot = yield_router.exchange(&residual_slot, std::memory_order_relaxed);
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
    // resilient to that as all operations are atomic and our writes are atomic.
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
  }

  // we unref the entries in a bottom-up order.
  // NOTE: we don't begin unref-ing the entries until we reach the end of
  // the chunks. since the the top-most element refers to the next one,
  // otherwise we'd risk a use-after-unref (use-after-free)
  // (inwards-outwards)
  void ____unref_pass(StreamChunkHandle<T> *chunk_handle) {
    if (chunk_handle != nullptr) {
      ____unref_pass(
          chunk_handle->operation.object.next.load(std::memory_order_relaxed));
      chunk_handle->unref();
    }
  }

  void ____unref_slots() {
    ____unref_pass(yield_slot.load(std::memory_order_relaxed));
    ____unref_pass(residual_slot.load(std::memory_order_relaxed));
  }

  // guaranteed to not happen along or before the operations.
  // the user can only perform the operations whilst the refcount of this stream
  // state. its chunks refcounts are always 1 and
  ~StreamState() { ____unref_slots(); }
};

template <typename F, typename S>
struct FutureStreamState : FutureState<F>, StreamState<S> {};

template <typename F, typename S>
struct Generator {
  explicit Generator(Rc<FutureStreamState<F, S> *> &&istate)
      : state{std::move(istate)} {}

  void yield();

  // void get_promise();

  Rc<FutureStreamState<F, S> *> state;
};

template <typename F, typename S>
struct Stream {
  explicit Stream(Rc<FutureStreamState<F, S> *> &&istate)
      : state{std::move(istate)} {}

  void iter();

  void get_future();

  Rc<FutureStreamState<F, S> *> state;
};

}  // namespace stx
