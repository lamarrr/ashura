/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/buffer.h"

using namespace ash;

TEST(BufferTest, Basic)
{
  i32         data[4];
  Buffer<i32> buf{data};

  ASSERT_TRUE(buf.push(0));
  ASSERT_TRUE(buf.push(1));
  ASSERT_EQ(buf.size(), 2);
  ASSERT_EQ(buf[0], 0);
  ASSERT_EQ(buf[1], 1);

  ASSERT_TRUE(buf.push(2));
  ASSERT_TRUE(buf.push(3));

  ASSERT_FALSE(buf.push(4));
  ASSERT_EQ(buf.size(), 4);
  ASSERT_EQ(buf.capacity(), 4);

  buf.pop(2);
  ASSERT_EQ(buf.size(), 2);
  ASSERT_EQ(buf[0], 0);
  ASSERT_EQ(buf[1], 1);
}

TEST(RingBufferTest, Basic)
{
  i32             data[16];
  RingBuffer<i32> ring{data};

  ASSERT_EQ(ring.size(), 0);

  for (i32 i = 0; i < 16; i++)
  {
    ASSERT_TRUE(ring.try_push(i));
  }

  ASSERT_EQ(ring.size(), 16);
  ASSERT_FALSE(ring.try_push(16));

  ring.push_overrun(16);
  ASSERT_EQ(ring.size(), 16);
  ASSERT_EQ(ring.storage_[0], 16);
  

  // [ ] test pop
  // [ ] test pop_many
}
