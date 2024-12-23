/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/error.h"
#include "ashura/std/option.h"
#include "ashura/std/range.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

struct PackRect
{
  Vec2I pos{};

  Vec2I extent{};

  // true if packed
  bool32 packed = false;

  u32 id = 0;
};

struct RectPacker
{
  struct Node
  {
    Vec2I  pos{};
    Node * next = nullptr;
  };

  struct FindResult
  {
    Vec2I   pos{};
    Node ** prev_link = nullptr;
  };

  Vec2I extent{};

  Node * active_head = nullptr;

  Node * free_head = nullptr;

  Vec<Node> nodes{};

  /// @brief Make a rectangle packer to pack a rectangle that is 'width' by 'height' in dimensions.
  static RectPacker make(Vec2I extent, AllocatorImpl allocator)
  {
    RectPacker packer{.nodes{allocator}};

    packer.reset(extent);

    return packer;
  }

  // You must call this function every time you start packing into a new target.
  void reset(Vec2I extent)
  {
    i32 const num_nodes = extent.x + 2;

    nodes.resize(num_nodes).unwrap();

    // the full width node
    nodes[0] = Node{.pos{}, .next = &nodes[1]};

    // the sentinel node (lets us not store width explicitly)
    nodes[1] = Node{
      .pos{extent.x, (1 << 30)},
      .next = nullptr
    };

    // link the nodes
    for (i32 i = 2; i < num_nodes - 1; i++)
    {
      nodes[i].next = &nodes[i + 1];
    }

    nodes[num_nodes - 1].next = nullptr;

    this->extent      = extent;
    this->active_head = &nodes[0];
    this->free_head   = &nodes[2];
  }

  // find minimum y position if it starts at x1
  static Tuple<i32, i32> find_min_y(Node & first, i32 x0, i32 width)
  {
    CHECK(first.pos.x <= x0);

    // we ended up handling this in the caller for efficiency
    Node * iter = &first;
    CHECK(iter->next->pos.x > x0);
    CHECK(iter->pos.x <= x0);

    i32       min_y         = 0;
    i32       waste_area    = 0;
    i32       visited_width = 0;
    i32 const x1            = x0 + width;

    while (iter->pos.x < x1)
    {
      if (iter->pos.y > min_y)
      {
        // raise min_y higher. we've accounted for all waste up to min_y,
        // but we'll now add more waste for everything we've visted
        waste_area += visited_width * (iter->pos.y - min_y);
        min_y = iter->pos.y;
        // the first time through, visited_width might be reduced
        visited_width +=
          iter->next->pos.x - ((iter->pos.x < x0) ? x0 : iter->pos.x);
      }
      else
      {
        // add waste area
        i32 under_width = iter->next->pos.x - iter->pos.x;
        if (under_width + visited_width > width)
        {
          under_width = width - visited_width;
        }
        waste_area += under_width * (min_y - iter->pos.y);
        visited_width += under_width;
      }
      iter = iter->next;
    }

    return {min_y, waste_area};
  }

  Option<FindResult> find_best_pos(Vec2I size)
  {
    // if it can't possibly fit, bail immediately
    if (size.x > extent.x || size.y > extent.y)
    {
      return none;
    }

    Node *  node   = active_head;
    Node ** prev   = &active_head;
    Node ** best   = nullptr;
    i32     best_y = (1 << 30);

    while (node->pos.x + size.x <= extent.x)
    {
      auto [y, waste] = find_min_y(*node, node->pos.x, extent.x);

      // actually just want to test BL bottom left
      if (y < best_y)
      {
        best_y = y;
        best   = prev;
      }

      prev = &node->next;
      node = node->next;
    }

    if (best == nullptr)
    {
      return none;
    }

    i32 const best_x = (*best)->pos.x;

    return FindResult{
      .pos{best_x, best_y},
      .prev_link = best
    };
  }

  Option<Vec2I> pack_rect(Vec2I size)
  {
    // find best position according to heuristic
    FindResult res;

    Option r = find_best_pos(size);

    // it failed
    if (!r)
    {
      return none;
    }

    res = r.unwrap();

    // the best node doesn't fit (we don't always check this)
    if (res.pos.y + extent.y > extent.y)
    {
      return none;
    }

    // we're out of memory
    if (free_head == nullptr)
    {
      return none;
    }

    // create new node
    Node * node = free_head;

    node->pos = res.pos + Vec2I{0, extent.y};

    free_head = node->next;

    // insert the new node into the right starting point, and let 'cur' point
    // to the remaining nodes needing to be stiched back in

    Node * cur = *res.prev_link;

    if (cur->pos.x < res.pos.x)
    {
      // preserve the existing one, so start testing with the next one
      Node * next = cur->next;
      cur->next   = node;
      cur         = next;
    }
    else
    {
      *res.prev_link = node;
    }

    // from here, traverse cur and free the nodes, until we get to one
    // that shouldn't be freed
    while (cur->next && cur->next->pos.x <= res.pos.x + extent.x)
    {
      Node * next = cur->next;
      // move the current node to the free list
      cur->next   = free_head;
      free_head   = cur;
      cur         = next;
    }

    // stitch the list back in
    node->next = cur;

    if (cur->pos.x < res.pos.x + extent.x)
    {
      cur->pos.x = res.pos.x + extent.x;
    }

    return res.pos;
  }

  // Assign packed locations to rectangles.
  //
  // Rectangles which are successfully packed have the 'packed' flag
  // set to a non-zero value and 'x' and 'y' store the minimum location
  // on each axis (i.e. bottom-left in cartesian coordinates, top-left
  // if you imagine y increasing downwards). Rectangles which do not fit
  // have the 'packed' flag set to 0.
  //
  // To pack into another rectangle, you need to call `reset`.
  // To continue packing into the same rectangle, you can call
  // this function again. Calling this multiple times with multiple rect
  // arrays will probably produce worse packing results than calling it
  // a single time with the full rectangle array, but the option is
  // available.
  //
  Tuple<Span<PackRect>, Span<PackRect>> pack(Span<PackRect> rects)
  {
    for (auto & r : rects)
    {
      r.packed = false;
    }

    sort(rects, [](PackRect const & a, PackRect const & b) {
      // sort by height, then by width
      if (a.extent.y > b.extent.y)
      {
        return true;
      }
      if (a.extent.y < b.extent.y)
      {
        return false;
      }
      return a.extent.x > b.extent.x;
    });

    for (PackRect & r : rects)
    {
      if (r.extent.x == 0 | r.extent.y == 0)
      {
        // empty rect needs no space
        r.pos    = Vec2I{};
        r.packed = true;
      }
      else
      {
        pack_rect(r.extent).match(
          [&](Vec2I pos) {
            r.packed = true;
            r.pos    = pos;
          },
          [&]() {
            r.packed = false;
            r.pos    = Vec2I::splat(I32_MAX);
          });
      }
    }

    return partition(rects, [](PackRect const & r) { return r.packed; });
  }
};

}    // namespace ash
