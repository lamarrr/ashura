#include "stx/stream.h"

#include "gtest/gtest.h"
#include "stx/mem.h"
#include "stx/rc.h"

TEST(StreamTest, Basic) {
  using namespace stx;

  StreamChunkHandle<int> chunk1{1, stx::static_storage_allocator};
  StreamChunkHandle<int> chunk2{1, stx::static_storage_allocator};
  StreamChunkHandle<int> chunk3{1, stx::static_storage_allocator};
  StreamChunkHandle<int> chunk4{1, stx::static_storage_allocator};

  {
    StreamState<int> state;

    state.generator____yield(&chunk1, false);

    EXPECT_EQ(state.residual_slot.load(), nullptr);
    EXPECT_EQ(state.yield_router.load(), &state.yield_slot);
    EXPECT_EQ(state.yield_slot.load(), &chunk1);
    EXPECT_EQ(state.yield_slot.load()->operation.object.next.load(), nullptr);
    EXPECT_EQ(state.yield_slot.load()->ref_count.ref_count.load(), 1);

    state.generator____yield(&chunk2, false);

    EXPECT_EQ(state.residual_slot.load(), nullptr);
    EXPECT_EQ(state.yield_router.load(), &state.yield_slot);
    EXPECT_EQ(state.yield_slot.load(), &chunk1);
    EXPECT_EQ(state.yield_slot.load()->operation.object.next.load(), &chunk2);
    EXPECT_EQ(state.yield_slot.load()
                  ->operation.object.next.load()
                  ->operation.object.next.load(),
              nullptr);
    EXPECT_EQ(state.yield_slot.load()->ref_count.ref_count.load(), 1);
    EXPECT_EQ(state.yield_slot.load()
                  ->operation.object.next.load()
                  ->ref_count.ref_count.load(),
              1);

    state.generator____yield(&chunk3, true);

    EXPECT_EQ(state.residual_slot.load(), nullptr);
    EXPECT_EQ(state.yield_router.load(), &state.residual_slot);
    EXPECT_EQ(state.yield_slot.load(), &chunk1);
    EXPECT_EQ(state.yield_slot.load()->operation.object.next.load(), &chunk2);
    EXPECT_EQ(state.yield_slot.load()
                  ->operation.object.next.load()
                  ->operation.object.next.load(),
              &chunk3);

    state.generator____yield(&chunk4, true);
    EXPECT_EQ(state.residual_slot.load(), &chunk4);
  }
}
