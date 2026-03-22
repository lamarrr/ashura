/// SPDX-License-Identifier: MIT
#include "ashura/engine/view_system.h"
#include "ashura/std/error.h"
#include "ashura/std/range.h"
#include "ashura/std/trace.h"

namespace ash
{

IViewSys::Nodes IViewSys::Nodes::create(Allocator allocator, usize initial_capacity)
{
    return IViewSys::Nodes{
      .views{Vec<ref<ui::View>>::make(initial_capacity, allocator).unwrap()},
      .depth{Vec<u16>::make(initial_capacity, allocator).unwrap()},
      .parent{Vec<u16>::make(initial_capacity, allocator).unwrap()},
      .children{Vec<Slice16>::make(initial_capacity, allocator).unwrap()}};
}

IViewSys::Props IViewSys::Props::create(Allocator allocator, usize capacity)
{
    return IViewSys::Props{
      .tab_indices{Vec<i32>::make(capacity, allocator).unwrap()},
      .viewports{Vec<u16>::make(capacity, allocator).unwrap()},
      .hidden{BitVec<u64>::make(capacity, allocator).unwrap()},
      .pointable{BitVec<u64>::make(capacity, allocator).unwrap()},
      .clickable{BitVec<u64>::make(capacity, allocator).unwrap()},
      .scrollable{BitVec<u64>::make(capacity, allocator).unwrap()},
      .draggable{BitVec<u64>::make(capacity, allocator).unwrap()},
      .droppable{BitVec<u64>::make(capacity, allocator).unwrap()},
      .focusable{BitVec<u64>::make(capacity, allocator).unwrap()},
      .input{Vec<TextInputInfo>::make(capacity, allocator).unwrap()},
      .is_viewport{BitVec<u64>::make(capacity, allocator).unwrap()},
      .extents{Vec<f32x2>::make(capacity, allocator).unwrap()},
      .centers{Vec<f32x2>::make(capacity, allocator).unwrap()},
      .viewport_extents{Vec<f32x2>::make(capacity, allocator).unwrap()},
      .viewport_centers{Vec<f32x2>::make(capacity, allocator).unwrap()},
      .viewport_zooms{Vec<f32x2>::make(capacity, allocator).unwrap()},
      .fixed{BitVec<u64>::make(capacity, allocator).unwrap()},
      .fixed_centers{Vec<f32x2>::make(capacity, allocator).unwrap()},
      .z_idx{Vec<i32>::make(capacity, allocator).unwrap()},
      .layers{Vec<i32>::make(capacity, allocator).unwrap()},
      .canvas_xfm{Vec<affinef32x3>::make(capacity, allocator).unwrap()},
      .canvas_inv_xfm{Vec<affinef32x3>::make(capacity, allocator).unwrap()},
      .canvas_centers{Vec<f32x2>::make(capacity, allocator).unwrap()},
      .canvas_extents{Vec<f32x2>::make(capacity, allocator).unwrap()},
      .clips{Vec<CRect>::make(capacity, allocator).unwrap()},
      .z_ord{Vec<u16>::make(capacity, allocator).unwrap()},
      .focus_ord{Vec<u16>::make(capacity, allocator).unwrap()},
      .focus_idx{Vec<u16>::make(capacity, allocator).unwrap()}};
}

IViewSys::Props IViewSys::Props::none()
{
    return IViewSys::Props{.tab_indices{noop_allocator},
                           .viewports{noop_allocator},
                           .hidden{noop_allocator},
                           .pointable{noop_allocator},
                           .clickable{noop_allocator},
                           .scrollable{noop_allocator},
                           .draggable{noop_allocator},
                           .droppable{noop_allocator},
                           .focusable{noop_allocator},
                           .input{noop_allocator},
                           .is_viewport{noop_allocator},
                           .extents{noop_allocator},
                           .centers{noop_allocator},
                           .viewport_extents{noop_allocator},
                           .viewport_centers{noop_allocator},
                           .viewport_zooms{noop_allocator},
                           .fixed{noop_allocator},
                           .fixed_centers{noop_allocator},
                           .z_idx{noop_allocator},
                           .layers{noop_allocator},
                           .canvas_xfm{noop_allocator},
                           .canvas_inv_xfm{noop_allocator},
                           .canvas_centers{noop_allocator},
                           .canvas_extents{noop_allocator},
                           .clips{noop_allocator},
                           .z_ord{noop_allocator},
                           .focus_ord{noop_allocator},
                           .focus_idx{noop_allocator}};
}

void IViewSys::push_view_(Tree & tree, ui::View & view, u16 depth,
                          [[maybe_unused]] u16 breadth, u16 parent)
{
    tree.nodes.views.push(view).unwrap();
    tree.nodes.depth.push(depth).unwrap();
    tree.nodes.parent.push(parent).unwrap();
    tree.nodes.children.extend_uninit(1).unwrap();
}

ui::Events IViewSys::drain_events_(Tree &, ui::View & view, u16 idx)
{
    ui::Events event;

    if (view.hot_)
    {
        hot_ids_.push(view.id(), idx).unwrap();
        view.hot_ = false;
        event_queue_.try_get(view.id()).match([&](auto & e) { event = e; });
    }

    if (view.id() == ui::ViewId::None) [[unlikely]]
    {
        // should never happen
        ASH_CHECK(next_id_ != U64_MAX, "");
        view.id_    = ui::ViewId{next_id_++};
        event.bits_ = ui::Events::Bits::Type{event.bits_ | ui::Events::Bits::Mount};
    }

    return event;
}

void IViewSys::build_children_(Tree & tree, ui::View & view, u16 idx, u16 depth,
                               u16 viewport, i32 & tab_index,
                               RequestQueue & request_queue)
{
    Slice16 children{size16(tree.nodes.views), 0};

    auto build = [&](ui::View & child) {
        push_view_(tree, child, depth + 1, children.span++, idx);
    };

    // tick the view with the previous frame's context & events.
    //
    // State = F(context, events);
    //
    // This requires that the context and events be delayed for a frame.
    //
    ui::ViewState s =
      view.tick(prev_frame_scope_, drain_events_(tree, view, idx), &build);
    view.state_ = ui::ViewInternalState{
      .tab_index = static_cast<i32>(
        s.tab_index == ui::TabIndex::Auto ? tab_index : static_cast<i32>(s.tab_index)),
      .viewport    = viewport,
      .hidden      = s.hidden,
      .pointable   = s.pointable,
      .clickable   = s.clickable,
      .scrollable  = s.scrollable,
      .draggable   = s.draggable,
      .droppable   = s.droppable,
      .focusable   = s.focusable,
      .is_viewport = s.viewport,
      .input       = s.text};

    request_queue.defer_close |= s.defer_close;

    if (!s.hidden && s.focusable && s.grab_focus) [[unlikely]]
    {
        request_queue.focus =
          FocusRequest{.tgt = idx, .active = true, .grab_focus = true};
    }

    tree.nodes.children[idx] = children;

    auto children_viewport = s.viewport ? idx : viewport;

    for (auto c = children.begin(); c < children.end(); c++)
    {
        tab_index++;    // depth-first
        build_children_(tree, tree.nodes.views[c], c, depth + 1, children_viewport,
                        tab_index, request_queue);
    }
}

void IViewSys::build_(Tree & tree, RootView & root, RequestQueue & request_queue)
{
    push_view_(tree, root, 0, 0, RootView::PARENT);
    i32 tab_index = 0;
    build_children_(tree, root, 0, 0, RootView::VIEWPORT, tab_index, request_queue);
}

void IViewSys::build_states_(Tree & tree)
{
    tracing::ScopeTrace trace;

    auto n = tree.nodes.views.size();

    tree.props.tab_indices.resize_uninit(within_capacity, n).unwrap();
    tree.props.viewports.resize_uninit(within_capacity, n).unwrap();
    tree.props.hidden.resize_uninit(within_capacity, n).unwrap();
    tree.props.pointable.resize_uninit(within_capacity, n).unwrap();
    tree.props.clickable.resize_uninit(within_capacity, n).unwrap();
    tree.props.scrollable.resize_uninit(within_capacity, n).unwrap();
    tree.props.draggable.resize_uninit(within_capacity, n).unwrap();
    tree.props.droppable.resize_uninit(within_capacity, n).unwrap();
    tree.props.focusable.resize_uninit(within_capacity, n).unwrap();
    tree.props.input.resize_uninit(within_capacity, n).unwrap();
    tree.props.is_viewport.resize_uninit(within_capacity, n).unwrap();

    tree.props.extents.resize_uninit(within_capacity, n).unwrap();
    tree.props.centers.resize_uninit(within_capacity, n).unwrap();
    tree.props.viewport_extents.resize_uninit(within_capacity, n).unwrap();
    tree.props.viewport_centers.resize_uninit(within_capacity, n).unwrap();
    tree.props.viewport_zooms.resize_uninit(within_capacity, n).unwrap();

    tree.props.fixed.resize_uninit(within_capacity, n).unwrap();

    tree.props.fixed_centers.resize_uninit(within_capacity, n).unwrap();

    tree.props.z_idx.resize_uninit(within_capacity, n).unwrap();
    tree.props.layers.resize_uninit(within_capacity, n).unwrap();

    tree.props.canvas_xfm.resize_uninit(within_capacity, n).unwrap();

    tree.props.canvas_inv_xfm.resize_uninit(within_capacity, n).unwrap();
    tree.props.canvas_centers.resize_uninit(within_capacity, n).unwrap();
    tree.props.canvas_extents.resize_uninit(within_capacity, n).unwrap();
    tree.props.clips.resize_uninit(within_capacity, n).unwrap();
    tree.props.z_ord.resize_uninit(within_capacity, n).unwrap();

    tree.props.focus_ord.resize_uninit(within_capacity, n).unwrap();

    tree.props.focus_idx.resize_uninit(within_capacity, n).unwrap();

    // populate view states
    for (auto [i, view] : enumerate(tree.nodes.views.view()))
    {
        auto & s                  = view->state_;
        tree.props.tab_indices[i] = s.tab_index;
        tree.props.viewports[i]   = s.viewport;
        tree.props.hidden.set(i, s.hidden);
        tree.props.pointable.set(i, s.pointable);
        tree.props.clickable.set(i, s.clickable);
        tree.props.scrollable.set(i, s.scrollable);
        tree.props.draggable.set(i, s.draggable);
        tree.props.droppable.set(i, s.droppable);
        tree.props.focusable.set(i, s.focusable);
        tree.props.input[i] = s.input;
        tree.props.is_viewport.set(i, s.is_viewport);
    }
}

void IViewSys::focus_order_(Tree & tree)
{
    tracing::ScopeTrace trace;

    iota(tree.props.focus_ord.view(), 0U);

    indirect_sort(tree.props.focus_ord.view(), [&](auto a, auto b) {
        return tree.props.tab_indices[a] < tree.props.tab_indices[b];
    });

    for (auto [i, f] : enumerate(tree.props.focus_ord))
    {
        tree.props.focus_idx[f] = i;
    }
}

void IViewSys::layout_(Tree & tree, f32x2 viewport_extent)
{
    tracing::ScopeTrace trace;

    if (tree.nodes.views.is_empty())
    {
        return;
    }

    auto n = tree.nodes.views.size();

    // allocate sizes to children recursively
    tree.props.extents[0] = viewport_extent;

    for (auto [children, view, extent] :
         zip(tree.nodes.children, tree.nodes.views, tree.props.extents))
    {
        view->size(prev_frame_scope_, extent,
                   tree.props.extents.view().slice(children));
    }

    tree.props.centers[0] = f32x2::splat(0);

    // fit parent views along the finalized sizes of the child views and
    // assign centers to the children based on their sizes.
    for (usize i = n; i != 0;)
    {
        i--;
        auto children = tree.nodes.children[i];
        auto layout =
          tree.nodes.views[i]->fit(prev_frame_scope_, tree.props.extents[i],
                                   tree.props.extents.view().slice(children),
                                   tree.props.centers.view().slice(children));
        tree.props.extents[i]          = layout.extent;
        tree.props.viewport_extents[i] = layout.viewport_extent;
        tree.props.viewport_centers[i] = layout.viewport_center;
        tree.props.viewport_zooms[i]   = layout.viewport_zoom;
        tree.props.fixed.set(i, layout.fixed_center.is_some());
        tree.props.fixed_centers[i] = layout.fixed_center.unwrap_or();
    }

    // calculate fixed centers; parent-space to local viewport space

    for (auto [i, children] : enumerate(tree.nodes.children))
    {
        // viewports don't propagate fixed-position centers to children
        auto fc =
          tree.props.is_viewport[i] ? f32x2::zero() : tree.props.fixed_centers[i];

        for (auto c = children.begin(); c < children.end(); c++)
        {
            if (!tree.props.fixed[c]) [[likely]]
            {
                tree.props.fixed_centers[c] = tree.props.centers[c] + fc;
            }
        }
    }

    // recursively apply viewport transforms to child viewports
    tree.props.canvas_xfm[0]     = affinef32x3::identity();
    tree.props.canvas_inv_xfm[0] = affinef32x3::identity();

    for (auto i : range(n))
    {
        if (tree.props.is_viewport[i]) [[unlikely]]
        {
            auto parent = tree.props.viewports[i];

            // accumulated parent transform
            auto & accum     = tree.props.canvas_xfm[parent];
            auto & inv_accum = tree.props.canvas_inv_xfm[parent];

            // transform we are applying to the viewport's contents
            auto transform = translate2d(tree.props.fixed_centers[i]) *
                             scale2d(tree.props.viewport_zooms[i]) *
                             translate2d(-tree.props.viewport_centers[i]);

            auto inv_transform = translate_scale_inv2d(transform);

            tree.props.canvas_xfm[i]     = accum * transform;
            tree.props.canvas_inv_xfm[i] = inv_accum * inv_transform;
        }
    }

    tree.props.canvas_centers[0] = tree.props.fixed_centers[0];
    tree.props.canvas_extents[0] = tree.props.extents[0];

    for (usize i : range(1uz, n))
    {
        auto & transform = tree.props.canvas_xfm[tree.props.viewports[i]];
        auto   zoom      = f32x2{transform[0][0], transform[1][1]};
        tree.props.canvas_centers[i] =
          ash::transform(transform, tree.props.fixed_centers[i]);
        tree.props.canvas_extents[i] = tree.props.extents[i] * zoom;
    }

    tree.props.clips[0] = CRect{.center = {}, .extent = viewport_extent};

    /// clip viewports recursively and assign viewport clips to contained views
    for (auto i : range(n))
    {
        auto parent_viewport = tree.props.viewports[i];
        if (tree.props.is_viewport[i]) [[unlikely]]
        {
            CRect clip{.center = tree.props.canvas_centers[i],
                       .extent = tree.props.canvas_extents[i]};
            tree.props.clips[i] = clip.intersection(tree.props.clips[parent_viewport]);
        }
        else
        {
            tree.props.clips[i] = tree.props.clips[parent_viewport];
        }
    }
}

/// @brief Compares the z-order of `a` and `b`
static constexpr Order z_cmp(i32 a_layer, i32 a_z_index, u16 a_depth, i32 b_layer,
                             i32 b_z_index, u16 b_depth)
{
    // cmp stacking context/layer first
    auto ord = cmp(a_layer, b_layer);

    if (ord != Order::Equal)
    {
        return ord;
    }

    // cmp z_index
    ord = cmp(a_z_index, b_z_index);

    if (ord != Order::Equal)
    {
        return ord;
    }

    // cmp depth in the view tree
    return cmp(a_depth, b_depth);
}

void IViewSys::stack_(Tree & tree)
{
    tracing::ScopeTrace trace;

    if (tree.nodes.views.is_empty())
    {
        return;
    }

    tree.props.z_idx[0] = 0;

    for (auto [children, z_index, view] :
         zip(tree.nodes.children, tree.props.z_idx, tree.nodes.views))
    {
        z_index = view->z_index(prev_frame_scope_, z_index,
                                tree.props.z_idx.view().slice(children));
    }

    tree.props.layers[0] = 0;

    for (auto [children, layer, view] :
         zip(tree.nodes.children, tree.props.layers, tree.nodes.views))
    {
        layer = view->layer(prev_frame_scope_, layer,
                            tree.props.layers.view().slice(children));
    }

    iota(tree.props.z_ord.view(), 0U);

    // sort layers
    indirect_sort(tree.props.z_ord.view(), [&](auto a, auto b) {
        return z_cmp(tree.props.layers[a], tree.props.z_idx[a], tree.nodes.depth[a],
                     tree.props.layers[b], tree.props.z_idx[b],
                     tree.nodes.depth[b]) == Order::Less;
    });
}

void IViewSys::visibility_(Tree & tree)
{
    tracing::ScopeTrace trace;

    for (auto [i, children] : enumerate(tree.nodes.children))
    {
        if (tree.props.hidden[i])
        {
            // if parent requested to be hidden, make children hidden
            for (auto c = children.begin(); c < children.end(); c++)
            {
                tree.props.hidden.set_bit(c);
            }
        }
        else
        {
            auto & clip = tree.props.clips[tree.props.viewports[i]];

            bool hidden = !clip.overlaps(CRect{.center = tree.props.canvas_centers[i],
                                               .extent = tree.props.canvas_extents[i]});

            tree.props.hidden.set(i, hidden);
        }
    }
}

void IViewSys::render_(Tree & tree, Canvas & canvas)
{
    tracing::ScopeTrace trace;

    for (auto i : tree.props.z_ord)
    {
        if (!tree.props.hidden[i])
        {
            auto   parent_viewport = tree.props.viewports[i];
            auto & clip            = tree.props.clips[parent_viewport];
            auto & xfm             = tree.props.canvas_xfm[parent_viewport];
            CRect  viewport_region{.center = tree.props.fixed_centers[i],
                                   .extent = tree.props.extents[i]};
            CRect  canvas_region{.center = tree.props.canvas_centers[i],
                                 .extent = tree.props.canvas_extents[i]};
            auto & view = tree.nodes.views[i];

            view->render(prev_frame_scope_, canvas,
                         ui::RenderInfo{.viewport_region  = viewport_region,
                                        .canvas_region    = canvas_region,
                                        .clip             = clip,
                                        .canvas_transform = xfm});
        }
    }
}

void IViewSys::dispatch_focus_(Tree & tree, FocusRequest const & request,
                               Vec<Event> & events)
{
    auto old        = hot_ids_[cross_frame_focus_state_.tgt];
    bool was_active = cross_frame_focus_state_.active;

    tree.nodes.views[old]->hot_         = true;
    tree.nodes.views[request.tgt]->hot_ = true;

    cross_frame_focus_state_ = CrossFrameFocusState{
      .active = request.active, .tgt = tree.nodes.views[request.tgt]->id()};

    if (was_active && (!request.active || old != request.tgt))
    {
        events.push(Event{.dst = old, .type = ui::Events::FocusOut}).unwrap();
    }

    if ((request.tgt != old && request.active) ||
        (request.tgt == old && !was_active && request.active))
    {
        events.push(Event{.dst = request.tgt, .type = ui::Events::FocusIn}).unwrap();
    }

    if (request.active && request.grab_focus)
    {
        auto it          = request.tgt;
        auto it_viewport = tree.props.viewports[it];

        while (true)
        {
            events
              .push(Event{
                .dst  = it_viewport,
                .type = ui::Events::Scroll,
                .scroll =
                  ui::ScrollInfo{.center = tree.props.fixed_centers[it],
                                 .zoom   = tree.props.viewport_zooms[it_viewport]}
            })
              .unwrap();

            if (it == RootView::NODE)
            {
                break;
            }

            it          = it_viewport;
            it_viewport = tree.props.viewports[it];
        };
    }
}

ui::HitInfo IViewSys::get_hit_info_(Tree & tree, u16 view, f32x2 position) const
{
    auto viewport          = tree.props.viewports[view];
    auto viewport_position = transform(tree.props.canvas_inv_xfm[viewport], position);

    CRect canvas_region{.center = tree.props.canvas_centers[view],
                        .extent = tree.props.canvas_extents[view]};

    // local position of the pointer within the view
    auto & fixed_center = tree.props.fixed_centers[view];

    return ui::HitInfo{
      .viewport_hit = viewport_position,
      .canvas_hit   = position,
      .viewport_region{.center = fixed_center, .extent = tree.props.extents[view]},
      .canvas_region    = canvas_region,
      .canvas_transform = tree.props.canvas_xfm[viewport]
    };
}

u16 IViewSys::navigate_focus_(Tree & tree, u16 from_idx, bool forward) const
{
    ASH_CHECK(from_idx < tree.nodes.views.size(), "");
    ASH_CHECK(!tree.nodes.views.is_empty(), "");

    if (tree.nodes.views.size() == 1)
    {
        return from_idx;
    }

    i64  n    = size16(tree.nodes.views);
    auto from = tree.props.focus_idx[from_idx];
    i64  f    = from;

    auto advance = [&]() {
        f += (forward ? 1 : -1);

        if (f >= n)
        {
            f = 0;
        }

        if (f < 0)
        {
            f = n - 1;
        }
    };

    advance();

    while (f != from)
    {
        auto i = tree.props.focus_ord[f];

        if (!tree.props.hidden[i] && tree.props.focusable[i])
        {
            return i;
        }

        advance();
    }

    return from_idx;
}

IViewSys::HitState IViewSys::none_seq_(Tree & tree, ui::InputScope const & input,
                                       Vec<Event> &   events,
                                       RequestQueue & request_queue)
{
    if (!input.mouse().focused())
    {
        return none;
    }

    return point_seq_(tree, input, none, events, request_queue);
}

template <typename Match>
Option<u16> bubble(IViewSys::Tree & tree, u16 from, Match && match)
{
    auto current = from;

    while (true)
    {
        if (tree.props.layers[current] != tree.props.layers[from])
        {
            return none;
        }

        if (match(current))
        {
            return current;
        }

        if (tree.props.is_viewport[current])
        {
            return none;
        }

        if (current == RootView::NODE)
        {
            return none;
        }

        current = tree.nodes.parent[current];
    }
}

Option<u16> hit_test(IViewSys::Tree & tree, f32x2 position)
{
    // find in reverse z-order
    for (auto z = tree.nodes.views.size(); z != 0;)
    {
        z--;

        auto i = tree.props.z_ord[z];

        // find first non-hidden view that overlaps the hit position
        if (!tree.props.hidden[i] && CRect{.center = tree.props.canvas_centers[i],
                                           .extent = tree.props.canvas_extents[i]}
                                       .contains(position)) [[unlikely]]
        {
            return i;
        }
    }

    return none;
}

template <typename Match>
Option<u16> bubble_hit(IViewSys::Tree & tree, f32x2 position, Match && match)
{
    return hit_test(tree, position).and_then([&](auto i) {
        return bubble(tree, i, match);
    });
}

IViewSys::HitState IViewSys::drag_start_seq_(Tree & tree, ui::InputScope const & input,
                                             Option<u16> src, Vec<Event> & events)
{
    auto diff = [&](Option<u16> tgt, Option<ui::HitInfo> hit) {
        tgt.match([&](auto i) {
            events.push(Event{.dst = i, .type = ui::Events::DragIn, .hit = hit})
              .unwrap();
            events.push(Event{.dst = i, .type = ui::Events::DragOver, .hit = hit})
              .unwrap();
        });
    };

    if (!input.mouse().focused() || input.key().held(KeyCode::Escape))
    {
        src.match([&](auto i) {
            events.push(Event{.dst = i, .type = ui::Events::DragEnd}).unwrap();
        });

        return none;
    }

    auto tgt = input.mouse().position().and_then([&](auto p) {
        return bubble_hit(tree, p, [&](auto i) { return tree.props.droppable[i]; });
    });

    if (!input.mouse().held(MouseButton::Primary))
    {
        src.match([&](auto i) {
            events
              .push(Event{.dst  = i,
                          .type = ui::Events::DragEnd,
                          .hit  = get_hit_info_(tree, i, input.mouse().position().v())})
              .unwrap();
        });

        if (tgt.is_none())
        {
            // canceled
            return none;
        }

        auto hit = get_hit_info_(tree, tgt.v(), input.mouse().position().v());

        diff(tgt, hit);

        events.push(Event{.dst = tgt.v(), .type = ui::Events::Drop, .hit = hit})
          .unwrap();

        return none;
    }

    src.match([&](auto i) {
        events
          .push(Event{.dst  = i,
                      .type = ui::Events::DragUpdate,
                      .hit  = get_hit_info_(tree, i, input.mouse().position().v())})
          .unwrap();
    });

    diff(tgt, tgt.map([&](auto i) {
        return get_hit_info_(tree, i, input.mouse().position().v());
    }));

    // change to update state
    return DragState{.seq = DragState::Update, .src = src, .tgt = tgt};
}

IViewSys::HitState IViewSys::drag_update_seq_(Tree & tree, ui::InputScope const & input,
                                              Option<u16> src, Option<u16> prev_tgt,
                                              Vec<Event> & events)
{
    auto diff = [&](Option<u16> tgt, Option<ui::HitInfo> hit) {
        tgt.match([&](auto i) {
            if (prev_tgt == i)
            {
                events.push(Event{.dst = i, .type = ui::Events::DragOver, .hit = hit})
                  .unwrap();
            }
            else
            {
                events.push(Event{.dst = i, .type = ui::Events::DragIn, .hit = hit})
                  .unwrap();
                events.push(Event{.dst = i, .type = ui::Events::DragOver, .hit = hit})
                  .unwrap();
            }
        });

        prev_tgt.match([&](auto i) {
            if (i != tgt)
            {
                events.push(Event{.dst = i, .type = ui::Events::DragOut}).unwrap();
            }
        });
    };

    if (!input.mouse().focused() || input.key().held(KeyCode::Escape))
    {
        diff(none, none);
        return none;
    }

    auto tgt = bubble_hit(tree, input.mouse().position().v(),
                          [&](auto i) { return tree.props.droppable[i]; });

    auto hit = tgt.map(
      [&](auto i) { return get_hit_info_(tree, i, input.mouse().position().v()); });

    if (!input.mouse().held(MouseButton::Primary))
    {
        diff(tgt, hit);

        src.match([&](auto i) {
            events
              .push(Event{.dst  = i,
                          .type = ui::Events::DragEnd,
                          .hit  = get_hit_info_(tree, i, input.mouse().position().v())})
              .unwrap();
        });

        tgt.match([&](auto i) {
            events.push(Event{.dst = i, .type = ui::Events::Drop, .hit = hit}).unwrap();
        });

        return none;
    }

    src.match([&](auto i) {
        events
          .push(Event{.dst  = i,
                      .type = ui::Events::DragUpdate,
                      .hit  = get_hit_info_(tree, i, input.mouse().position().v())})
          .unwrap();
    });

    diff(tgt, hit);

    return DragState{.seq = DragState::Update, .src = src, .tgt = tgt};
}

IViewSys::HitState IViewSys::point_seq_(Tree & tree, ui::InputScope const & input,
                                        Option<u16> prev_tgt, Vec<Event> & events,
                                        RequestQueue & request_queue)
{
    // TODO: handle external drop
    auto diff = [&](Option<u16> tgt, Option<ui::HitInfo> hit) {
        tgt.match([&](auto i) {
            if (i != prev_tgt)
            {
                events.push(Event{.dst = i, .type = ui::Events::PointerIn, .hit = hit})
                  .unwrap();
            }

            events.push(Event{.dst = i, .type = ui::Events::PointerOver, .hit = hit})
              .unwrap();

            if (input.mouse().any_up())
            {
                events.push(Event{.dst = i, .type = ui::Events::PointerUp, .hit = hit})
                  .unwrap();
            }
        });

        prev_tgt.match([&](auto i) {
            if (i != tgt)
            {
                events.push(Event{.dst = i, .type = ui::Events::PointerOut}).unwrap();
            }
        });
    };

    if (!input.mouse().focused())
    {
        diff(none, none);
        return none;
    }

    if (input.mouse().scrolled())
    {
        auto tgt = bubble_hit(tree, input.mouse().position().v(),
                              [&](auto i) { return tree.props.scrollable[i]; });

        auto hit = tgt.map(
          [&](auto i) { return get_hit_info_(tree, i, input.mouse().position().v()); });

        if (tgt.is_some())
        {
            auto i = tgt.v();

            diff(i, hit);
            events
              .push(Event{
                .dst  = i,
                .type = ui::Events::Scroll,
                .hit  = hit,
                .scroll =
                  ui::ScrollInfo{.center =
                                   tree.props.viewport_centers[i] +
                                   -1 * input.window().scroll_delta() *
                                     input.mouse().wheel_translation().v().to<f32>(),
                                 .zoom = tree.props.viewport_zooms[i]}
            })
              .unwrap();
            return PointState{.tgt = tgt};
        }
    }

    // TODO: pointerup

    if (input.mouse().held(MouseButton::Primary))
    {
        auto tgt = bubble_hit(tree, input.mouse().position().v(), [&](auto i) {
            return tree.props.draggable[i] || tree.props.clickable[i];
        });

        auto hit = tgt.map(
          [&](auto i) { return get_hit_info_(tree, i, input.mouse().position().v()); });

        if (tgt.is_some())
        {
            auto i         = tgt.v();
            auto draggable = tree.props.draggable[i];

            // focus_on(tree, i, false, false, events);
            request_queue.focus =
              FocusRequest{.tgt = i, .active = false, .grab_focus = false};

            diff(tgt, hit);

            if (draggable)
            {
                events.push(Event{.dst = i, .type = ui::Events::DragStart, .hit = hit})
                  .unwrap();
                events.push(Event{.dst = i, .type = ui::Events::DragUpdate, .hit = hit})
                  .unwrap();
                return DragState{.seq = DragState::Start, .src = i, .tgt = none};
            }

            if (input.mouse().any_down())
            {
                events
                  .push(Event{.dst = i, .type = ui::Events::PointerDown, .hit = hit})
                  .unwrap();
            }

            return PointState{.tgt = tgt};
        }
    }

    auto tgt = bubble_hit(tree, input.mouse().position().v(),
                          [&](auto i) { return tree.props.pointable[i]; });

    auto hit = tgt.map(
      [&](auto i) { return get_hit_info_(tree, i, input.mouse().position().v()); });
    diff(tgt, hit);

    return PointState{.tgt = tgt};
}

void IViewSys::hit_seq_(Tree & tree, ui::InputScope const & input, Vec<Event> & events,
                        RequestQueue & request_queue)
{
    tracing::ScopeTrace trace;
    // - build hitstate from the ids
    // - process event state
    // - mark eventful views as hot
    // - store the events in an event queue using idsm so they can be referenced
    // in the next frame

    auto hit_state = cross_frame_hit_state_.match(
      [](None) -> HitState { return none; },
      [&](auto & h) -> HitState {
          return DragState{.seq = h.seq,
                           .src = h.src.and_then(
                             [&](auto id) { return hot_ids_.try_get(id).unref(); }),
                           .tgt = h.tgt.and_then(
                             [&](auto id) { return hot_ids_.try_get(id).unref(); })};
      },
      [&](auto & h) -> HitState {
          return PointState{.tgt = h.tgt.and_then(
                              [&](auto id) { return hot_ids_.try_get(id).unref(); })};
      });

    hit_state = hit_state.match(
      [&](None) { return none_seq_(tree, input, events, request_queue); },
      [&](auto & h) {
          // mark previous frame's src and dst views as hot, so we can
          // dispatch pointer in/out events to them
          h.src.match([&](auto i) { tree.nodes.views[i]->hot_ = true; });
          h.tgt.match([&](auto i) { tree.nodes.views[i]->hot_ = true; });

          switch (h.seq)
          {
              case DragState::Start:
                  return drag_start_seq_(tree, input, h.src, events);
              case DragState::Update:
                  return drag_update_seq_(tree, input, h.src, h.tgt, events);
          }
      },
      [&](auto & h) {
          h.tgt.match([&](auto i) { tree.nodes.views[i]->hot_ = true; });
          return point_seq_(tree, input, h.tgt, events, request_queue);
      });

    // mark current source and dst as hot, will still receive events on this frame
    cross_frame_hit_state_ = hit_state.match(
      [](None) -> CrossFrameHitState { return none; },
      [&](DragState const & h) -> CrossFrameHitState {
          h.src.match([&](auto i) { tree.nodes.views[i]->hot_ = true; });
          h.tgt.match([&](auto i) { tree.nodes.views[i]->hot_ = true; });

          return CrossFrameDragState{
            .seq = h.seq,
            .src = h.src.map([&](auto i) { return tree.nodes.views[i]->id(); }),
            .tgt = h.tgt.map([&](auto i) { return tree.nodes.views[i]->id(); })};
      },
      [&](PointState const & h) -> CrossFrameHitState {
          h.tgt.match([&](auto i) { tree.nodes.views[i]->hot_ = true; });
          return CrossFramePointState{
            .tgt = h.tgt.map([&](auto i) { return tree.nodes.views[i]->id(); })};
      });
}

void IViewSys::focus_seq_(Tree & tree, ui::InputScope const & input,
                          Vec<Event> & events, RequestQueue & request_queue)
{
    tracing::ScopeTrace trace;
    // view might be gone when we begin this frame so we can focus on the root
    // view if it has disappeared
    auto                focus_active = cross_frame_focus_state_.active;
    auto focus_tgt = hot_ids_.try_get(cross_frame_focus_state_.tgt).unref().unwrap_or();

    tree.nodes.views[focus_tgt]->hot_ = true;

    bool tabbed = input.key().down(KeyCode::Tab);
    bool alternate =
      input.key().held(KeyCode::LeftShift) || input.key().held(KeyCode::RightShift);
    bool accepts_tab =
      tree.props.input[focus_tgt].enabled && tree.props.input[focus_tgt].tab_input;

    auto focus_action = (tabbed && !accepts_tab) ?
                          (alternate ? FocusAction::Backward : FocusAction::Forward) :
                          FocusAction::None;

    if (focus_action != FocusAction::None)
    {
        auto next =
          navigate_focus_(tree, focus_tgt, focus_action == FocusAction::Forward);

        request_queue.focus =
          FocusRequest{.tgt = next, .active = true, .grab_focus = true};
    }

    if (focus_active)
    {
        events.push(Event{.dst = focus_tgt, .type = ui::Events::FocusOver}).unwrap();

        if (input.key().any_down())
        {
            events.push(Event{.dst = focus_tgt, .type = ui::Events::KeyDown}).unwrap();
        }

        if (input.key().any_up())
        {
            events.push(Event{.dst = focus_tgt, .type = ui::Events::KeyUp}).unwrap();
        }

        if (input.key().input())
        {
            events.push(Event{.dst = focus_tgt, .type = ui::Events::TextInput})
              .unwrap();
        }
    }

    request_queue.focus.match([&](auto r) { dispatch_focus_(tree, r, events); });
}

void IViewSys::compose_event_(Tree &, ui::ViewId id, ui::Events::Type event,
                              Option<ui::HitInfo> hit, Option<ui::ScrollInfo> scroll)
{
    auto [_, v] = event_queue_.push(id, ui::Events{}, nullptr, false).v();

    v.bits_ = ui::Events::Bits::Type{v.bits_ | ui::Events::Bits::at(event)};

    if (hit)
    {
        v.hit_info_ = hit;
    }

    if (scroll)
    {
        v.scroll_info_ = scroll;
    }
}

Tuple<Option<ui::FocusRect>, Option<TextInputInfo>, Cursor>
  IViewSys::prepare_events_(Tree & tree, ui::InputScope const & input,
                            RequestQueue & request_queue, Allocator scratch_allocator)
{
    tracing::ScopeTrace trace;

    Vec<Event> events{scratch_allocator};

    hit_seq_(tree, input, events, request_queue);
    focus_seq_(tree, input, events, request_queue);

    Option<ui::FocusRect> focus_rect;
    Option<TextInputInfo> input_info;
    Cursor                cursor = Cursor::Default;

    for (auto & event : events)
    {
        auto i     = event.dst;
        ref  view  = tree.nodes.views[i];
        view->hot_ = true;
        compose_event_(tree, view->id(), event.type, event.hit, event.scroll);

        if (event.type == ui::Events::PointerOver)
        {
            Cursor c = view->cursor(
              prev_frame_scope_, tree.props.extents[i],
              ash::transform(tree.props.canvas_inv_xfm[tree.props.viewports[i]],
                             input.mouse().position().v()) -
                tree.props.fixed_centers[i]);

            cursor = c;
        }
        else if (event.type == ui::Events::FocusOver)
        {
            focus_rect = ui::FocusRect{
              .area = CRect{.center = tree.props.canvas_centers[i],
                            .extent = tree.props.canvas_extents[i]},
              .clip = tree.props.clips[tree.props.viewports[i]]
            };

            input_info = tree.props.input[i];
        }
    }

    events.clear();

    return {focus_rect, input_info, cursor};
}

ViewSysState IViewSys::tick(Engine engine, ui::InputScope const & input, Canvas canvas,
                            Fn<ui::View &(Engine, ui::Scope const &)> loop)
{
    tracing::ScopeTrace trace;
    ScratchScope        scratch{allocator_};

    auto & base      = loop(engine, prev_frame_scope_);
    root_view_.next_ = base;

    auto nodes = Nodes::create(allocator_, initial_nodes_capacity_);

    Tree tree{.nodes{std::move(nodes)}, .props{Props::none()}};

    RequestQueue request_queue;

    build_(tree, root_view_, request_queue);

    auto n = size16(tree.nodes.views);

    tree.props = Props::create(scratch, n);

    build_states_(tree);
    event_queue_.clear();
    focus_order_(tree);
    layout_(tree, input.window().extent_.to<f32>());
    stack_(tree);
    visibility_(tree);
    render_(tree, canvas);

    /// Based on the current state and the new input events, prepare the view
    /// events
    auto [focus_rect, input_info, cursor] =
      prepare_events_(tree, input, request_queue, scratch);

    /// Prepare the new system and window states for dispath on the next frame
    prev_frame_sys_state_ = input.system().copy();
    prev_frame_win_state_ = input.window().copy(allocator_).unwrap();

    bool was_closing             = prev_frame_sys_scope_.closing_;
    bool has_close_defer_request = request_queue.defer_close;

    prev_frame_sys_scope_.focus_rect_ = focus_rect;
    prev_frame_sys_scope_.cursor_     = cursor;
    prev_frame_sys_scope_.closing_ =
      prev_frame_sys_scope_.closing_ || input.window().close_requested();
    prev_frame_sys_scope_.closing_deferred_ = request_queue.defer_close;

    prev_frame_sys_scope_.frame_++;

    bool should_close = was_closing && !has_close_defer_request;

    return ViewSysState{
      .should_continue = !should_close, .cursor = cursor, .input_info = input_info};
}

}    // namespace ash
