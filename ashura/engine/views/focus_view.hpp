/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

namespace ui
{

// TODO: DEFAULT FOCUS VIEW
// - set the global focus rect, focus view can move there
struct FocusView : View
{
    CRect canvas_region_;

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    // TODO: properly handle fixed-centered, should they be counted as part of the
    // children?
    virtual void render(Canvas & canvas, RenderInfo const & info) override;

    virtual i32 layer(i32 allocated, Span<i32> children) override;
};

}    // namespace ui
}    // namespace ash
