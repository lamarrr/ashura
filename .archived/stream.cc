#include "stx/stream.h"

#include "gtest/gtest.h"
#include "stx/mem.h"
#include "stx/rc.h"

struct ObjectMock {
  ObjectMock() {
    id = make_id();
    EXPECT_GE(ref(1), 0);
  }

  ObjectMock(ObjectMock const &) {
    id = make_id();
    EXPECT_GE(ref(1), 1);
  }

  ObjectMock(ObjectMock &&other) {
    id = other.id;
    EXPECT_GE(ref(1), 1);
  }

  ObjectMock &operator=(ObjectMock const &) {
    id = make_id();
    EXPECT_GE(ref(1), 1);
    return *this;
  }

  ObjectMock &operator=(ObjectMock &&other) {
    id = other.id;
    EXPECT_GE(ref(1), 1);
    return *this;
  }

  ~ObjectMock() { EXPECT_GT(ref(-1), 0); }

  static int64_t ref(int64_t inc) {
    static int64_t refs = 0;
    int64_t old_refs = refs;
    refs += inc;
    return old_refs;
  }

  static int64_t make_id() {
    static int64_t next_id = 0;
    int64_t id = next_id;
    next_id++;
    return id;
  }

  int64_t id;
};

TEST(StreamTest, Basic) {
  using namespace stx;
  // heap allocated so we can also test for use-after-free and double-free, we
  // could instrument the code to check this but we use ASAN and UB-SAN.

  void *mem0, *mem1, *mem2, *mem3, *mem4, *mem5;
  ASSERT_EQ(os_allocator.handle->allocate(
                mem0, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);
  ASSERT_EQ(os_allocator.handle->allocate(
                mem1, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);
  ASSERT_EQ(os_allocator.handle->allocate(
                mem2, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);
  ASSERT_EQ(os_allocator.handle->allocate(
                mem3, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);
  ASSERT_EQ(os_allocator.handle->allocate(
                mem4, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);
  ASSERT_EQ(os_allocator.handle->allocate(
                mem5, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);

  using chunk_manager = RcOperation<DeallocateObject<StreamChunk<int>>>;

  auto *chunk0 = new (mem0) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem0)}, 0};
  auto *chunk1 = new (mem1) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem1)}, 1};
  auto *chunk2 = new (mem2) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem2)}, 2};
  auto *chunk3 = new (mem3) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem3)}, 3};
  auto *chunk4 = new (mem4) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem4)}, 4};
  auto *chunk5 = new (mem5) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem5)}, 5};

  {
    StreamState<int> state;

    // test that the first pop returns a pending error
    EXPECT_EQ(state.stream____pop(), Err(StreamError::Pending));

    // yield to the stream without closing it
    state.generator____yield(&chunk0->operation.object, false);

    // test that we can get the yielded value back from the stream
    EXPECT_EQ(state.stream____pop(), Ok(0));

    // test that we get a pending error when elements in the stream are
    // exhausted
    state.generator____yield(&chunk1->operation.object, false);
    state.generator____yield(&chunk2->operation.object, false);

    EXPECT_EQ(state.stream____pop(), Ok(1));
    EXPECT_EQ(state.stream____pop(), Ok(2));
    EXPECT_EQ(state.stream____pop(), Err(StreamError::Pending));
    EXPECT_EQ(state.stream____pop(), Err(StreamError::Pending));

    // test that closing immediately after yielding actually closes the stream
    state.generator____yield(&chunk3->operation.object, false);
    state.generator____yield(&chunk4->operation.object, true);
    state.generator____yield(&chunk5->operation.object, true);

    // test that the stream has been closed
    EXPECT_TRUE(state.stream____is_closed());

    // test that we can pop the remaining elements in the stream
    EXPECT_EQ(state.stream____pop(), Ok(3));
    EXPECT_EQ(state.stream____pop(), Ok(4));

    // check that we can't get any element added after the stream was closed
    EXPECT_EQ(state.stream____pop(), Err(StreamError::Closed));
    EXPECT_EQ(state.stream____pop(), Err(StreamError::Closed));
  }
}

template <typename T>
auto make_memory_backed_generator(stx::Allocator allocator, uint64_t capacity) {
  auto generator_state =
      stx::rc::make_inplace<stx::StreamState<T>>(allocator).unwrap();

  auto buffer_memory =
      stx::make_fixed_buffer_memory<stx::StreamChunk<T>>(allocator, capacity)
          .unwrap();

  auto smp_ring_buffer_manager =
      stx::make_smp_ring_buffer_manager<stx::StreamChunk<T>>(
          allocator, std::move(buffer_memory))
          .unwrap();

  return stx::MemoryBackedGenerator{
      stx::Generator<T>{std::move(generator_state)},
      std::move(smp_ring_buffer_manager)};
}

TEST(StreamRingMemoryTest, Basic) {
  using namespace stx;

  
  auto generator = make_memory_backed_generator<int>(os_allocator, 200);

  auto buffer =  generator.ring_buffer_manager.handle->buffer.memory[0];

  EXPECT_TRUE(memory->generator____allocate(0).is_ok());
  EXPECT_TRUE(memory->generator____allocate(1).is_ok());
  EXPECT_TRUE(memory->generator____allocate(2).is_ok());

  EXPECT_EQ(memory->memory_chunks[0].data, 0);
  EXPECT_EQ(memory->memory_chunks[1].data, 1);
  EXPECT_EQ(memory->memory_chunks[2].data, 2);
  EXPECT_EQ(memory->available_start, 0);
  EXPECT_EQ(memory->num_available, 0);
  EXPECT_EQ(memory->next_destruct_index, 0);

  EXPECT_TRUE(memory->generator____allocate(3).is_err());
  memory->manager____unref();

  EXPECT_TRUE(memory->generator____allocate(3).is_ok());
  EXPECT_FALSE(memory->generator____allocate(3).is_ok());

  EXPECT_EQ(memory->memory_chunks[0].data, 3);

  memory->manager____unref();
  memory->manager____unref();
  memory->manager____unref();
  delete memory;
}

TEST(StreamRingMemoryTest, ObjectMockTest) {
  using namespace stx;

  auto memory = new GeneratorRingMemory<ObjectMock, 3>;

  EXPECT_TRUE(memory->generator____allocate(ObjectMock{}).is_ok());
  EXPECT_TRUE(memory->generator____allocate(ObjectMock{}).is_ok());
  EXPECT_TRUE(memory->generator____allocate(ObjectMock{}).is_ok());

  EXPECT_EQ(memory->memory_chunks[0].data.id, 0);
  EXPECT_EQ(memory->memory_chunks[1].data.id, 1);
  EXPECT_EQ(memory->memory_chunks[2].data.id, 2);
  EXPECT_EQ(memory->available_start, 0);
  EXPECT_EQ(memory->num_available, 0);
  EXPECT_EQ(memory->next_destruct_index, 0);

  EXPECT_TRUE(memory->generator____allocate(ObjectMock{}).is_err());
  memory->manager____unref();

  EXPECT_TRUE(memory->generator____allocate(ObjectMock{}).is_ok());
  EXPECT_FALSE(memory->generator____allocate(ObjectMock{}).is_ok());

  // TEST that only the sucessfully inserted one succeeded
  EXPECT_EQ(memory->memory_chunks[0].data.id, 4);

  memory->manager____unref();
  memory->manager____unref();
  memory->manager____unref();
  delete memory;
}

TEST(MemBacked, HSJjjs) {
  using namespace stx;

  Rc state = rc::make_inplace<StreamState<int>>(stx::os_allocator).unwrap();

  Rc generator = rc::make_inplace<MemoryBackedGeneratorState<int, 2>>(
                     stx::os_allocator, Generator{std::move(state)})
                     .unwrap();

  EXPECT_TRUE(generator.handle->generator____yield(0, false).is_ok());
  EXPECT_TRUE(generator.handle->generator____yield(0, false).is_ok());
  EXPECT_TRUE(generator.handle->generator____yield(0, false).is_err());
}

TEST(NonMemBacked, Yhd) {
  using namespace stx;

  // TODO(lamarrr): i.e. optional internal state.

  Generator generator{
      rc::make_inplace<StreamState<int>>(os_allocator).unwrap()};
  Stream stream{generator.state.share()};

  for (int i = 0; i < 20; i++) {
    EXPECT_TRUE(
        generator.yield(os_allocator, static_cast<int>(i), false).is_ok());
  }

  for (int i = 0; i < 20; i++) {
    EXPECT_EQ(stream.pop(), Ok(static_cast<int>(i)));
  }

  EXPECT_EQ(stream.pop(), Err(StreamError::Pending));
}
