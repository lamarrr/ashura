/// SPDX-License-Identifier: MIT
#include "ashura/engine/view_system.h"
#include "ashura/std/error.h"
#include "ashura/std/range.h"
#include "ashura/std/trace.h"

namespace ash
{

/// @brief compares the z-order of `a` and `b`
static constexpr Ordering z_order_cmp(i32 a_layer, i32 a_z_index, u32 a_depth,
                                      i32 b_layer, i32 b_z_index, u32 b_depth)
{
  // cmp stacking context/layer first
  Ordering ord = cmp(a_layer, b_layer);

  if (ord != Ordering::Equal)
  {
    return ord;
  }

  // cmp z_index
  ord = cmp(a_z_index, b_z_index);

  if (ord != Ordering::Equal)
  {
    return ord;
  }

  // cmp depth in the view tree
  return cmp(a_depth, b_depth);
}

static constexpr bool hit_test(ui::View & view, Vec2 position)
{
  return contains(view.region_, position) &&
         view.hit(view.region_, view.zoom_, position);
}

void ViewSystem::clear()
{
  views.clear();
  nodes.clear();

  tab_indices.clear();
  viewports.clear();
  is_hidden.clear();
  is_pointable.clear();
  is_clickable.clear();
  is_scrollable.clear();
  is_draggable.clear();
  is_droppable.clear();
  is_focusable.clear();

  is_input.clear();
  input_type.clear();
  is_multiline_input.clear();
  is_esc_input.clear();
  is_tab_input.clear();
  input_cap.clear();
  input_autocorrect.clear();

  is_viewport.clear();

  centers.clear();
  extents.clear();
  viewport_extents.clear();
  viewport_transforms.clear();
  is_fixed_positioned.clear();
  fixed_positions.clear();
  z_indices.clear();
  layers.clear();

  transforms.clear();
  clips.clear();
  z_ordering.clear();
  focus_ordering.clear();
  scrolls.clear();
  closing_deferred = false;
}

ui::ViewEvents ViewSystem::process_events(ui::View & view)
{
  ui::ViewEvents events;

  if (view.id() == ui::ViewId::None) [[unlikely]]
  {
    // should never happen
    CHECK(next_id != U64_MAX, "");
    view.id_       = ui::ViewId{next_id++};
    events.mounted = true;
  }

  auto const id = view.id();

  events.view_hit = (view.last_rendered_frame_ + 1) == frame;

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

  Option<Vec2> scroll_position;

  for (auto scroll : scrolls)
  {
    if (scroll.view_id == view.id())
    {
      scroll_position = scroll.position;
      break;
    }
  }

  events.scroll = scroll_position;

  return events;
}

void ViewSystem::push_view(ui::View & view, u32 depth, u32 breadth, u32 parent)
{
  views.push(view).unwrap();
  nodes
    .push(ViewNode{
      .depth = depth, .breadth = breadth, .parent = parent, .children = {}})
    .unwrap();
  tab_indices.extend_uninit(1).unwrap();
  viewports.extend_uninit(1).unwrap();
  is_hidden.extend_uninit(1).unwrap();
  is_pointable.extend_uninit(1).unwrap();
  is_clickable.extend_uninit(1).unwrap();
  is_scrollable.extend_uninit(1).unwrap();
  is_draggable.extend_uninit(1).unwrap();
  is_droppable.extend_uninit(1).unwrap();
  is_focusable.extend_uninit(1).unwrap();
  is_input.extend_uninit(1).unwrap();
  input_type.extend_uninit(1).unwrap();
  is_multiline_input.extend_uninit(1).unwrap();
  is_esc_input.extend_uninit(1).unwrap();
  is_tab_input.extend_uninit(1).unwrap();
  input_cap.extend_uninit(1).unwrap();
  input_autocorrect.extend_uninit(1).unwrap();
  is_tab_input.extend_uninit(1).unwrap();
  is_esc_input.extend_uninit(1).unwrap();
  is_viewport.extend_uninit(1).unwrap();
}

void ViewSystem::build_children(ui::ViewContext const & ctx, ui::View & view,
                                u32 idx, u32 depth, i32 & tab_index,
                                u32 viewport)
{
  Slice32 children{views.size32(), 0};

  auto builder = [&](ui::View & child) {
    push_view(child, depth + 1, children.span++, idx);
  };

  ui::ViewState s =
    view.tick(ctx, view.region_, view.zoom_, process_events(view), &builder);

  bool const    has_input  = s.text.is_some();
  TextInputInfo input_info = s.text.unwrap_or(TextInputInfo{});

  tab_indices.set(idx, (s.tab == I32_MIN) ? tab_index : s.tab);
  viewports.set(idx, viewport);
  is_hidden.set(idx, s.hidden);
  is_pointable.set(idx, s.pointable);
  is_clickable.set(idx, s.clickable);
  is_scrollable.set(idx, s.scrollable);
  is_draggable.set(idx, s.draggable);
  is_droppable.set(idx, s.droppable);
  is_focusable.set(idx, s.focusable);
  is_input.set(idx, has_input);
  input_type.set(idx, input_info.type);
  is_multiline_input.set(idx, input_info.multiline);
  is_esc_input.set(idx, input_info.esc_input);
  is_tab_input.set(idx, input_info.tab_input);
  input_cap.set(idx, input_info.cap);
  input_autocorrect.set(idx, input_info.autocorrect);
  is_viewport.set(idx, s.viewport);
  closing_deferred |= s.defer_close;

  if (!s.hidden && s.focusable && s.grab_focus) [[unlikely]]
  {
    f1.grab_focus = Focus{.active     = true,
                          .view       = view.id(),
                          .focus_idx  = view.focus_idx_,
                          .input      = has_input,
                          .input_info = input_info};
  }

  nodes[idx].children         = children;
  u32 const children_viewport = s.viewport ? idx : viewport;

  for (u32 c = children.begin(); c < children.end(); c++)
  {
    tab_index++;
    build_children(ctx, views[c], c, depth + 1, tab_index, children_viewport);
  }
}

void ViewSystem::build(ui::ViewContext const & ctx, ui::View & root)
{
  push_view(root, 0, 0, U32_MAX);
  i32 tab_index = 0;
  build_children(ctx, root, 0, 0, tab_index, U32_MAX);
}

void ViewSystem::focus_order()
{
  ScopeTrace trace;

  iota(focus_ordering.view(), 0U);

  indirect_sort(focus_ordering.view(),
                [&](u32 a, u32 b) { return tab_indices[a] < tab_indices[b]; });

  for (u32 i = 0; i < views.size32(); i++)
  {
    views[focus_ordering[i]]->focus_idx_ = i;
  }
}

void ViewSystem::layout(Vec2 viewport_extent)
{
  ScopeTrace trace;

  if (views.is_empty())
  {
    return;
  }

  u32 const n = views.size32();

  // allocate sizes to children recursively
  extents[0] = viewport_extent;
  for (u32 i = 0; i < n; i++)
  {
    auto const & node = nodes[i];
    views[i]->size(extents[i], extents.view().slice(node.children));
  }

  centers[0] = Vec2::splat(0);

  // fit parent views along the finalized sizes of the child views and
  // assign centers to the children based on their sizes.
  for (u32 i = n; i != 0;)
  {
    i--;
    auto const & node = nodes[i];
    auto layout = views[i]->fit(extents[i], extents.view().slice(node.children),
                                centers.view().slice(node.children));
    extents[i]  = layout.extent;
    viewport_extents[i]    = layout.viewport_extent;
    viewport_transforms[i] = layout.viewport_transform;
    is_fixed_positioned.set(i, layout.fixed_position.is_some());
    if (layout.fixed_position.is_some()) [[unlikely]]
    {
      fixed_positions[i] = layout.fixed_position.value();
    }
  }

  // transform views to canvas-space


  transforms[0] = Affine3::identity();

  for (u32 i = 0; i < n; i++)
  {
    auto const & node               = nodes[i];
    // parent-space to local viewport-space transformation matrix
    auto const & viewport_transform = viewport_transforms[i];
    // accumulated transform of all ancestors, determines position until this
    // parent
    auto const & ancestor_transform = transforms[i];
    for (u32 c = node.children.begin(); c < node.children.end(); c++)
    {
      transforms[c] =
        // apply viewport-space transform
        viewport_transform
        // apply parent-space transform
        * translate2d(centers[c])
        // first use accumulated ancestor transform
        * ancestor_transform;
    }
  }

  //  [ ] pre-transform or post-transform?
  // views[i]->viewport_region_ = CRect{.center = center, .extent = extent};

  // transform to [0, viewport_extent] space
  for (u32 i = 0; i < n; i++)
  {
    auto const & t    = transforms[i];
    auto const center = ash::transform(t, Vec2{0, 0}) + 0.5F * viewport_extent;
    auto const extent = extents[i];
    f32 const  zoom   = t[0][0];
    centers[i]        = center;
    extents[i]        = extent * zoom;
    viewport_extents[i] = viewport_extents[i] * zoom;
  }

  for (u32 i = 0; i < n; i++)
  {
    if (is_fixed_positioned[i]) [[unlikely]]
    {
      centers[i] = fixed_positions[i];
    }
  }

  CRect const viewport_clip =
    CRect::from_offset(Vec2::splat(0), viewport_extent);

  fill(clips, viewport_clip);

  /// recursive view clipping
  for (u32 i = 0; i < n; i++)
  {
    u32 const viewport = viewports[i];
    if (is_viewport[i]) [[unlikely]]
    {
      CRect const clip{.center = centers[i], .extent = extents[i]};
      if (viewport != U32_MAX) [[likely]]
      {
        clips[i] = intersect(clip, clips[viewport]);
      }
      else
      {
        clips[i] = clip;
      }
    }
  }

  /// assign viewport clips to contained views
  for (u32 i = 0; i < n; i++)
  {
    u32 const viewport = viewports[i];
    if (!is_viewport[i] && viewport != U32_MAX) [[likely]]
    {
      clips[i] = clips[viewport];
    }
  }

  for (u32 i = 0; i < n; i++)
  {
    ui::View & view = views[i];
    view.region_    = CRect{.center = centers[i], .extent = extents[i]};
    view.zoom_      = transforms[i][0][0];
  }
}

void ViewSystem::stack()
{
  ScopeTrace trace;

  u32 const n = views.size32();

  if (n == 0)
  {
    return;
  }

  z_indices[0] = 0;

  for (u32 i = 0; i < n; i++)
  {
    ViewNode const & node = nodes[i];
    z_indices[i] =
      views[i]->z_index(z_indices[i], z_indices.view().slice(node.children));
  }

  layers[0] = 0;
  for (u32 i = 0; i < n; i++)
  {
    ViewNode const & node = nodes[i];
    if (node.parent != U32_MAX)
    {
      layers[i] = views[i]->stack(layers[node.parent]);
    }
  }

  iota(z_ordering.view(), 0U);

  // sort layers
  indirect_sort(z_ordering.view(), [&](u32 a, u32 b) {
    Ordering ord = z_order_cmp(layers[a], z_indices[a], nodes[a].depth,
                               layers[b], z_indices[b], nodes[b].depth);
    return ord == Ordering::Less;
  });
}

void ViewSystem::visibility()
{
  ScopeTrace trace;

  for (u32 i = 0; i < views.size32(); i++)
  {
    ViewNode const & node = nodes[i];

    if (is_hidden[i])
    {
      // if parent requested to be hidden, make children hidden
      for (u32 c = node.children.begin(); c < node.children.end(); c++)
      {
        is_hidden.set_bit(c);
      }
    }
    else
    {
      CRect region{.center = centers[i], .extent = extents[i]};

      CRect const & clip = clips[i];

      bool const hidden = !overlaps(region, clip);

      is_hidden.set(i, hidden);
    }
  }
}

void ViewSystem::render(Canvas & canvas)
{
  ScopeTrace trace;

  for (u32 i : z_ordering)
  {
    if (!is_hidden.get(i)) [[unlikely]]
    {
      ui::View & view = views[i];
      canvas.clip(clips[i]);
      view.render(canvas, view.region_, view.zoom_, clips[i]);
      view.last_rendered_frame_ = frame;
    }
  }
}

void ViewSystem::scroll_to(u32 view_idx)
{
  while (view_idx != U32_MAX && viewports[view_idx] != U32_MAX)
  {
    ref  view               = views[view_idx];
    auto parent_viewport_id = views[viewports[view_idx]]->id();

    scrolls
      .push(ScrollRequest{.position = view->viewport_region_.center,
                          .view_id  = parent_viewport_id})
      .unwrap();

    view_idx = viewports[view_idx];
  }
}

Option<u32> ViewSystem::hit_views(Vec2              mouse_position,
                                  ViewHitAttributes match) const
{
  u32 const n = views.size32();

  // find in reverse z-order
  for (u32 z = n; z != 0;)
  {
    z--;

    u32 const  i    = z_ordering[z];
    ui::View & view = views[i];

    bool matches = false;

    // avoids reading from the bit arrays if not needed

    if (has_bits(match, ViewHitAttributes::Pointable))
    {
      matches |= is_pointable[i];
    }

    if (has_bits(match, ViewHitAttributes::Clickable))
    {
      matches |= is_clickable[i];
    }

    if (has_bits(match, ViewHitAttributes::Scrollable))
    {
      matches |= is_scrollable[i];
    }

    if (has_bits(match, ViewHitAttributes::Draggable))
    {
      matches |= is_draggable[i];
    }

    if (has_bits(match, ViewHitAttributes::Droppable))
    {
      matches |= is_droppable[i];
    }

    if (!is_hidden[i] && matches && hit_test(view, mouse_position)) [[unlikely]]
    {
      return i;
    }
  }

  return none;
}

Option<u32> ViewSystem::navigate_focus(u32 from, bool forward) const
{
  u32 const n = views.size32();

  if (n == 0)
  {
    return none;
  }

  if (from > n)
  {
    from = n - 1;
  }

  if (forward)
  {
    u32 f = (from + 1) % n;

    while (f != from)
    {
      u32 const i = focus_ordering[f];
      if (!is_hidden[i] && is_focusable[i])
      {
        break;
      }

      f = (f + 1) % n;
    }

    return f;
  }
  else
  {
    u32 start = (n - 1) - from;
    u32 f     = (start + 1) % n;

    while (f != start)
    {
      u32 const i = focus_ordering[(n - 1) - f];
      if (!is_hidden[i] && is_focusable[i])
      {
        break;
      }

      f = (f + 1) % n;
    }

    return (n - 1) - f;
  }
}

void ViewSystem::dispatch_hit(ui::ViewContext const & ctx, HitEvent event)
{
  auto       i    = event.view;
  ui::View & view = views[i];

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

      f1.cursor = view.cursor(view.region_, view.zoom_, ctx.mouse.position);

      f1.focus = Focus{
        .active     = true,
        .view       = view.id(),
        .focus_idx  = view.focus_idx_,
        .input      = is_input[i],
        .input_info = TextInputInfo{.type        = input_type[i],
                                    .multiline   = is_multiline_input[i],
                                    .esc_input   = is_esc_input[i],
                                    .tab_input   = is_tab_input[i],
                                    .cap         = input_cap[i],
                                    .autocorrect = input_autocorrect[i]}
      };
    }
    break;
    case HitType::Drop:
    case HitType::DragUpdate:
    case HitType::Release:
    case HitType::Scroll:
    case HitType::Point:
    {
      f1.pointed = view.id();
      f1.cursor  = view.cursor(view.region_, view.zoom_, ctx.mouse.position);
    }
    break;
  }

  scroll_to(i);
}

void ViewSystem::events(ui::ViewContext const & ctx)
{
  ScopeTrace trace;

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

  // mouse click & drag
  if (f1.mouse_down)
  {
    hit_views(ctx.mouse.position,
              ViewHitAttributes::Clickable | ViewHitAttributes::Draggable)
      .match([&](u32 i) {}, [&]() { f1.focus = none; });
  }
  // mouse drop event
  else if ((f0.dragging && ctx.mouse_up(MouseButton::Primary)) || ctx.dropped)
  {
    f1.dropped  = true;
    f1.dragging = false;

    hit_views(ctx.mouse.position, ViewHitAttributes::Droppable)
      .match([&](u32 i) {});
  }
  // mouse dragging update event
  else if (f0.dragging || ctx.drop_hovering)
  {
    f1.drag_src = f0.drag_src;
    f1.dragging = true;

    hit_views(ctx.mouse.position, ViewHitAttributes::Droppable)
      .match([&](u32 i) {

      });
  }
  // mouse release event
  else if (f1.mouse_up)
  {
    hit_views(ctx.mouse.position, ViewHitAttributes::Clickable)
      .match([&](u32 i) {});
  }
  // mouse scroll event
  else if (ctx.mouse.wheel_scrolled)
  {
    hit_views(ctx.mouse.position, ViewHitAttributes::Scrollable)
      .match([&](u32 i) {

      });
  }
  // mouse pointing event
  else
  {
    hit_views(ctx.mouse.position,
              ViewHitAttributes::Pointable | ViewHitAttributes::Clickable |
                ViewHitAttributes::Draggable | ViewHitAttributes::Scrollable)
      .match([&](u32 i) {

      });
  }

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
            scroll_to(i);
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
    u32 const i     = focus_ordering[focus.focus_idx];
    focus.region    = views[i]->region_;
  }
}

Cursor ViewSystem::cursor() const
{
  return f1.cursor;
}

Option<TextInputInfo> ViewSystem::text_input() const
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

bool ViewSystem::tick(InputState const & input, ui::View & root,
                      Canvas & canvas, Fn<void(ui::ViewContext const &)> loop)
{
  ScopeTrace trace;

  clear();

  build(s1, root);
  u32 const n = views.size32();
  centers.resize_uninit(n).unwrap();
  extents.resize_uninit(n).unwrap();
  viewport_extents.resize_uninit(n).unwrap();
  viewport_transforms.resize_uninit(n).unwrap();
  is_fixed_positioned.resize_uninit(n).unwrap();
  fixed_positions.resize_uninit(n).unwrap();
  z_indices.resize_uninit(n).unwrap();
  layers.resize_uninit(n).unwrap();
  transforms.resize_uninit(n).unwrap();
  clips.resize_uninit(n).unwrap();
  z_ordering.resize_uninit(n).unwrap();
  focus_ordering.resize_uninit(n).unwrap();

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

}    // namespace ash
