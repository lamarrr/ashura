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
  nodes.ancestory.clear();
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
  is_fixed_centered.clear();
  fixed_centers.clear();
  z_idx.clear();
  layers.clear();
  canvas_tx.clear();
  canvas_inverse_tx.clear();
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
  is_fixed_centered.resize_uninit(n).unwrap();
  fixed_centers.resize_uninit(n).unwrap();
  z_idx.resize_uninit(n).unwrap();
  layers.resize_uninit(n).unwrap();
  canvas_tx.resize_uninit(n).unwrap();
  canvas_inverse_tx.resize_uninit(n).unwrap();
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
  nodes.ancestory.extend_uninit(1).unwrap();
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
                            u32 viewport, i32 & tab_index, u32 & timer)
{
  Slice32 children{views.size32(), 0};

  auto builder = [&](View & child) {
    push_view(child, depth + 1, children.span++, idx);
  };

  State s = view.tick(ctx, events_for(view), &builder);

  bool pointable =
    s.pointable | s.scrollable | s.draggable | s.droppable | s.focusable;

  att.tab_idx.set(idx, (s.tab == I32_MIN) ? tab_index : s.tab);
  att.viewports.set(idx, viewport);
  att.hidden.set(idx, s.hidden);
  att.pointable.set(idx, pointable);
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

  auto enter = timer;

  for (auto c = children.begin(); c < children.end(); c++)
  {
    tab_index++;    // depth-first
    timer++;
    build_children(ctx, views[c], c, depth + 1, children_viewport, tab_index,
                   timer);
  }

  timer++;

  auto exit = timer;

  nodes.ancestory[idx] = NodeTiming{.enter = enter, .exit = exit};
}

void System::build(Ctx const & ctx, RootView & root)
{
  push_view(root, 0, 0, RootView::PARENT);
  i32 tab_index = 0;
  u32 timer     = 0;
  build_children(ctx, root, 0, 0, RootView::VIEWPORT, tab_index, timer);
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
    is_fixed_centered.set(i, layout.fixed_center.is_some());
    fixed_centers[i] = layout.fixed_center.unwrap_or(Vec2::ZERO);
  }

  // calculate fixed centers; parent-space to local viewport space

  for (auto [i, children] : enumerate(nodes.children))
  {
    // viewports don't propagate fixed-position centers to children
    auto const & fc = att.is_viewport[i] ? Vec2::ZERO : fixed_centers[i];

    for (auto c = children.begin(); c < children.end(); c++)
    {
      if (!is_fixed_centered[c]) [[likely]]
      {
        fixed_centers[c] = centers[c] + fc;
      }
    }
  }

  // recursively apply viewport transforms to child viewports
  canvas_tx[0]         = Affine3::IDENTITY;
  canvas_inverse_tx[0] = Affine3::IDENTITY;

  for (usize i = 0; i < n; i++)
  {
    if (att.is_viewport[i]) [[unlikely]]
    {
      auto parent = att.viewports[i];

      // accumulated parent transform
      auto const & accum     = canvas_tx[parent];
      auto const & inv_accum = canvas_inverse_tx[parent];

      // transform we are applying to the viewport's contents
      auto const transform = translate2d(fixed_centers[i]) *
                             scale2d(Vec2::splat(viewport_zooms[i])) *
                             translate2d(-viewport_centers[i]);

      auto const inv_transform = translate_scale_inv2d(transform);

      canvas_tx[i]         = accum * transform;
      canvas_inverse_tx[i] = inv_accum * inv_transform;
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

  for (auto [parent, layer, view] : zip(nodes.parent, layers, views))
  {
    layer = view->layer(layers[parent]);
  }

  iota(z_ord.view(), 0U);

  // sort layers
  indirect_sort(z_ord.view(), [&](auto a, auto b) {
    return z_cmp(layers[a], z_idx[a], nodes.depth[a], layers[b],
                 z_idx[b], nodes.depth[b]) == Order::Less;
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
      CRect const  region{.center = canvas_centers[i],
                          .extent = canvas_extents[i]};

      auto const zoom = canvas_tx[parent_viewport][0][0];

      auto & view = views[i];
      canvas.clip(clip);
      view->render(canvas, region, zoom, clip);
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

Option<HitEvent> System::hit_test(u32 view, Vec2 position) const
{
  // pointer's viewport position
  auto p = transform(canvas_inverse_tx[att.viewports[view]], position);

  CRect canvas_region{.center = canvas_centers[view],
                      .extent = canvas_extents[view]};

  if (!canvas_region.contains(p)) [[likely]]
  {
    return none;
  }

  // local position of the pointer within the view
  auto const & fixed_center = fixed_centers[view];
  auto         local        = p - fixed_center;

  if (!views[view]->hit(extents[view], local)) [[likely]]
  {
    return none;
  }

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

/*
void System::dispatch_hit(Ctx const & ctx, HitEvent event)
{
  auto       i    = event.view;
  View & view = views[i];

  switch (event.type)
  {
    case HitType::None:
      break;
    case HitType::Click:
    case HitType::Drag:
    {
      f1.pointed = view.id();

      if (ctx.mouse_down(MouseButton::Primary) && is_draggable[i])
      {
        f1.dragging   = true;
        f1.drag_src   = view.id();
        f1.drag_start = true;
      }

      //[ ] f1.cursor = view.cursor(extents[i], fixed_centers[i]);

      f1.focus = Focus{.active     = true,
                       .view       = view.id(),
                       .focus_idx  = view.focus_idx_,
                       .input_info = input_infos[i]};
    }
    break;
    case HitType::Drop:
    case HitType::DragUpdate:
    case HitType::Release:
    case HitType::Scroll:
    case HitType::Point:
    {
      f1.pointed = view.id();
      // f1.cursor  = view.cursor(view.region_, view.zoom_, ctx.mouse.position);
    }
    break;
  }

  focus_scroll(i);
}
*/

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

  auto const id = view.id();

  if (f1.pointed.contains(id)) [[unlikely]]
  {
    if (f1.dragging) [[unlikely]]
    {
      events.drag_in   = !f0.pointed.contains(id);
      events.drag_over = true;
    }

    events.drop         = f1.dropped;
    events.mouse_in     = !f0.pointed.contains(id);
    events.mouse_down   = f1.mouse_down;
    events.mouse_up     = f1.mouse_up;
    events.mouse_moved  = f1.mouse_moved;
    events.mouse_scroll = f1.mouse_scrolled;
  }
  else if (f0.pointed.contains(id)) [[unlikely]]
  {
    events.mouse_out = true;
    events.drag_out  = f0.dragging;
  }

  if (f1.drag_src.contains(id)) [[unlikely]]
  {
    events.drag_start = f1.drag_start;
    events.dragging   = f1.dragging;
    events.drag_end   = f1.drag_end;
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

HitState System::none_seq(Ctx const & ctx)
{
  // if has mouse focused, activate point state
  // [ ] if mouse not focused
  if (ctx.mouse.focused)
  {
    return point_seq(ctx, ViewId::None);
  }
  else
  {
    return none;
  }
}

HitState System::drag_start_seq(Ctx const & ctx, MouseButton btn, ViewId src)
{
  // canceled by release/out of focus:
  // if canceled, send dropend to source, move to point state
  auto hit =
    hit_views(ctx.mouse.position, [&](auto i) { return is_pointable[i]; });

  bool can_drop = hit && att.droppable[hit.value().v0];
  bool canceled = !ctx.mouse.focused || (ctx.mouse.state(btn) && !can_drop) ||
                  ctx.key.state(KeyCode::Escape);

  // [ ] updating focus

  if (canceled)
  {
    // send cancel to src
    return none;
  }
  else if (can_drop)
  {
    // send drop-in
  }
  else
  {
    // change to update state
    return none;
  }
}

HitState System::drag_update_seq(Ctx const & ctx, MouseButton btn, ViewId src,
                                 ViewId tgt)
{
  // send dropout if previous state was update and left a droppable.
  // continue at current state; unless canceled by r/oof
  // if canceled, send dropend to source, move to point state

  auto hit =
    hit_views(ctx.mouse.position, [&](auto i) { return is_pointable[i]; });

  bool can_drop = hit && att.droppable[hit.value().v0];
  bool canceled = !ctx.mouse.focused || (ctx.mouse.state(btn) && !can_drop) ||
                  ctx.key.state(KeyCode::Escape);
  bool dropped = can_drop && !canceled;

  if (canceled)
  {
    // send cancel to src
    // send drop-out to tgt
    return none;
  }
  else if (dropped)
  {
    return none;
  }
  else if (can_drop)
  {
    // send drop-in to new tgt
    // send drop-out to old tgt
    return DragState{
      .seq = DragState::Seq::Update, .src = src, .tgt = ViewId::None};
  }
  else
  {
    // send update to src
    // send drop-in to new tgt
    // send drop-out to old tgt
  }
}

HitState System::point_seq(Ctx const & ctx, ViewId tgt)
{
  // get currently pointed, if different, send mouse out
  // if mouse down: hit-test for drag & click, if dragged, move to drag state, otherwise stay at point state and send mouse down
  // if mouse up: send mouse up to new view
  // if mouse moved - mouse update
  // if same, update point target
  // if mouse

  // we only want views that are actually clickable/draggable and not obstructed!!!
  // our hit test function should stop totally if the !hidden && .contains(p) && .hit(p) test passes

  // our filter will make sure we only stop if the hit criteria matches a condition
  // i.e. for click or drag tests we want to only consider the top-most clickable or draggable item.

  // for each intended event, if it is a mouse down, check the first clickable, if none,
  // check the first draggable?
  //
  auto hit = hit_views(ctx.mouse.position, [&](auto i) {
    //[ ] isn't this just pointable?
    return is_clickable[i] | is_draggable[i];
  });

  // [ ] how views can request drag cancel?
  // [ ] let the view determine which button to use for dragging
  // [ ] be more generic with handling buttons

  bool drag_start = hit && is_draggable[hit.value().v0];
  bool pointing   = hit && is_pointable[hit.value().v0];
  bool canceled   = !ctx.mouse.focused;

  //

  // if out of view or move to new view, or mouse out of focus; send mouse out
  //
  //
  // [ ] wheel scroll?
  // [ ] middle wheel drag
  //
  if (canceled)
  {
    return none;
  }
  else if (drag_start)
  {
  }
  else if (pointing)
  {
  }
  else
  {
    // pointing to none
  }
}

HitState System::hit_seq(Ctx const & ctx)
{
  // [ ] if focusable and pressed or dragged, update focus index; preserve focus active

  auto new_hit =
    hit.match([&](None) { return none_seq(ctx); },
              [&](DragState const & h) {
                switch (h.seq)
                {
                  case DragState::Seq::Start:
                    return drag_start_seq(ctx, h.btn, h.src);
                  case DragState::Seq::Update:
                    return drag_update_seq(ctx, h.btn, h.src, h.tgt);
                }
              },
              [&](PointState const & h) { return point_seq(ctx, h.tgt); });

  hit = new_hit;
}

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
