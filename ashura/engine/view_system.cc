/// SPDX-License-Identifier: MIT
#include "ashura/engine/view_system.h"
#include "ashura/std/error.h"
#include "ashura/std/range.h"
#include "ashura/std/trace.h"

namespace ash
{
namespace ui
{

/// @brief compares the z-order of `a` and `b`
static constexpr Order z_cmp(i32 a_layer, i32 a_z_index, u32 a_depth,
                             i32 b_layer, i32 b_z_index, u32 b_depth)
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

void System::clear_frame()
{
  views.clear();
  nodes.depth.clear();
  nodes.parent.clear();
  nodes.children.clear();
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
  canvas_tx.clear();
  canvas_inv_tx.clear();
  canvas_centers.clear();
  canvas_extents.clear();
  clips.clear();
  z_ord.clear();
  focus_ord.clear();
  focus_idx.clear();
  closing_deferred = false;
  focus_grab       = none;
  events.clear();
}

void System::prepare_for(u32 n)
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
  canvas_tx.resize_uninit(n).unwrap();
  canvas_inv_tx.resize_uninit(n).unwrap();
  canvas_centers.resize_uninit(n).unwrap();
  canvas_extents.resize_uninit(n).unwrap();
  clips.resize_uninit(n).unwrap();
  z_ord.resize_uninit(n).unwrap();
  focus_ord.resize_uninit(n).unwrap();
  focus_idx.resize_uninit(n).unwrap();
}

void System::push_view(View & view, u32 depth, [[maybe_unused]] u32 breadth,
                       u32 parent)
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

Event System::events_for(View & view)
{
}

void System::build_children(Ctx const & ctx, View & view, u32 idx, u32 depth,
                            u32 viewport, i32 & tab_index)
{
  Slice32 children{views.size32(), 0};

  auto builder = [&](View & child) {
    push_view(child, depth + 1, children.span++, idx);
  };

  State s = view.tick(ctx, events_for(view), &builder);

  att.tab_idx.set(idx, (s.tab == I32_MIN) ? tab_index : s.tab);
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
    focus_grab = idx;
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
    focus_idx[i] = f;
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
    fixed_centers[i] = layout.fixed_center.unwrap_or(Vec2::ZERO);
  }

  // calculate fixed centers; parent-space to local viewport space

  for (auto [i, children] : enumerate(nodes.children))
  {
    // viewports don't propagate fixed-position centers to children
    auto const & fc = att.is_viewport[i] ? Vec2::ZERO : fixed_centers[i];

    for (auto c = children.begin(); c < children.end(); c++)
    {
      if (!fixed[c]) [[likely]]
      {
        fixed_centers[c] = centers[c] + fc;
      }
    }
  }

  // recursively apply viewport transforms to child viewports
  canvas_tx[0]     = Affine3::IDENTITY;
  canvas_inv_tx[0] = Affine3::IDENTITY;

  for (usize i = 0; i < n; i++)
  {
    if (att.is_viewport[i]) [[unlikely]]
    {
      auto parent = att.viewports[i];

      // accumulated parent transform
      auto const & accum     = canvas_tx[parent];
      auto const & inv_accum = canvas_inv_tx[parent];

      // transform we are applying to the viewport's contents
      auto const transform = translate2d(fixed_centers[i]) *
                             scale2d(Vec2::splat(viewport_zooms[i])) *
                             translate2d(-viewport_centers[i]);

      auto const inv_transform = translate_scale_inv2d(transform);

      canvas_tx[i]     = accum * transform;
      canvas_inv_tx[i] = inv_accum * inv_transform;
    }
  }

  for (usize i = 0; i < n; i++)
  {
    auto const & transform   = canvas_tx[att.viewports[i]];
    auto const   zoom        = transform[0][0];
    auto const   translation = transform.z().xy();
    canvas_centers[i]        = fixed_centers[i] + translation;
    canvas_extents[i]        = extents[i] * zoom;
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

void System::focus_scroll(u32 view_idx)
{
  while (view_idx != RootView::NODE &&
         viewports[view_idx] != RootView::VIEWPORT)
  {
    events
      .push(Scroll{.center = fixed_centers[view_idx],
                   .tgt    = views[viewports[view_idx]]->id()})
      .unwrap();

    view_idx = viewports[view_idx];
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
      CRect const  region{.center = canvas_centers[i],
                          .extent = canvas_extents[i]};

      auto const zoom = canvas_tx[parent_viewport][0][0];

      auto & view = views[i];
      canvas.clip(clip);
      view->render(canvas, region, zoom, clip);
    }
  }
}

Option<u32> System::hit_test(Vec2 position) const
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

// [ ] reuse function
Option<HitEvent> System::hit_test(u32 view, Vec2 position) const
{
  // pointer's viewport position
  auto p = transform(canvas_inv_tx[att.viewports[view]], position);

  CRect canvas_region{.center = canvas_centers[view],
                      .extent = canvas_extents[view]};

  if (!canvas_region.contains(p)) [[likely]]
  {
    return none;
  }

  // local position of the pointer within the view
  auto const & fixed_center = fixed_centers[view];
  auto         local        = p - fixed_center;

  return HitEvent{
    .hit = local,
    .region{.center = fixed_center, .extent = extents[view]},
    .canvas = canvas_region
  };
}

u32 System::navigate_focus(u32 from, bool forward) const
{
  CHECK(from < views.size(), "");

  if (views.size() == 1)
  {
    return from;
  }

  i64 const n       = views.size32();
  i64 const advance = forward ? 1 : -1;
  i64       f       = from + advance;

  while (f != from)
  {
    auto const i = focus_ord[f];

    if (!att.hidden[i] && att.focusable[i])
    {
      return (u32) f;
    }

    f += advance;

    if (f == -1)
    {
      f = n - 1;
    }

    if (f == n)
    {
      f = 0;
    }
  }

  return from;
}

HitState System::none_seq(Ctx const & ctx)
{
  if (!ctx.mouse.focused)
  {
    return none;
  }

  return point_seq(ctx, none);
}

HitState System::drag_start_seq(Ctx const & ctx, MouseButton btn, u32 src)
{
  auto diff = [&](Option<u32> tgt) {
    tgt.match([&](auto i) {
      pointer_events.push(PointerEvent{.type = PointerEvent::DragIn, .dst = i})
        .unwrap();
      pointer_events
        .push(PointerEvent{.type = PointerEvent::DragOver, .dst = i})
        .unwrap();
    });
  };

  if (!ctx.mouse.focused || ctx.key.held(KeyCode::Escape))
  {
    pointer_events.push(PointerEvent{.type = PointerEvent::DragEnd, .dst = 0})
      .unwrap();
    return none;
  }

  auto tgt = ctx.mouse.position.match(
    [&](auto p) {
      return bubble_hit(p, [&](auto i) { return att.droppable[i]; });
    },
    []() -> Option<u32> { return none; });

  if (!ctx.mouse.held(btn))
  {
    pointer_events.push(PointerEvent{.type = PointerEvent::DragEnd, .dst = src})
      .unwrap();

    if (tgt.is_none())
    {
      // canceled
      return none;
    }

    diff(tgt);

    pointer_events
      .push(PointerEvent{.type = PointerEvent::Drop, .dst = tgt.value()})
      .unwrap();

    return none;
  }

  pointer_events.push(PointerEvent{.type = PointerEvent::Dragging, .dst = src})
    .unwrap();

  diff(tgt);

  // change to update state
  return DragState{
    .seq = DragState::Update, .src = src, .tgt = tgt, .btn = btn};
}

HitState System::drag_update_seq(Ctx const & ctx, MouseButton btn, u32 src,
                                 Option<u32> prev_tgt)
{
  auto diff = [&](Option<u32> tgt) {
    if (tgt != ViewId::None && tgt != prev_tgt)
    {
      pointer_events.push(PointerEvent{.type = PointerEvent::DragIn, .dst = 0})
        .unwrap();
      pointer_events
        .push(PointerEvent{.type = PointerEvent::DragOver, .dst = 0})
        .unwrap();
    }

    if (prev_tgt != ViewId::None && tgt == prev_tgt)
    {
      pointer_events
        .push(PointerEvent{.type = PointerEvent::DragOver, .dst = 0})
        .unwrap();
    }

    if (prev_tgt != ViewId::None && tgt != prev_tgt)
    {
      pointer_events.push(PointerEvent{.type = PointerEvent::DragOut, .dst = 0})
        .unwrap();
    }
  };

  if (!ctx.mouse.focused || ctx.key.held(KeyCode::Escape))
  {
    diff(ViewId::None);
    return none;
  }

  auto itgt = bubble_hit(ctx.mouse.position.value(),
                         [&](auto i) { return att.droppable[i]; });

  auto tgt =
    itgt.map([&](auto i) { return views[i]->id(); }).unwrap_or(ViewId::None);

  if (!ctx.mouse.held(btn))
  {
    diff(ViewId::None);

    pointer_events.push(PointerEvent{.type = PointerEvent::DragEnd, .dst = 0})
      .unwrap();

    pointer_events
      .push(PointerEvent{.type = PointerEvent::Drop, .dst = itgt.value()})
      .unwrap();

    return none;
  }

  pointer_events.push(PointerEvent{.type = PointerEvent::Dragging, .dst = 0})
    .unwrap();

  diff(tgt);

  return DragState{.seq = DragState::Update, .src = src, .tgt = ViewId::None};
}

HitState System::point_seq(Ctx const & ctx, Option<u32> prev_tgt)
{
  auto diff = [&](ViewId tgt) {
    if (prev_tgt != ViewId::None && tgt != prev_tgt)
    {
      pointer_events
        .push(PointerEvent{.type = PointerEvent::PointerOut, .dst = 0})
        .unwrap();
    }

    if (tgt != ViewId::None && tgt != prev_tgt)
    {
      pointer_events
        .push(PointerEvent{.type = PointerEvent::PointerIn, .dst = 0})
        .unwrap();
      pointer_events
        .push(PointerEvent{.type = PointerEvent::PointerOver, .dst = 0})
        .unwrap();
    }

    if (prev_tgt != ViewId::None && tgt == prev_tgt)
    {
      pointer_events
        .push(PointerEvent{.type = PointerEvent::PointerOver, .dst = 0})
        .unwrap();
    }
  };

  /**
 
  
  */

  // [ ] be more generic with handling buttons
  // [ ] when frame is being built store ids and idx map of views we are interested in.
  // [ ] indices; use index to attach event: MetaData, don't need index, reset metadata field at start of frame? ----> if frame isn't rebuilt?
  //
  // [ ] we don't want views to run away with the attached metadata
  //

  if (!ctx.mouse.focused)
  {
    diff(ViewId::None);
    return none;
  }

  if (ctx.mouse.scrolled)
  {
    auto itgt = bubble_hit(ctx.mouse.position.value(),
                           [&](auto i) { return att.scrollable[i]; });


    if (tgt != ViewId::None)
    {
      diff(tgt);
      return PointState{};
    }
  }

  if (ctx.mouse.down(MouseButton::Primary))
  {
    auto itgt = bubble_hit(ctx.mouse.position.value(), [&](auto i) {
      return att.draggable[i] || att.clickable[i];
    });


    if (tgt != ViewId::None)
    {
      auto draggable = att.draggable[itgt.value()];

      diff(tgt);

      if (draggable)
      {
        return DragState{};
      }

      return PointState{};
    }
  }

  auto itgt = bubble_hit(ctx.mouse.position.value(),
                         [&](auto i) { return att.pointable[i]; });


  diff(tgt);

  return PointState{};
}

HitState System::hit_seq(Ctx const & ctx)
{
  auto new_hit =
    hit.match([&](None) { return none_seq(ctx); },
              [&](auto & h) {
                switch (h.seq)
                {
                  case DragState::Start:
                    return drag_start_seq(ctx, h.btn, h.src);
                  case DragState::Update:
                    return drag_update_seq(ctx, h.btn, h.src, h.tgt);
                }
              },
              [&](auto & h) { return point_seq(ctx, h.tgt); });

  for (auto const & event : pointer_events)
  {
    switch (event.type)
    {
      case PointerEvent::None:
      case PointerEvent::PointerIn:
      case PointerEvent::PointerOut:
      case PointerEvent::PointerOver:
      case PointerEvent::Scroll:
      case PointerEvent::DragIn:
      case PointerEvent::DragOut:
      case PointerEvent::DragOver:
        break;

      case PointerEvent::PointerDown:
      case PointerEvent::PointerUp:
      case PointerEvent::DragStart:
      case PointerEvent::Dragging:
      case PointerEvent::DragEnd:
      case PointerEvent::Drop:
        // [ ] if focusable and pressed or dragged, update focus index; preserve focus active
        break;
    }
  }

  hit = new_hit;
}

/*
ViewEvents System::process_events(View & view)
{
  ViewEvents events;

  if (view.id() == ViewId::None) [[unlikely]]
  {
    // should never happen
    CHECK(next_id != U64_MAX, "");
    view.id_       = ViewId{next_id++};
    events.mounted = true;
  }



  if (f1.focus.is_some() && f1.focus.value().view == view.id() &&
      f1.focus.value().active) [[unlikely]]
  {
    events.focus_in = f0.focus.is_none() ||
                      (f0.focus.value().view != view.id()) ||
                      !f0.focus.value().active;
    events.key_down   = f1.key_down;
    events.key_up     = f1.key_up;
    events.text_input = f1.text_input;
  }
  else if (f0.focus.is_some() && f0.focus.value().view == view.id() &&
           f0.focus.value().active) [[unlikely]]
  {
    events.focus_out = true;
  }

  for (auto scroll : scrolls)
  {
    if (scroll.tgt == view.id())
    {
      events.scroll = Scroll{.center = scroll.center, .extent = none};
      break;
    }
  }

  return events;
}
  */

void System::event_dispatch(InputState const & ctx)
{
  ScopeTrace trace;

  /*
  bool const esc_input = ctx.key_down(KeyCode::Escape);
  bool const tab_input = ctx.key_down(KeyCode::Tab);

  f0 = f1;
  f1 = State{};

  f1.mouse_down     = ctx.mouse.any_down;
  f1.mouse_up       = ctx.mouse.any_up;
  f1.mouse_moved    = ctx.mouse.moved;
  f1.mouse_scrolled = ctx.mouse.wheel_scrolled;
  f1.key_down       = ctx.key.any_down;
  f1.key_up         = ctx.key.any_up;
  f1.text_input     = ctx.text_input;

  f1.dropped =
    (f0.dragging && ctx.mouse_up(MouseButton::Primary)) || ctx.dropped;
  f1.drag_end = f1.dropped;
  f1.dragging = !f1.dropped;
  f1.drag_src = f1.dragging ? f0.drag_src : none;

  // use grab focus request if any, otherwise persist previous frame's focus
  f1.focus = f0.grab_focus ? f0.grab_focus : f0.focus;
*/

  // mouse click & drag

  // determine focus navigation direction
  FocusAction focus_action = FocusAction::None;

  // navigate focus request
  if (tab_input)
  {
    if (!(f1.focus.is_some() && f1.focus.value().input_info.tab_input))
    {
      if (ctx.key_down(KeyCode::LeftShift) || ctx.key_down(KeyCode::RightShift))
      {
        focus_action = FocusAction::Backward;
      }
      else
      {
        focus_action = FocusAction::Forward;
      }
    }
  }

  // clear focus request
  if (esc_input)
  {
    if (!(f1.focus.is_some() && f1.focus.value().input_info.esc_input))
    {
      f1.focus = none;
    }
  }

  switch (focus_action)
  {
    case FocusAction::Forward:
    case FocusAction::Backward:
    {
      u32 from = 0;
      if (f1.focus.is_some())
      {
        from = f1.focus.value().focus_idx;
      }
      else
      {
        from = 0;
      }

      f1.focus =
        navigate_focus(from, focus_action == FocusAction::Forward)
          .map([&](u32 focus_idx) {
            u32 const i = focus_ordering[focus_idx];
            focus_scroll(i);
            // [ ] remove previous focus code. refactor this part of the code4 instead
            return Focus{
              .active     = true,
              .view       = views[i]->id(),
              .focus_idx  = focus_idx,
              .input      = is_input[i],
              .input_info = TextInputInfo{.type        = input_type[i],
                                          .multiline   = is_multiline_input[i],
                                          .esc_input   = is_esc_input[i],
                                          .tab_input   = is_tab_input[i],
                                          .cap         = input_cap[i],
                                          .autocorrect = input_autocorrect[i]}
            };
          });
    }
    break;
    default:
    case FocusAction::None:
      break;
  }

  if (f1.focus.is_some())
  {
    Focus &   focus = f1.focus.value();
    u32 const i     = focus_ord[focus.focus_idx];
    focus.region    = views[i]->region_;
  }
}

Option<TextInputInfo> System::text_input() const
{
  if (!f1.focus)
  {
    return none;
  }

  // [ ] text region rect

  Focus f = f1.focus.value();

  if (!f.input)
  {
    return none;
  }

  return f.input_info;
}

bool System::tick(InputState const & input, View & root, Canvas & canvas,
                  Fn<void(Ctx const &)> loop)
{
  ScopeTrace trace;

  // [ ]
  RootView base_root{&root};

  clear_frame();

  build(s1, root);
  u32 const n = views.size32();
  prepare_for(n);

  loop(input);
  // [ ] focus rect

  focus_order();

  layout(as_vec2(input.window_extent));
  stack();
  visibility();
  render(canvas);

  events(input);
  input.clone_to(s1);

  frame++;

  bool const should_close = input.closing && !closing_deferred;

  return !should_close;
}

}    // namespace ui
}    // namespace ash
