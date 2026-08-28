/// SPDX-License-Identifier: MIT
#include "ashura/std/list.hpp"
#include "ashura/std/allocators.hpp"
#include "ashura/std/error.hpp"
#include "gtest/gtest.h"

TEST(ListTest, Insertion)
{
    using namespace ash;
    u8     storage[512];
    IArena arena{storage};

    struct Node
    {
        Node *next = nullptr, *prev = nullptr;
        int   v = 0;
    };

    List<Node> l;
    Node *     x;
    Node *     y;
    ASH_CHECK(arena.nalloc(1, x), "");
    ASH_CHECK(arena.nalloc(1, y), "");

    EXPECT_EQ(l.head(), &l.head_);
    l.push_front(x);

    EXPECT_EQ(l.pop_back(), x);
    EXPECT_EQ(l.pop_back(), nullptr);
    l.push_front(x);
    l.push_front(y);
    EXPECT_EQ(l.pop_back(), x);
    EXPECT_EQ(l.pop_back(), y);
    EXPECT_EQ(l.pop_back(), nullptr);
}

TEST(ListTest, ReverseView)
{
    using namespace ash;
    u8     storage[512];
    IArena arena{storage};

    struct Node
    {
        Node *next = nullptr, *prev = nullptr;
        int   v = 0;
    };

    List<Node> l;
    Node *     x;
    Node *     y;
    Node *     z;
    Node *     w;
    ASH_CHECK(arena.nalloc(1, x), "");
    ASH_CHECK(arena.nalloc(1, y), "");
    ASH_CHECK(arena.nalloc(1, z), "");
    ASH_CHECK(arena.nalloc(1, w), "");

    l.push_back(x);
    {
        auto iter = l.rbegin();
        EXPECT_EQ(&*iter, x);
    }

    l.push_back(y);
    l.push_back(z);
    l.push_back(w);

    {
        auto iter = l.rbegin();
        EXPECT_EQ(&*iter, w);
        ++iter;
        EXPECT_EQ(&*iter, z);
        ++iter;
        EXPECT_EQ(&*iter, y);
        ++iter;
        EXPECT_EQ(&*iter, x);
        ++iter;
        EXPECT_EQ(iter, iter_end);
    }
}
