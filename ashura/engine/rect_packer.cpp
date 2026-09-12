/// SPDX-License-Identifier: MIT
/// Rewritten from `stb_rect_pack.h`
#pragma once

#include "ashura/engine/rect_packer.hpp"
#include "ashura/std/error.hpp"
#include "ashura/std/range.hpp"

namespace ash
{

RectPacker::RectPacker(i32x2 extent, Span<Node> nodes)
{
    i32 i         = 0;
    i32 num_nodes = (i32) nodes.size();

    for (; i < num_nodes - 1; ++i)
    {
        nodes[i].next = &nodes[i + 1];
    }

    nodes[i].next = nullptr;
    init_mode_    = Mode::InitSkyline;
    heuristic_    = Heuristic::BL_sortHeight;
    free_head_    = &nodes[0];
    active_head_  = &extra_[0];
    extent_       = extent;
    num_nodes_    = num_nodes;

    // if it's not ok to run out of memory, then quantize the widths
    // so that num_nodes is always enough nodes.
    //
    // I.e. num_nodes * align >= width
    //                  align >= width / num_nodes
    //                  align = ceil(width/num_nodes)
    align_ = (extent_.x() + num_nodes_ - 1) / num_nodes_;

    // node 0 is the full width, node 1 is the sentinel (lets us not store width
    // explicitly)
    extra_[0]      = {};
    extra_[0].next = &extra_[1];
    extra_[1].pos  = {extent.x(), (1 << 30)};
    extra_[1].next = nullptr;
}

// find minimum y position if it starts at x1
i32 RectPacker::skyline_find_min_y_(Node * first, i32 x0, i32 width, i32 * pwaste)
{
    Node * node = first;
    i32    x1   = x0 + width;
    i32    min_y, visited_width, waste_area;

    ASH_CHECK(first->pos.x() <= x0, "");

    // we ended up handling this in the caller for efficiency
    ASH_CHECK(node->next->pos.x() > x0, "", "");

    ASH_CHECK(node->pos.x() <= x0, "", "");

    min_y         = 0;
    waste_area    = 0;
    visited_width = 0;
    while (node->pos.x() < x1)
    {
        if (node->pos.y() > min_y)
        {
            // raise min_y higher.
            // we've accounted for all waste up to min_y,
            // but we'll now add more waste for everything we've visted
            waste_area += visited_width * (node->pos.y() - min_y);
            min_y = node->pos.y();
            // the first time through, visited_width might be reduced
            if (node->pos.x() < x0)
                visited_width += node->next->pos.x() - x0;
            else
                visited_width += node->next->pos.x() - node->pos.x();
        }
        else
        {
            // add waste area
            i32 under_width = node->next->pos.x() - node->pos.x();
            if (under_width + visited_width > width)
                under_width = width - visited_width;
            waste_area += under_width * (min_y - node->pos.y());
            visited_width += under_width;
        }
        node = node->next;
    }

    *pwaste = waste_area;
    return min_y;
}

RectPacker::FindResult RectPacker::skyline_find_best_pos_(i32x2 extent)
{
    // align to multiple of align
    extent.x() = (extent.x() + align_ - 1);
    extent.x() -= extent.x() % align_;
    ASH_CHECK(extent.x() % align_ == 0, "");

    // if it can't possibly fit, bail immediately
    if (extent.x() > extent_.x() || extent.y() > extent_.y())
    {
        return FindResult{};
    }

    Node *  node       = active_head_;
    Node ** prev       = &active_head_;
    Node ** best       = nullptr;
    i32     best_waste = (1 << 30);
    i32     best_x     = 0;
    i32     best_y     = (1 << 30);

    while (node->pos.x() + extent.x() <= extent_.x())
    {
        i32 y, waste;
        y = skyline_find_min_y_(node, node->pos.x(), extent.x(), &waste);
        if (heuristic_ == Heuristic::BL_sortHeight)
        {    // actually just want
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
            if (y + extent.y() <= extent_.y())
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

    best_x = (best == nullptr) ? 0 : (*best)->pos.x();

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

    Node * tail = nullptr;

    if (heuristic_ == Heuristic::BF_sortHeight)
    {
        tail = active_head_;
        node = active_head_;
        prev = &active_head_;
        // find first node that's admissible
        while (tail->pos.x() < extent.x())
            tail = tail->next;
        while (tail)
        {
            i32 xpos = tail->pos.x() - extent.x();
            i32 y, waste;
            ASH_CHECK(xpos >= 0, "");
            // find the left position that matches this
            while (node->next->pos.x() <= xpos)
            {
                prev = &node->next;
                node = node->next;
            }
            ASH_CHECK(node->next->pos.x() > xpos && node->pos.x() <= xpos, "");
            y = skyline_find_min_y_(node, xpos, extent.x(), &waste);
            if (y + extent.y() <= extent_.y())
            {
                if (y <= best_y)
                {
                    if (y < best_y || waste < best_waste ||
                        (waste == best_waste && xpos < best_x))
                    {
                        best_x = xpos;
                        ASH_CHECK(y <= best_y, "");
                        best_y     = y;
                        best_waste = waste;
                        best       = prev;
                    }
                }
            }
            tail = tail->next;
        }
    }

    return FindResult{
      .pos{best_x, best_y},
      .prev_link = best
    };
}

RectPacker::FindResult RectPacker::skyline_pack_rectangle_(i32x2 extent)
{
    // find best position according to heuristic
    auto res = skyline_find_best_pos_(extent);

    // bail if:
    //    1. it failed
    //    2. the best node doesn't fit (we don't always check this)
    //    3. we're out of memory
    if (res.prev_link == nullptr || res.pos.y() + extent.y() > extent_.y() ||
        free_head_ == nullptr)
    {
        res.prev_link = nullptr;
        return res;
    }

    // on success, create new node
    Node * node   = free_head_;
    node->pos.x() = res.pos.x();
    node->pos.y() = res.pos.y() + extent.y();

    free_head_ = node->next;

    // insert the new node into the right starting point, and
    // let 'cur' point to the remaining nodes needing to be
    // stiched back in

    Node * cur = *res.prev_link;
    if (cur->pos.x() < res.pos.x())
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
    while (cur->next && cur->next->pos.x() <= res.pos.x() + extent.x())
    {
        Node * next = cur->next;
        // move the current node to the free list
        cur->next   = free_head_;
        free_head_  = cur;
        cur         = next;
    }

    // stitch the list back in
    node->next = cur;

    if (cur->pos.x() < res.pos.x() + extent.x())
        cur->pos.x() = res.pos.x() + extent.x();

    return res;
}

bool RectPacker::pack(Span<Rect> rects)
{
    i32 num_rects = (i32) rects.size();

    // we use the 'was_packed' field internally to allow sorting/unsorting
    for (i32 i = 0; i < num_rects; ++i)
    {
        rects[i].was_packed = i != 0;
    }

    // sort according to heuristic
    sort(rects, [](Rect const & a, Rect const & b) { return a.compare(b) < 0; });

    for (i32 i = 0; i < num_rects; ++i)
    {
        if (rects[i].extent.x() == 0 || rects[i].extent.y() == 0)
        {
            // empty rect needs no space
            rects[i].pos = {};
        }
        else
        {
            auto find_result = skyline_pack_rectangle_(rects[i].extent);
            if (find_result.prev_link)
            {
                rects[i].pos = find_result.pos;
            }
            else
            {
                rects[i].pos = i32x2::splat(I32_MAX);
            }
        }
    }

    bool all_rects_packed = true;

    // set was_packed flags and all_rects_packed status
    for (i32 i = 0; i < num_rects; ++i)
    {
        rects[i].was_packed =
          !(rects[i].pos.x() == I32_MAX && rects[i].pos.y() == I32_MAX);
        all_rects_packed = all_rects_packed && rects[i].was_packed;
    }

    // return the all_rects_packed status
    return all_rects_packed;
}

}    // namespace ash
