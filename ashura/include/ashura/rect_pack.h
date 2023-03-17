// ------------------------------------------------------------------------------
// This software is available under 2 licenses -- choose whichever you prefer.
// ------------------------------------------------------------------------------
// ALTERNATIVE A - MIT License
// Copyright (c) 2017 Sean Barrett
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions: The above copyright
// notice and this permission notice shall be included in all copies or
// substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS",
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ------------------------------------------------------------------------------
// ALTERNATIVE B - Public Domain (www.unlicense.org)
// This is free and unencumbered software released into the public domain.
// Anyone is free to copy, modify, publish, use, compile, sell, or distribute
// this software, either in source code form or as a compiled binary, for any
// purpose, commercial or non-commercial, and by any means. In jurisdictions
// that recognize copyright laws, the author or authors of this software
// dedicate any and all copyright interest in the software to the public domain.
// We make this dedication for the benefit of the public at large and to the
// detriment of our heirs and successors. We intend this dedication to be an
// overt act of relinquishment in perpetuity of all present and future rights to
// this software under copyright law.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ------------------------------------------------------------------------------
//
// stb_rect_pack.h - v1.01 - public domain - rectangle packing
// Sean Barrett 2014
//
// Useful for e.g. packing rectangular textures into an atlas.
// Does not do rotation.
//
// Before #including,
//
//    #define STB_RECT_PACK_IMPLEMENTATION
//
// in the file that you want to have the implementation.
//
// Not necessarily the awesomest packing method, but better than
// the totally naive one in stb_truetype (which is primarily what
// this is meant to replace).
//
// Has only had a few tests run, may have issues.
//
// More docs to come.
//
// This library currently uses the Skyline Bottom-Left algorithm.
//
// Please note: better rectangle packers are welcome! Please
// implement them to the same API, but with a different init
// function.
//
// Credits
//
//  Library
//    Sean Barrett
//  Minor features
//    Martins Mozeiko
//    github:IntellectualKitty
//
//  Bugfixes / warning fixes
//    Jeremy Jaussaud
//    Fabian Giesen
//
// Version history:
//
//     1.01  (2021-07-11)  always use large rect mode, expose STBRP__MAXVAL in
//     public section 1.00  (2019-02-25)  avoid small space waste; gracefully
//     fail too-wide rectangles 0.99  (2019-02-07)  warning fixes 0.11
//     (2017-03-03)  return packing success/fail result 0.10  (2016-10-25)
//     remove cast-away-const to avoid warnings 0.09  (2016-08-27)  fix compiler
//     warnings 0.08  (2015-09-13)  really fix bug with empty rects (w=0 or h=0)
//     0.07  (2015-09-13)  fix bug with empty rects (w=0 or h=0)
//     0.06  (2015-04-15)  added STBRP_SORT to allow replacing qsort
//     0.05:  added STBRP_ASSERT to allow replacing assert
//     0.04:  fixed minor bug in STBRP_LARGE_RECTS support
//     0.01:  initial release
//
// LICENSE
//
//   See end of file for license information.

#pragma once

#include "ashura/primitives.h"
#include "ashura/utils.h"

namespace ash
{
namespace rp
{

struct rect
{
  u32 glyph_index = 0;

  // input:
  i32 w = 0, h = 0;

  // output:
  i32 x = 0, y = 0;

  // non-zero if valid packing
  i32 was_packed = 0;
};

constexpr int rect_height_compare(void const *a, void const *b)
{
  rect const *p = AS(rect const *, a);
  rect const *q = AS(rect const *, b);
  if (p->h > q->h)
    return -1;
  if (p->h < q->h)
    return 1;
  return (p->w > q->w) ? -1 : (p->w < q->w);
}

constexpr int rect_original_order(void const *a, void const *b)
{
  rect const *p = AS(rect const *, a);
  rect const *q = AS(rect const *, b);
  return (p->was_packed < q->was_packed) ? -1 : (p->was_packed > q->was_packed);
}

//////////////////////////////////////////////////////////////////////////////
//
// the details of the following structures don't matter to you, but they must
// be visible so you can handle the memory allocations for them

struct Node
{
  i32   x = 0, y = 0;
  Node *next = nullptr;
};

enum class Heuristic
{
  BL_sortHeight,
  BF_sortHeight
};

enum class Mode
{
  Default,
  InitSkyline
};

struct Context
{
  i32       width = 0, height = 0;
  i32       align       = 0;
  Mode      init_mode   = Mode::Default;
  Heuristic heuristic   = Heuristic::BL_sortHeight;
  i32       num_nodes   = 0;
  Node     *active_head = nullptr;
  Node     *free_head   = nullptr;
  // we allocate two extra nodes so optimal
  // user-node-count is 'width' not 'width+2'
  Node extra[2];
};

// find minimum y position if it starts at x1
inline i32 skyline_find_min_y(Context &c, Node *first, i32 x0, i32 width, i32 *pwaste)
{
  Node *node = first;
  i32   x1   = x0 + width;
  i32   min_y, visited_width, waste_area;

  (void) c;

  ASH_CHECK(first->x <= x0);

#if 0
   // skip in case we're past the node
   while (node->next->x <= x0)
      ++node;
#else
  // we ended up handling this in the caller for efficiency
  ASH_CHECK(node->next->x > x0);
#endif

  ASH_CHECK(node->x <= x0);

  min_y         = 0;
  waste_area    = 0;
  visited_width = 0;
  while (node->x < x1)
  {
    if (node->y > min_y)
    {
      // raise min_y higher.
      // we've accounted for all waste up to min_y,
      // but we'll now add more waste for everything we've visted
      waste_area += visited_width * (node->y - min_y);
      min_y = node->y;
      // the first time through, visited_width might be reduced
      if (node->x < x0)
        visited_width += node->next->x - x0;
      else
        visited_width += node->next->x - node->x;
    }
    else
    {
      // add waste area
      i32 under_width = node->next->x - node->x;
      if (under_width + visited_width > width)
        under_width = width - visited_width;
      waste_area += under_width * (min_y - node->y);
      visited_width += under_width;
    }
    node = node->next;
  }

  *pwaste = waste_area;
  return min_y;
}

struct FindResult
{
  i32    x = 0, y = 0;
  Node **prev_link = nullptr;
};

inline FindResult skyline_find_best_pos(Context &context, i32 width, i32 height)
{
  i32        best_waste = (1 << 30), best_x, best_y = (1 << 30);
  FindResult find_result;
  Node     **prev, *node, *tail, **best = nullptr;

  // align to multiple of context.align
  width = (width + context.align - 1);
  width -= width % context.align;
  ASH_CHECK(width % context.align == 0);

  // if it can't possibly fit, bail immediately
  if (width > context.width || height > context.height)
  {
    find_result.prev_link = nullptr;
    find_result.x = find_result.y = 0;
    return find_result;
  }

  node = context.active_head;
  prev = &context.active_head;
  while (node->x + width <= context.width)
  {
    i32 y, waste;
    y = skyline_find_min_y(context, node, node->x, width, &waste);
    if (context.heuristic == Heuristic::BL_sortHeight)
    {        // actually just want
             // to test BL
      // bottom left
      if (y < best_y)
      {
        best_y = y;
        best   = prev;
      }
    }
    else
    {
      // best-fit
      if (y + height <= context.height)
      {
        // can only use it if it first vertically
        if (y < best_y || (y == best_y && waste < best_waste))
        {
          best_y     = y;
          best_waste = waste;
          best       = prev;
        }
      }
    }
    prev = &node->next;
    node = node->next;
  }

  best_x = (best == nullptr) ? 0 : (*best)->x;

  // if doing best-fit (BF), we also have to try aligning right edge to each
  // node position
  //
  // e.g, if fitting
  //
  //     ____________________
  //    |____________________|
  //
  //            into
  //
  //   |                         |
  //   |             ____________|
  //   |____________|
  //
  // then right-aligned reduces waste, but bottom-left BL is always chooses
  // left-aligned
  //
  // This makes BF take about 2x the time

  if (context.heuristic == Heuristic::BF_sortHeight)
  {
    tail = context.active_head;
    node = context.active_head;
    prev = &context.active_head;
    // find first node that's admissible
    while (tail->x < width)
      tail = tail->next;
    while (tail)
    {
      i32 xpos = tail->x - width;
      i32 y, waste;
      ASH_CHECK(xpos >= 0);
      // find the left position that matches this
      while (node->next->x <= xpos)
      {
        prev = &node->next;
        node = node->next;
      }
      ASH_CHECK(node->next->x > xpos && node->x <= xpos);
      y = skyline_find_min_y(context, node, xpos, width, &waste);
      if (y + height <= context.height)
      {
        if (y <= best_y)
        {
          if (y < best_y || waste < best_waste || (waste == best_waste && xpos < best_x))
          {
            best_x = xpos;
            ASH_CHECK(y <= best_y);
            best_y     = y;
            best_waste = waste;
            best       = prev;
          }
        }
      }
      tail = tail->next;
    }
  }

  find_result.prev_link = best;
  find_result.x         = best_x;
  find_result.y         = best_y;
  return find_result;
}

inline FindResult skyline_pack_rectangle(Context &context, i32 width, i32 height)
{
  // find best position according to heuristic
  FindResult res = skyline_find_best_pos(context, width, height);
  Node      *node, *cur;

  // bail if:
  //    1. it failed
  //    2. the best node doesn't fit (we don't always check this)
  //    3. we're out of memory
  if (res.prev_link == nullptr || res.y + height > context.height || context.free_head == nullptr)
  {
    res.prev_link = nullptr;
    return res;
  }

  // on success, create new node
  node    = context.free_head;
  node->x = res.x;
  node->y = res.y + height;

  context.free_head = node->next;

  // insert the new node into the right starting point, and
  // let 'cur' point to the remaining nodes needing to be
  // stiched back in

  cur = *res.prev_link;
  if (cur->x < res.x)
  {
    // preserve the existing one, so start testing with the next one
    Node *next = cur->next;
    cur->next  = node;
    cur        = next;
  }
  else
  {
    *res.prev_link = node;
  }

  // from here, traverse cur and free the nodes, until we get to one
  // that shouldn't be freed
  while (cur->next && cur->next->x <= res.x + width)
  {
    Node *next = cur->next;
    // move the current node to the free list
    cur->next         = context.free_head;
    context.free_head = cur;
    cur               = next;
  }

  // stitch the list back in
  node->next = cur;

  if (cur->x < res.x + width)
    cur->x = res.x + width;

#ifdef _DEBUG
  cur = context.active_head;
  while (cur->x < context.width)
  {
    ASH_CHECK(cur->x < cur->next->x);
    cur = cur->next;
  }
  ASH_CHECK(cur->next == nullptr);

  {
    i32 count = 0;
    cur       = context.active_head;
    while (cur)
    {
      cur = cur->next;
      ++count;
    }
    cur = context.free_head;
    while (cur)
    {
      cur = cur->next;
      ++count;
    }
    ASH_CHECK(count == context.num_nodes + 2);
  }
#endif

  return res;
}

// Initialize a rectangle packer to:
//    pack a rectangle that is 'width' by 'height' in dimensions
//    using temporary storage provided by the array 'nodes', which is
//    'num_nodes' long
//
// You must call this function every time you start packing into a new target.
//
// There is no "shutdown" function. The 'nodes' memory must stay valid for
// the following pack_rects() call (or calls), but can be freed after
// the call (or calls) finish.
//
// Note: to guarantee best results, either:
//       1. make sure 'num_nodes' >= 'width'
//   or  2. call allow_out_of_mem() defined below with 'allow_out_of_mem =
//   1'
//
// If you don't do either of the above things, widths will be quantized to
// multiples of small integers to guarantee the algorithm doesn't run out of
// temporary storage.
//
// If you do #2, then the non-quantized algorithm will be used, but the
// algorithm may run out of temporary storage and be unable to pack some
// rectangles.
//
inline Context init(i32 width, i32 height, Node *nodes, i32 num_nodes, bool allow_out_of_mem)
{
  Context context;

  i32 i = 0;
  for (; i < num_nodes - 1; ++i)
  {
    nodes[i].next = &nodes[i + 1];
  }

  nodes[i].next       = nullptr;
  context.init_mode   = Mode::InitSkyline;
  context.heuristic   = Heuristic::BL_sortHeight;
  context.free_head   = &nodes[0];
  context.active_head = &context.extra[0];
  context.width       = width;
  context.height      = height;
  context.num_nodes   = num_nodes;

  if (allow_out_of_mem)
  {
    // if it's ok to run out of memory, then don't bother aligning them;
    // this gives better packing, but may fail due to OOM (even though
    // the rectangles easily fit). @TODO a smarter approach would be to only
    // quantize once we've hit OOM, then we could get rid of this parameter.
    context.align = 1;
  }
  else
  {
    // if it's not ok to run out of memory, then quantize the widths
    // so that num_nodes is always enough nodes.
    //
    // I.e. num_nodes * align >= width
    //                  align >= width / num_nodes
    //                  align = ceil(width/num_nodes)
    context.align = (context.width + context.num_nodes - 1) / context.num_nodes;
  }

  // node 0 is the full width, node 1 is the sentinel (lets us not store width
  // explicitly)
  context.extra[0].x    = 0;
  context.extra[0].y    = 0;
  context.extra[0].next = &context.extra[1];
  context.extra[1].x    = width;
  context.extra[1].y    = (1 << 30);
  context.extra[1].next = nullptr;

  return context;
}

// Assign packed locations to rectangles. The rectangles are of type
// 'rect' defined below, stored in the array 'rects', and there
// are 'num_rects' many of them.
//
// Rectangles which are successfully packed have the 'was_packed' flag
// set to a non-zero value and 'x' and 'y' store the minimum location
// on each axis (i.e. bottom-left in cartesian coordinates, top-left
// if you imagine y increasing downwards). Rectangles which do not fit
// have the 'was_packed' flag set to 0.
//
// You should not try to access the 'rects' array from another thread
// while this function is running, as the function temporarily reorders
// the array while it executes.
//
// To pack into another rectangle, you need to call init_target
// again. To continue packing into the same rectangle, you can call
// this function again. Calling this multiple times with multiple rect
// arrays will probably produce worse packing results than calling it
// a single time with the full rectangle array, but the option is
// available.
//
// The function returns 1 if all of the rectangles were successfully
// packed and 0 otherwise.
inline bool pack_rects(Context &context, rect *rects, i32 num_rects)
{
  i32 all_rects_packed = 1;

  // we use the 'was_packed' field internally to allow sorting/unsorting
  for (i32 i = 0; i < num_rects; ++i)
  {
    rects[i].was_packed = i;
  }

  // sort according to heuristic
  std::qsort(rects, num_rects, sizeof(rects[0]), rect_height_compare);

  for (i32 i = 0; i < num_rects; ++i)
  {
    if (rects[i].w == 0 || rects[i].h == 0)
    {
      rects[i].x = rects[i].y = 0;        // empty rect needs no space
    }
    else
    {
      FindResult find_result = skyline_pack_rectangle(context, rects[i].w, rects[i].h);
      if (find_result.prev_link)
      {
        rects[i].x = AS(i32, find_result.x);
        rects[i].y = AS(i32, find_result.y);
      }
      else
      {
        rects[i].x = rects[i].y = stx::I32_MAX;
      }
    }
  }

  // unsort
  std::qsort(rects, AS(size_t, num_rects), sizeof(rects[0]), rect_original_order);

  // set was_packed flags and all_rects_packed status
  for (i32 i = 0; i < num_rects; ++i)
  {
    rects[i].was_packed = !(rects[i].x == stx::I32_MAX && rects[i].y == stx::I32_MAX);
    if (!rects[i].was_packed)
      all_rects_packed = 0;
  }

  // return the all_rects_packed status
  return all_rects_packed;
}

}        // namespace rp
}        // namespace ash
