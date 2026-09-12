/// SPDX-License-Identifier: MIT
/// Rewritten from `stb_rect_pack.h`
#pragma once

#include "ashura/std/math.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

struct RectPacker
{
    struct Rect
    {
        u32 id = 0;

        // input:
        i32x2 extent;

        // output:
        i32x2 pos;

        bool was_packed = false;

        constexpr i32 compare(Rect const & other) const
        {
            if (extent.y() > other.extent.y())
                return -1;
            if (extent.y() < other.extent.y())
                return 1;
            return (extent.x() > other.extent.x()) ? -1 :
                                                     (extent.x() < other.extent.x());
        }
    };

    //////////////////////////////////////////////////////////////////////////////
    //
    // the details of the following structures don't matter to you, but they must
    // be visible so you can handle the memory allocations for them

    struct Node
    {
        i32x2  pos;
        Node * next = nullptr;
    };

    enum class Heuristic : u8
    {
        BL_sortHeight = 0,
        BF_sortHeight = 1
    };

    enum class Mode : u8
    {
        Default     = 0,
        InitSkyline = 1
    };

    struct FindResult
    {
        i32x2   pos;
        Node ** prev_link = nullptr;
    };

    i32x2     extent_      = {};
    i32       align_       = 0;
    Mode      init_mode_   = Mode::Default;
    Heuristic heuristic_   = Heuristic::BL_sortHeight;
    i32       num_nodes_   = 0;
    Node *    active_head_ = nullptr;
    Node *    free_head_   = nullptr;
    // we allocate two extra nodes so optimal
    // user-node-count is 'width' not 'width+2'
    Node      extra_[2]    = {};

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
    // Note: to guarantee best results, make sure 'num_nodes' >= 'width'
    //
    // If you don't do either of the above things, widths will be quantized to
    // multiples of small integers to guarantee the algorithm doesn't run out of
    // temporary storage.
    //
    // If you do #2, then the non-quantized algorithm will be used, but the
    // algorithm may run out of temporary storage and be unable to pack some
    // rectangles.
    //
    RectPacker(i32x2 extent, Span<Node> nodes);

    RectPacker(RectPacker const &)             = delete;
    RectPacker & operator=(RectPacker const &) = delete;
    RectPacker(RectPacker &&)                  = delete;
    RectPacker & operator=(RectPacker &&)      = delete;
    ~RectPacker()                              = default;

    // find minimum y position if it starts at x1
    i32 skyline_find_min_y_(Node * first, i32 x0, i32 width, i32 * pwaste);

    FindResult skyline_find_best_pos_(i32x2 extent);

    FindResult skyline_pack_rectangle_(i32x2 extent);

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
    bool pack(Span<Rect> rects);
};

}    // namespace ash
