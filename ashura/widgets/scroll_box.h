
#pragma once

#include "ashura/color.h"
#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/widget.h"

namespace ash
{
namespace gui
{

struct ScrollBoxProps
{
  Color        thumb_color = material::GRAY_400;
  Color        track_color = material::GRAY_800;
  f32          bar_width   = 15;
  Constraint2D view_offset = Constraint2D::relative(0, 0);
  Constraint2D frame       = Constraint2D::absolute(200, 200);
};

struct ScrollCtx
{
  Constraint2D   view_offset;
  Vec2           view_size;
  Vec2           content_size;
  ScrollBoxProps props;

  constexpr bool can_scroll_x() const
  {
    return content_size.x > view_size.x;
  }

  constexpr bool can_scroll_y() const
  {
    return content_size.y > view_size.y;
  }
};

struct ScrollBar : public Widget
{
  ScrollBar(Direction idirection, stx::Rc<ScrollCtx *> iscroll_ctx) :
      direction{idirection}, scroll_ctx{std::move(iscroll_ctx)}
  {
  }

  STX_DISABLE_COPY(ScrollBar)
  STX_DEFAULT_MOVE(ScrollBar)

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
  {
    Vec2 size;
    if (direction == Direction::V)
    {
      if (scroll_ctx->can_scroll_x() && scroll_ctx->can_scroll_y())
      {
        size.x = scroll_ctx->props.bar_width;
        size.y = scroll_ctx->view_size.y - scroll_ctx->props.bar_width;
      }
      else if (scroll_ctx->can_scroll_y())
      {
        size.x = scroll_ctx->props.bar_width;
        size.y = scroll_ctx->view_size.y;
      }
    }
    else
    {
      if (scroll_ctx->can_scroll_x() && scroll_ctx->can_scroll_y())
      {
        size.x = scroll_ctx->view_size.x - scroll_ctx->props.bar_width;
        size.y = scroll_ctx->props.bar_width;
      }
      else if (scroll_ctx->can_scroll_x())
      {
        size.x = scroll_ctx->view_size.x;
        size.y = scroll_ctx->props.bar_width;
      }
    }
    return size;
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    Vec2 view_offset = scroll_ctx->view_offset.resolve(
        scroll_ctx->content_size - scroll_ctx->view_size);
    if (direction == Direction::H && scroll_ctx->can_scroll_x())
    {
      f32  scale        = area.extent.x / scroll_ctx->content_size.x;
      f32  thumb_width  = scroll_ctx->view_size.x * scale;
      f32  thumb_offset = view_offset.x * scale;
      Rect thumb_rect   = area;
      thumb_rect.offset.x += thumb_offset;
      thumb_rect.extent.x = thumb_width;
      canvas
          .draw_rect_filled(thumb_rect.offset, thumb_rect.extent,
                            scroll_ctx->props.thumb_color)
          .draw_rect_filled(area.offset, area.extent,
                            scroll_ctx->props.track_color);
    }
    else if (direction == Direction::V && scroll_ctx->can_scroll_y())
    {
      f32  scale        = area.extent.y / scroll_ctx->content_size.y;
      f32  thumb_height = scroll_ctx->view_size.y * scale;
      f32  thumb_offset = view_offset.y * scale;
      Rect thumb_rect   = area;
      thumb_rect.offset.y += thumb_offset;
      thumb_rect.extent.y = thumb_height;
      canvas
          .draw_rect_filled(thumb_rect.offset, thumb_rect.extent,
                            scroll_ctx->props.thumb_color)
          .draw_rect_filled(area.offset, area.extent,
                            scroll_ctx->props.track_color);
    }
  }

  virtual bool hit_test(Context &ctx, Vec2 mouse_position) override
  {
    return true;
  }

  virtual stx::Option<DragData> on_drag_start(Context &ctx,
                                              Vec2     mouse_position) override
  {
    if (direction == Direction::H)
    {
      f32 offset = (mouse_position.x - area.offset.x) / area.extent.x *
                   scroll_ctx->content_size.x;
      scroll_ctx->view_offset.x = Constraint::absolute(offset);
    }
    else
    {
      f32 offset = (mouse_position.y - area.offset.y) / area.extent.y *
                   scroll_ctx->content_size.y;
      scroll_ctx->view_offset.y = Constraint::absolute(offset);
    }
    return stx::Some(DragData{});
  }

  virtual void on_drag_update(Context &ctx, Vec2 mouse_position,
                              Vec2            translation,
                              DragData const &drag_data) override
  {
    if (direction == Direction::H)
    {
      f32 offset = (mouse_position.x - area.offset.x) / area.extent.x *
                   scroll_ctx->content_size.x;
      scroll_ctx->view_offset.x = Constraint::absolute(offset);
    }
    else
    {
      f32 offset = (mouse_position.y - area.offset.y) / area.extent.y *
                   scroll_ctx->content_size.y;
      scroll_ctx->view_offset.y = Constraint::absolute(offset);
    }
  }

  Direction            direction = Direction::V;
  stx::Rc<ScrollCtx *> scroll_ctx;
};

struct ScrollViewport : public Widget
{
  template <Impl<Widget> DerivedWidget>
  ScrollViewport(stx::Rc<ScrollCtx *> ctx, DerivedWidget child) :
      ScrollViewport{std::move(ctx), new DerivedWidget{std::move(child)}}
  {
  }

  ScrollViewport(stx::Rc<ScrollCtx *> ictx, Widget *child) :
      scroll_ctx{std::move(ictx)}
  {
    children.push_inplace(child).unwrap();
  }

  STX_DISABLE_COPY(ScrollViewport)
  STX_DEFAULT_MOVE(ScrollViewport)

  virtual void allocate_size(Context &ctx, Vec2 allocated_size,
                             stx::Span<Vec2> children_allocation) override
  {
    children_allocation.fill(scroll_ctx->props.frame.resolve(allocated_size));
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
  {
    Vec2 view_size           = scroll_ctx->props.frame.resolve(allocated_size);
    Vec2 content_size        = children_sizes[0];
    bool x_scrollable        = content_size.x > view_size.x;
    bool y_scrollable        = content_size.y > view_size.y;
    view_size.x              = x_scrollable ? view_size.x : content_size.x;
    view_size.y              = y_scrollable ? view_size.y : content_size.y;
    scroll_ctx->view_size    = view_size;
    scroll_ctx->content_size = content_size;
    Vec2 view_translation =
        0.0f - scroll_ctx->view_offset.resolve(content_size - view_size);
    children_positions.fill(view_translation);
    return view_size;
  }

  virtual Rect clip(Context &ctx, Rect allocated_clip,
                    stx::Span<Rect> children_allocation) override
  {
    children_allocation.fill(area.intersect(allocated_clip));
    return area;
  }

  virtual stx::Span<Widget *const> get_children(Context &ctx) override
  {
    return children;
  }

  virtual ~ScrollViewport() override
  {
    for (Widget *child : children)
    {
      delete child;
    }
  }

  template <Impl<Widget> DerivedWidget>
  void update_child(DerivedWidget widget)
  {
    ASH_CHECK(children.size() == 1);
    delete children[0];
    children[0] = new DerivedWidget{std::move(widget)};
  }

  void update_child(Widget *widget)
  {
    ASH_CHECK(children.size() == 1);
    delete children[0];
    children[0] = widget;
  }

  stx::Vec<Widget *>   children;
  stx::Rc<ScrollCtx *> scroll_ctx;
};

struct ScrollBox : public Widget
{
  template <Impl<Widget> DerivedWidget>
  ScrollBox(ScrollBoxProps iprops, DerivedWidget child) :
      ScrollBox{std::move(iprops), new DerivedWidget{std::move(child)}}
  {
  }

  ScrollBox(ScrollBoxProps iprops, Widget *child) :
      scroll_ctx{
          stx::rc::make(stx::os_allocator, ScrollCtx{.props = iprops}).unwrap()}
  {
    children.push(new ScrollViewport{scroll_ctx.share(), child}).unwrap();
    children.push(new ScrollBar{Direction::H, scroll_ctx.share()}).unwrap();
    children.push(new ScrollBar{Direction::V, scroll_ctx.share()}).unwrap();
    scroll_ctx->view_offset = iprops.view_offset;
  }

  STX_DISABLE_COPY(ScrollBox)
  STX_DEFAULT_MOVE(ScrollBox)

  virtual void allocate_size(Context &ctx, Vec2 allocated_size,
                             stx::Span<Vec2> children_allocation) override
  {
    children_allocation.fill(allocated_size);
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
  {
    children_positions[0] = Vec2{0, 0};
    children_positions[1] =
        Vec2{0, scroll_ctx->view_size.y - scroll_ctx->props.bar_width};
    children_positions[2] =
        Vec2{scroll_ctx->view_size.x - scroll_ctx->props.bar_width, 0};
    return children_sizes[0];
  }

  virtual i32 z_stack(Context &ctx, i32 allocated_z_index,
                      stx::Span<i32> children_allocation) override
  {
    children_allocation[0] = allocated_z_index + 1;
    children_allocation[1] = allocated_z_index + 1 + 256 * 256;
    children_allocation[2] = allocated_z_index + 1 + 256 * 256;
    return allocated_z_index;
  }

  virtual stx::Span<Widget *const> get_children(Context &ctx) override
  {
    return children;
  }

  virtual ~ScrollBox() override
  {
    for (Widget *child : children)
    {
      delete child;
    }
  }

  template <Impl<Widget> DerivedWidget>
  void update_child(DerivedWidget widget)
  {
    update_child(new DerivedWidget{std::move(widget)});
  }

  void update_child(Widget *widget)
  {
    delete children[0];
    children[0] = new ScrollViewport{scroll_ctx.share(), widget};
  }

  stx::Vec<Widget *>   children;
  stx::Rc<ScrollCtx *> scroll_ctx;
};

};        // namespace gui
}        // namespace ash
