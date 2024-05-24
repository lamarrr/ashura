#include "ashura/std/list.h"
#include "ashura/std/arena_allocator.h"
#include "ashura/std/error.h"
#include "gtest/gtest.h"

TEST(ListTest, Insertion)
{
  using namespace ash;
  ArenaPool pool;
  List<int> list;

  EXPECT_TRUE(list.is_empty());
  EXPECT_EQ(list.pop_back(), nullptr);
  EXPECT_EQ(list.pop_front(), nullptr);

  ListNode<int> *x;
  CHECK(pool.nalloc(1, &x));
  x->next = x;
  x->prev = x;

  list.extend_back(List{x});
  EXPECT_FALSE(list.is_empty());
  EXPECT_EQ(x, list.pop_back());
  EXPECT_TRUE(list.is_empty());

  pool.reset();
}