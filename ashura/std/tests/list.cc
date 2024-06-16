#include "ashura/std/list.h"
#include "ashura/std/arena_allocator.h"
#include "ashura/std/error.h"
#include "gtest/gtest.h"

TEST(ListTest, Insertion)
{
  using namespace ash;
  ArenaPool pool;

  List<int>      l;
  ListNode<int> *x;
  ListNode<int> *y;
  CHECK(pool.nalloc(1, &x));
  CHECK(pool.nalloc(1, &y));
  x->link();
  y->link();

  l.push_front(x);
  EXPECT_NE(l.head, nullptr);
  EXPECT_EQ(l.head, x);
  EXPECT_EQ(l.pop_back(), x);
  EXPECT_EQ(l.pop_back(), nullptr);
  l.push_front(x);
  l.push_front(y);
  EXPECT_EQ(l.pop_back(), x);
  EXPECT_EQ(l.pop_back(), y);
  EXPECT_EQ(l.pop_back(), nullptr);

  pool.reset();
}