/// SPDX-License-Identifier: MIT
#include "ashura/engine/view_system.h"
#include "ashura/std/error.h"
#include "ashura/std/range.h"
#include "ashura/std/trace.h"

namespace ash
{
namespace ui
{

void System::clear_frame()
{
  views.clear();
  nodes.depth.clear();
  nodes.parent.clear();
  nodes.children.clear();
  ids.clear();
  att.tab_idx.clear();
  att.viewports.clear();
  att.hidden.clear();
  att.pointable.clear();
  att.clickable.clear();
  att.scrollable.clear();
  att.draggable.clear();
  att.droppable.clear();
  att.focusable.clear();
  att.input.clear();
  att.is_viewport.clear();
  extents.clear();
  centers.clear();
  viewport_extents.clear();
  viewport_centers.clear();
  viewport_zooms.clear();
  fixed.clear();
  fixed_centers.clear();
  z_idx.clear();
  layers.clear();
  canvas_xfm.clear();
  canvas_inv_xfm.clear();
  canvas_centers.clear();
  canvas_extents.clear();
  clips.clear();
  z_ord.clear();
  focus_ord.clear();
  focus_idx.clear();
  closing_deferred = false;
  focus_grab_tgt   = none;
}

void System::prepare_for(u16 n)
{
  extents.resize_uninit(n).unwrap();
  centers.resize_uninit(n).unwrap();
  viewport_extents.resize_uninit(n).unwrap();
  viewport_centers.resize_uninit(n).unwrap();
  viewport_zooms.resize_uninit(n).unwrap();
  fixed.resize_uninit(n).unwrap();
  fixed_centers.resize_uninit(n).unwrap();
  z_idx.resize_uninit(n).unwrap();
  layers.resize_uninit(n).unwrap();
  canvas_xfm.resize_uninit(n).unwrap();
  canvas_inv_xfm.resize_uninit(n).unwrap();
  canvas_centers.resize_uninit(n).unwrap();
  canvas_extents.resize_uninit(n).unwrap();
  clips.resize_uninit(n).unwrap();
  z_ord.resize_uninit(n).unwrap();
  focus_ord.resize_uninit(n).unwrap();
  focus_idx.resize_uninit(n).unwrap();
}

void System::push_view(View & view, u16 depth, [[maybe_unused]] u16 breadth,
                       u16 parent)
{
  views.push(view).unwrap();
  nodes.depth.push(depth).unwrap();
  nodes.parent.push(parent).unwrap();
  nodes.children.extend_uninit(1).unwrap();
  att.tab_idx.extend_uninit(1).unwrap();
  att.viewports.extend_uninit(1).unwrap();
  att.hidden.extend_uninit(1).unwrap();
  att.pointable.extend_uninit(1).unwrap();
  att.clickable.extend_uninit(1).unwrap();
  att.scrollable.extend_uninit(1).unwrap();
  att.draggable.extend_uninit(1).unwrap();
  att.droppable.extend_uninit(1).unwrap();
  att.focusable.extend_uninit(1).unwrap();
  att.input.extend_uninit(1).unwrap();
  att.is_viewport.extend_uninit(1).unwrap();
}

Events System::drain_events(View & view, u16 idx)
{
  Events event;

  if (view.hot_)
  {
    ids.push(view.id(), idx).unwrap();
    view.hot_ = false;
    event_queue.try_get(view.id()).match([&](auto & e) { event = e; });
  }

  if (view.id() == ViewId::None) [[unlikely]]
  {
    // should never happen
    CHECK(next_id != U64_MAX, "");
    view.id_   = ViewId{next_id++};
    event.bits = Events::Bits::Type{event.bits | Events::Bits::Mount};
  }

  return event;
}

void System::build_children(Ctx const & ctx, View & view, u16 idx, u16 depth,
                            u16 viewport, i32 & tab_index)
{
  Slice16 children{size16(views), 0};

  auto build = [&](View & child) {
    push_view(child, depth + 1, children.span++, idx);
  };

  State s = view.tick(ctx, drain_events(view, idx), &build);

  att.tab_idx.set(idx, s.tab.unwrap_or(tab_index));
  att.viewports.set(idx, viewport);
  att.hidden.set(idx, s.hidden);
  att.pointable.set(idx, s.pointable);
  att.clickable.set(idx, s.clickable);
  att.scrollable.set(idx, s.scrollable);
  att.draggable.set(idx, s.draggable);
  att.droppable.set(idx, s.droppable);
  att.focusable.set(idx, s.focusable);
  att.input.set(idx, s.text);
  att.is_viewport.set(idx, s.viewport);
  closing_deferred |= s.defer_close;

  if (!s.hidden && s.focusable && s.grab_focus) [[unlikely]]
  {
    focus_grab_tgt = idx;
  }

  nodes.children[idx] = children;

  auto const children_viewport = s.viewport ? idx : viewport;

  for (auto c = children.begin(); c < children.end(); c++)
  {
    tab_index++;    // depth-first
    build_children(ctx, views[c], c, depth + 1, children_viewport, tab_index);
  }
}

void System::build(Ctx const & ctx, RootView & root)
{
  push_view(root, 0, 0, RootView::PARENT);
  i32 tab_index = 0;
  build_children(ctx, root, 0, 0, RootView::VIEWPORT, tab_index);
}

void System::focus_order()
{
  ScopeTrace trace;

  iota(focus_ord.view(), 0U);

  indirect_sort(focus_ord.view(), [&](auto a, auto b) {
    return att.tab_idx[a] < att.tab_idx[b];
  });

  for (auto [i, f] : enumerate(focus_ord))
  {
    focus_idx[f] = i;
  }
}

void System::layout(Vec2 viewport_extent)
{
  ScopeTrace trace;

  if (views.is_empty())
  {
    return;
  }

  auto const n = views.size();

  // allocate sizes to children recursively
  extents[0] = viewport_extent;

  for (auto [children, view, extent] : zip(nodes.children, views, extents))
  {
    view->size(extent, extents.view().slice(children));
  }

  centers[0] = Vec2::splat(0);

  // fit parent views along the finalized sizes of the child views and
  // assign centers to the children based on their sizes.
  for (usize i = n; i != 0;)
  {
    i--;
    auto const children = nodes.children[i];
    auto layout = views[i]->fit(extents[i], extents.view().slice(children),
                                centers.view().slice(children));
    extents[i]  = layout.extent;
    viewport_extents[i] = layout.viewport_extent;
    viewport_centers[i] = layout.viewport_center;
    viewport_zooms[i]   = layout.viewport_zoom;
    fixed.set(i, layout.fixed_center.is_some());
    fixed_centers[i] = layout.fixed_center.unwrap_or();
  }

  // calculate fixed centers; parent-space to local viewport space

  for (auto [i, children] : enumerate(nodes.children))
  {
    // viewports don't propagate fixed-position centers to children
    auto const fc = att.is_viewport[i] ? Vec2::ZERO : fixed_centers[i];

    for (auto c = children.begin(); c < children.end(); c++)
    {
      if (!fixed[c]) [[likely]]
      {
        fixed_centers[c] = centers[c] + fc;
      }
    }
  }

  // recursively apply viewport transforms to child viewports
  canvas_xfm[0]     = Affine3::IDENTITY;
  canvas_inv_xfm[0] = Affine3::IDENTITY;

  for (usize i = 0; i < n; i++)
  {
    if (att.is_viewport[i]) [[unlikely]]
    {
      auto parent = att.viewports[i];

      // accumulated parent transform
      auto const & accum     = canvas_xfm[parent];
      auto const & inv_accum = canvas_inv_xfm[parent];

      // transform we are applying to the viewport's contents
      auto const transform = translate2d(fixed_centers[i]) *
                             scale2d(viewport_zooms[i]) *
                             translate2d(-viewport_centers[i]);

      auto const inv_transform = translate_scale_inv2d(transform);

      canvas_xfm[i]     = accum * transform;
      canvas_inv_xfm[i] = inv_accum * inv_transform;
    }
  }

  canvas_centers[0] = fixed_centers[0];
  canvas_extents[0] = extents[0];

  for (usize i = 1; i < n; i++)
  {
    auto const & transform = canvas_xfm[att.viewports[i]];
    auto const   zoom      = Vec2{transform[0][0], transform[1][1]};
    canvas_centers[i]      = ash::transform(transform, fixed_centers[i]);
    canvas_extents[i]      = extents[i] * zoom;
  }

  clips[0] = CRect{.center = {}, .extent = viewport_extent};

  /// clip viewports recursively and assign viewport clips to contained views
  for (usize i = 0; i < n; i++)
  {
    auto const parent_viewport = att.viewports[i];
    if (att.is_viewport[i]) [[unlikely]]
    {
      CRect const clip{.center = canvas_centers[i],
                       .extent = canvas_extents[i]};
      clips[i] = clip.intersect(clips[parent_viewport]);
    }
    else
    {
      clips[i] = clips[parent_viewport];
    }
  }
}

/// @brief compares the z-order of `a` and `b`
static constexpr Order z_cmp(i32 a_layer, i32 a_z_index, u16 a_depth,
                             i32 b_layer, i32 b_z_index, u16 b_depth)
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

void System::stack()
{
  ScopeTrace trace;

  if (views.is_empty())
  {
    return;
  }

  z_idx[0] = 0;

  for (auto [children, z_index, view] : zip(nodes.children, z_idx, views))
  {
    z_index = view->z_index(z_index, z_idx.view().slice(children));
  }

  layers[0] = 0;

  for (auto [children, layer, view] : zip(nodes.children, layers, views))
  {
    layer = view->layer(layer, layers.view().slice(children));
  }

  iota(z_ord.view(), 0U);

  // sort layers
  indirect_sort(z_ord.view(), [&](auto a, auto b) {
    return z_cmp(layers[a], z_idx[a], nodes.depth[a], layers[b], z_idx[b],
                 nodes.depth[b]) == Order::Less;
  });
}

void System::visibility()
{
  ScopeTrace trace;

  for (auto [i, children] : enumerate(nodes.children))
  {
    if (att.hidden[i])
    {
      // if parent requested to be hidden, make children hidden
      for (auto c = children.begin(); c < children.end(); c++)
      {
        att.hidden.set_bit(c);
      }
    }
    else
    {
      auto const & clip = clips[att.viewports[i]];

      bool const hidden = !clip.overlaps(
        CRect{.center = canvas_centers[i], .extent = canvas_extents[i]});

      att.hidden.set(i, hidden);
    }
  }
}

void System::render(Canvas & canvas)
{
  ScopeTrace trace;

  for (auto i : z_ord)
  {
    if (!att.hidden[i])
    {
      auto         parent_viewport = att.viewports[i];
      auto const & clip            = clips[parent_viewport];
      auto const & xfm             = canvas_xfm[parent_viewport];
      CRect const  viewport_region{.center = fixed_centers[i],
                                   .extent = extents[i]};
      CRect const  canvas_region{.center = canvas_centers[i],
                                 .extent = canvas_extents[i]};

      auto & view = views[i];

      view->render(canvas, RenderInfo{.viewport_region  = viewport_region,
                                      .canvas_region    = canvas_region,
                                      .clip             = clip,
                                      .canvas_transform = xfm});
    }
  }
}

void System::focus_on(u16 i, bool active, bool grab_focus)
{
  auto old        = focus_state.tgt;
  bool was_active = focus_state.active;

  views[old]->hot_ = true;
  views[i]->hot_   = true;

  focus_state = FocusState{.active = active, .tgt = i};
  xframe_focus_state =
    XFrameFocusState{.active = active, .tgt = views[i]->id()};

  focus_rect = FocusRect{};

  if (was_active && (!active || old != i))
  {
    events.push(Event{.dst = old, .type = Events::FocusOut}).unwrap();
  }

  if ((i != old && active) || (i == old && !was_active && active))
  {
    events.push(Event{.dst = i, .type = Events::FocusIn}).unwrap();

    focus_rect = FocusRect{
      .area = CRect{.center = canvas_centers[i], .extent = canvas_extents[i]},
      .clip = clips[att.viewports[i]]
    };
  }

  if (active && grab_focus)
  {
    auto it          = i;
    auto it_viewport = att.viewports[it];

    while (true)
    {
      events
        .push(Event{
          .dst    = it_viewport,
          .type   = Events::Scroll,
          .scroll = ScrollInfo{.center = fixed_centers[it],
                               .zoom   = viewport_zooms[it_viewport]}
      })
        .unwrap();

      if (it == RootView::NODE)
      {
        break;
      }

      it          = it_viewport;
      it_viewport = att.viewports[it];
    };
  }
}

Option<u16> System::hit_test(Vec2 position) const
{
  // find in reverse z-order
  for (auto z = views.size(); z != 0;)
  {
    z--;

    auto const i = z_ord[z];

    // find first non-hidden view that overlaps the hit position
    if (!att.hidden[i] &&
        CRect{.center = canvas_centers[i], .extent = canvas_extents[i]}
          .contains(position)) [[unlikely]]
    {
      return i;
    }
  }

  return none;
}

HitInfo System::get_hit_info(u16 view, Vec2 position) const
{
  auto viewport          = att.viewports[view];
  auto viewport_position = transform(canvas_inv_xfm[viewport], position);

  CRect canvas_region{.center = canvas_centers[view],
                      .extent = canvas_extents[view]};

  // local position of the pointer within the view
  auto const & fixed_center = fixed_centers[view];

  return HitInfo{
    .viewport_hit = viewport_position,
    .canvas_hit   = position,
    .viewport_region{.center = fixed_center, .extent = extents[view]},
    .canvas_region    = canvas_region,
    .canvas_transform = canvas_xfm[viewport]
  };
}

u16 System::navigate_focus(u16 from_idx, bool forward) const
{
  CHECK(from_idx < views.size(), "");
  CHECK(!views.is_empty(), "");

  if (views.size() == 1)
  {
    return from_idx;
  }

  i64 const n    = size16(views);
  auto      from = focus_idx[from_idx];
  i64       f    = from;

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
    auto const i = focus_ord[f];

    if (!att.hidden[i] && att.focusable[i])
    {
      return i;
    }

    advance();
  }

  return from_idx;
}

System::HitState System::none_seq(Ctx const & ctx)
{
  if (!ctx.mouse.focused)
  {
    return none;
  }

  return point_seq(ctx, none);
}

System::HitState System::drag_start_seq(Ctx const & ctx, Option<u16> src)
{
  auto diff = [&](Option<u16> tgt, Option<HitInfo> hit) {
    tgt.match([&](auto i) {
      events.push(Event{.dst = i, .type = Events::DragIn, .hit = hit}).unwrap();
      events.push(Event{.dst = i, .type = Events::DragOver, .hit = hit})
        .unwrap();
    });
  };

  if (!ctx.mouse.focused || ctx.key.held(KeyCode::Escape))
  {
    src.match([&](auto i) {
      events.push(Event{.dst = i, .type = Events::DragEnd}).unwrap();
    });

    return none;
  }

  auto tgt = ctx.mouse.position.and_then([&](auto p) {
    return bubble_hit(p, [&](auto i) { return att.droppable[i]; });
  });

  if (!ctx.mouse.held(MouseButton::Primary))
  {
    src.match([&](auto i) {
      events
        .push(Event{.dst  = i,
                    .type = Events::DragEnd,
                    .hit  = get_hit_info(i, ctx.mouse.position.v())})
        .unwrap();
    });

    if (tgt.is_none())
    {
      // canceled
      return none;
    }

    auto hit = get_hit_info(tgt.v(), ctx.mouse.position.v());

    diff(tgt, hit);

    events.push(Event{.dst = tgt.v(), .type = Events::Drop, .hit = hit})
      .unwrap();

    return none;
  }

  src.match([&](auto i) {
    events
      .push(Event{.dst  = i,
                  .type = Events::DragUpdate,
                  .hit  = get_hit_info(i, ctx.mouse.position.v())})
      .unwrap();
  });

  diff(tgt, tgt.map(
              [&](auto i) { return get_hit_info(i, ctx.mouse.position.v()); }));

  // change to update state
  return DragState{.seq = DragState::Update, .src = src, .tgt = tgt};
}

System::HitState System::drag_update_seq(Ctx const & ctx, Option<u16> src,
                                         Option<u16> prev_tgt)
{
  auto diff = [&](Option<u16> tgt, Option<HitInfo> hit) {
    tgt.match([&](auto i) {
      if (prev_tgt == i)
      {
        events.push(Event{.dst = i, .type = Events::DragOver, .hit = hit})
          .unwrap();
      }
      else
      {
        events.push(Event{.dst = i, .type = Events::DragIn, .hit = hit})
          .unwrap();
        events.push(Event{.dst = i, .type = Events::DragOver, .hit = hit})
          .unwrap();
      }
    });

    prev_tgt.match([&](auto i) {
      if (i != tgt)
      {
        events.push(Event{.dst = i, .type = Events::DragOut}).unwrap();
      }
    });
  };

  if (!ctx.mouse.focused || ctx.key.held(KeyCode::Escape))
  {
    diff(none, none);
    return none;
  }

  auto tgt = bubble_hit(ctx.mouse.position.v(),
                        [&](auto i) { return att.droppable[i]; });

  auto hit =
    tgt.map([&](auto i) { return get_hit_info(i, ctx.mouse.position.v()); });

  if (!ctx.mouse.held(MouseButton::Primary))
  {
    diff(tgt, hit);

    src.match([&](auto i) {
      events
        .push(Event{.dst  = i,
                    .type = Events::DragEnd,
                    .hit  = get_hit_info(i, ctx.mouse.position.v())})
        .unwrap();
    });

    tgt.match([&](auto i) {
      events.push(Event{.dst = i, .type = Events::Drop, .hit = hit}).unwrap();
    });

    return none;
  }

  src.match([&](auto i) {
    events
      .push(Event{.dst  = i,
                  .type = Events::DragUpdate,
                  .hit  = get_hit_info(i, ctx.mouse.position.v())})
      .unwrap();
  });

  diff(tgt, hit);

  return DragState{.seq = DragState::Update, .src = src, .tgt = tgt};
}

System::HitState System::point_seq(Ctx const & ctx, Option<u16> prev_tgt)
{
  // [ ] handle external drop
  auto diff = [&](Option<u16> tgt, Option<HitInfo> hit) {
    tgt.match([&](auto i) {
      if (i != prev_tgt)
      {
        events.push(Event{.dst = i, .type = Events::PointerIn, .hit = hit})
          .unwrap();
      }

      events.push(Event{.dst = i, .type = Events::PointerOver, .hit = hit})
        .unwrap();

      if (ctx.mouse.any_up)
      {
        events.push(Event{.dst = i, .type = Events::PointerUp, .hit = hit})
          .unwrap();
      }
    });

    prev_tgt.match([&](auto i) {
      if (i != tgt)
      {
        events.push(Event{.dst = i, .type = Events::PointerOut}).unwrap();
      }
    });
  };

  if (!ctx.mouse.focused)
  {
    diff(none, none);
    return none;
  }

  if (ctx.mouse.scrolled)
  {
    auto tgt = bubble_hit(ctx.mouse.position.v(),
                          [&](auto i) { return att.scrollable[i]; });

    auto hit =
      tgt.map([&](auto i) { return get_hit_info(i, ctx.mouse.position.v()); });

    if (tgt.is_some())
    {
      auto i = tgt.v();

      diff(i, hit);
      events
        .push(Event{
          .dst  = i,
          .type = Events::Scroll,
          .hit  = hit,
          .scroll =
            ScrollInfo{.center = viewport_centers[i] +
                                 -1 * scroll_delta *
                                   as_vec2(ctx.mouse.wheel_translation.v()),
                       .zoom = viewport_zooms[i]}
      })
        .unwrap();
      return PointState{.tgt = tgt};
    }
  }

  // [ ] pointerup

  if (ctx.mouse.held(MouseButton::Primary))
  {
    auto tgt = bubble_hit(ctx.mouse.position.v(), [&](auto i) {
      return att.draggable[i] || att.clickable[i];
    });

    auto hit =
      tgt.map([&](auto i) { return get_hit_info(i, ctx.mouse.position.v()); });

    if (tgt.is_some())
    {
      auto i         = tgt.v();
      auto draggable = att.draggable[i];

      focus_on(i, false, false);

      diff(tgt, hit);

      if (draggable)
      {
        events.push(Event{.dst = i, .type = Events::DragStart, .hit = hit})
          .unwrap();
        events.push(Event{.dst = i, .type = Events::DragUpdate, .hit = hit})
          .unwrap();
        return DragState{.seq = DragState::Start, .src = i, .tgt = none};
      }

      if (ctx.mouse.any_down)
      {
        events.push(Event{.dst = i, .type = Events::PointerDown, .hit = hit})
          .unwrap();
      }

      return PointState{.tgt = tgt};
    }
  }

  auto tgt = bubble_hit(ctx.mouse.position.v(),
                        [&](auto i) { return att.pointable[i]; });

  auto hit =
    tgt.map([&](auto i) { return get_hit_info(i, ctx.mouse.position.v()); });

  diff(tgt, hit);

  return PointState{.tgt = tgt};
}

void System::hit_seq(Ctx const & ctx)
{
  // build hitstate from the ids
  // process event state
  // mark eventful views as hot
  // store the events in an event queue using ids

  hit_state = xframe_hit_state.match(
    [](None) -> HitState { return none; },
    [&](auto & h) -> HitState {
      return DragState{
        .seq = h.seq,
        .src = h.src.and_then([&](auto id) { return ids.try_get(id).unref(); }),
        .tgt =
          h.tgt.and_then([&](auto id) { return ids.try_get(id).unref(); })};
    },
    [&](auto & h) -> HitState {
      return PointState{.tgt = h.tgt.and_then(
                          [&](auto id) { return ids.try_get(id).unref(); })};
    });

  hit_state =
    hit_state.match([&](None) { return none_seq(ctx); },
                    [&](auto & h) {
                      // mark previous frame's src and dst views as hot, so we can
                      // dispatch pointer in/out events to them
                      h.src.match([&](auto i) { views[i]->hot_ = true; });
                      h.tgt.match([&](auto i) { views[i]->hot_ = true; });

                      switch (h.seq)
                      {
                        case DragState::Start:
                          return drag_start_seq(ctx, h.src);
                        case DragState::Update:
                          return drag_update_seq(ctx, h.src, h.tgt);
                      }
                    },
                    [&](auto & h) {
                      h.tgt.match([&](auto i) { views[i]->hot_ = true; });
                      return point_seq(ctx, h.tgt);
                    });

  // mark current source and dst as hot, will still receive events on this frame
  xframe_hit_state = hit_state.match(
    [](None) -> XFrameHitState { return none; },
    [&](DragState const & h) -> XFrameHitState {
      h.src.match([&](auto i) { views[i]->hot_ = true; });
      h.tgt.match([&](auto i) { views[i]->hot_ = true; });

      return XFrameDragState{
        .seq = h.seq,
        .src = h.src.map([&](auto i) { return views[i]->id(); }),
        .tgt = h.tgt.map([&](auto i) { return views[i]->id(); })};
    },
    [&](PointState const & h) -> XFrameHitState {
      h.tgt.match([&](auto i) { views[i]->hot_ = true; });
      return XFramePointState{
        .tgt = h.tgt.map([&](auto i) { return views[i]->id(); })};
    });
}

void System::focus_seq(Ctx const & ctx)
{
  // view might be gone when we begin this frame so we can focus on the root view if it has disappeared
  focus_state =
    FocusState{.active = xframe_focus_state.active,
               .tgt = ids.try_get(xframe_focus_state.tgt).unref().unwrap_or()};

  views[focus_state.tgt]->hot_ = true;

  focus_grab_tgt.match([&](auto i) { focus_on(i, true, true); });

  bool tabbed = ctx.key.down(KeyCode::Tab);
  bool alternate =
    ctx.key.held(KeyCode::LeftShift) || ctx.key.held(KeyCode::RightShift);
  bool accepts_tab = att.input[focus_state.tgt].is_some() &&
                     att.input[focus_state.tgt].v().tab_input;

  auto focus_action =
    (tabbed && !accepts_tab) ?
      (alternate ? FocusAction::Backward : FocusAction::Forward) :
      FocusAction::None;

  if (focus_action != FocusAction::None)
  {
    auto next =
      navigate_focus(focus_state.tgt, focus_action == FocusAction::Forward);

    focus_on(next, true, true);
  }

  if (focus_state.active)
  {
    events.push(Event{.dst = focus_state.tgt, .type = Events::FocusOver})
      .unwrap();

    if (ctx.key.any_down)
    {
      events.push(Event{.dst = focus_state.tgt, .type = Events::KeyDown})
        .unwrap();
    }

    if (ctx.key.any_up)
    {
      events.push(Event{.dst = focus_state.tgt, .type = Events::KeyUp})
        .unwrap();
    }

    if (ctx.key.input)
    {
      events.push(Event{.dst = focus_state.tgt, .type = Events::TextInput})
        .unwrap();
    }
  }
}

void System::compose_event(ViewId id, Events::Type event, Option<HitInfo> hit,
                           Option<ScrollInfo> scroll)
{
  auto [_, v] = event_queue.insert(id, Events{}, nullptr, false).v();

  v.bits = Events::Bits::Type{v.bits | Events::Bits::at(event)};

  if (hit)
  {
    v.hit_info = hit;
  }

  if (scroll)
  {
    v.scroll_info = scroll;
  }
}

void System::process_input(Ctx const & ctx)
{
  ScopeTrace trace;

  hit_seq(ctx);
  focus_seq(ctx);

  focus_rect = none;
  input_info = none;
  cursor     = Cursor::Default;

  for (auto const & event : events)
  {
    auto i     = event.dst;
    ref  view  = views[i];
    view->hot_ = true;
    compose_event(view->id(), event.type, event.hit, event.scroll);

    if (event.type == Events::PointerOver)
    {
      Cursor c = view->cursor(extents[i],
                              ash::transform(canvas_inv_xfm[att.viewports[i]],
                                             ctx.mouse.position.v()) -
                                fixed_centers[i]);

      cursor = c;
    }
    else if (event.type == Events::FocusOver)
    {
      focus_rect = FocusRect{
        .area = CRect{.center = canvas_centers[i], .extent = canvas_extents[i]},
        .clip = clips[att.viewports[i]]
      };

      input_info = att.input[i];
    }
  }

  events.clear();
}

Option<TextInputInfo> System::text_input() const
{
  return input_info;
}

bool System::tick(InputState const & input, View & root, Canvas & canvas,
                  Fn<void(Ctx const &)> loop)
{
  ScopeTrace trace;
  // [ ] message propagation, i.e theme change

  clear_frame();

  root_view.next_ = root;

  ctx.focused = focus_rect;
  ctx.cursor  = cursor;

  loop(ctx);
  build(ctx, root_view);
  event_queue.clear();
  prepare_for(size16(views));
  focus_order();
  layout(as_vec2(input.window.extent));
  stack();
  visibility();
  render(canvas);

  ctx.tick(input);

  process_input(ctx);

  bool const should_close = ctx.closing && !closing_deferred;

  frame++;

  return !should_close;
}

}    // namespace ui
}    // namespace ash
