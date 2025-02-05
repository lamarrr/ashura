/// SPDX-License-Identifier: MIT
#include "ashura/std/list.h"
#include "ashura/std/allocators.h"
#include "ashura/std/error.h"
#include "gtest/gtest.h"

TEST(ListTest, Insertion)
{
  using namespace ash;
  u8    storage[512];
  Arena arena = to_arena(storage);

  struct Node
  {
    Node *next = nullptr, *prev = nullptr;
    int   v = 0;
  };

  List<Node> l;
  Node *     x;
  Node *     y;
  CHECK(arena.nalloc(1, x),"");
  CHECK(arena.nalloc(1, y),"");

  EXPECT_EQ(l.head(), nullptr);
  l.push_front(x);
  EXPECT_NE(l.head(), nullptr);

  EXPECT_EQ(l.head(), x);
  EXPECT_EQ(l.pop_back(), x);
  EXPECT_EQ(l.pop_back(), nullptr);
  l.push_front(x);
  l.push_front(y);
  EXPECT_EQ(l.pop_back(), x);
  EXPECT_EQ(l.pop_back(), y);
  EXPECT_EQ(l.pop_back(), nullptr);
}
