#pragma once

#include "ashura/color.h"
#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"

namespace ash
{
namespace gui
{

enum class BoxCornerShape : u8
{
  Round,
  Bevel
};

struct BoxProps
{
  Vec4                     background_color;
  gfx::LinearColorGradient background_gradient;
  EdgeInsets               padding;
  f32                      border_thickness = 0;
  Vec4                     border_color     = {0, 0, 0, 1};
  BorderRadius             border_radius    = BorderRadius::relative(0);
  BoxCornerShape           corner_shape     = BoxCornerShape::Round;
  Constraint2D             frame            = Constraint2D::relative(1, 1);
};

struct Box : public Widget
{
  template <Impl<Widget> DerivedWidget>
  Box(BoxProps iprops, DerivedWidget child) : props{iprops}
  {
    children.push(new DerivedWidget{std::move(child)}).unwrap();
  }

  Box(BoxProps iprops, Widget *child) : props{iprops}
  {
    children.push_inplace(child).unwrap();
  }

  explicit Box(BoxProps iprops) : props{iprops}
  {
  }

  STX_DISABLE_COPY(Box)
  STX_DEFAULT_MOVE(Box)

  virtual void allocate_size(Context &ctx, Vec2 allocated_size,
                             stx::Span<Vec2> children_allocation) override
  {
    Vec2 const box_size   = props.border_thickness * 2 + props.padding.xy();
    Vec2       child_size = allocated_size - box_size;
    child_size.x          = op::max(child_size.x, 0.0f);
    child_size.y          = op::max(child_size.y, 0.0f);
    children_allocation.fill(child_size);
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
  {
    if (children.size() > 0)
    {
      children_positions[0] =
          props.border_thickness + Vec2{props.padding.left, props.padding.top};
    }
    return props.frame.resolve(
        props.border_thickness * 2 + props.padding.xy() +
        (children.size() > 0 ? children_sizes[0] : Vec2{0, 0}));
  }

  virtual stx::Span<Widget *const> get_children(Context &ctx) override
  {
    return children;
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    Vec4 const border_radius = props.border_radius.resolve(area.extent);
    if (props.background_color.w > 0 ||
        ((!props.background_gradient.is_uniform()) &&
         (props.background_gradient.begin.w > 0 ||
          props.background_gradient.end.w > 0)))
    {
      Rect inner_area;
      inner_area.offset = area.offset + props.border_thickness * 0.88f;
      inner_area.extent = area.extent - props.border_thickness * 0.88f * 2;
      if (props.corner_shape == BoxCornerShape::Round)
      {
        canvas.draw_round_rect_filled(
            inner_area.offset, inner_area.extent, border_radius, 360,
            props.background_color, props.background_gradient);
      }
      else
      {
        canvas.draw_bevel_rect_filled(inner_area.offset, inner_area.extent,
                                      border_radius, props.background_color,
                                      props.background_gradient);
      }
    }

    if (props.border_color.w > 0 && props.border_thickness > 0)
    {
      if (props.corner_shape == BoxCornerShape::Round)
      {
        canvas.draw_round_rect_stroke(area.offset, area.extent, border_radius,
                                      360, props.border_color,
                                      props.border_thickness);
      }
      else
      {
        canvas.draw_bevel_rect_stroke(area.offset, area.extent, border_radius,
                                      props.border_color,
                                      props.border_thickness);
      }
    }
  }

  virtual ~Box() override
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
    ASH_CHECK(children.size() == 1);
    delete children[0];
    children[0] = widget;
  }

  stx::Vec<Widget *> children;
  BoxProps           props;
};
}        // namespace gui
};        // namespace ash