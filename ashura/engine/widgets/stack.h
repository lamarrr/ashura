#pragma once

#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{
namespace gui
{

struct StackProps
{
  Vec2         alignment;
  Constraint2D frame = Constraint2D::relative(1, 1);
};

struct Stack : public Widget
{
  template <Impl<Widget>... DerivedWidget>
  explicit Stack(StackProps iprops, DerivedWidget... ichildren) :
      props{std::move(iprops)}
  {
    update_children(std::move(ichildren)...);
  }

  STX_DISABLE_COPY(Stack)
  STX_DEFAULT_MOVE(Stack)

  virtual ~Stack() override
  {
    for (Widget *child : children)
    {
      delete child;
    }
  }

  template <Impl<Widget>... DerivedWidget>
  void update_children(DerivedWidget... new_children)
  {
    for (Widget *child : children)
    {
      delete child;
    }
    children.clear();
    (children.push(new DerivedWidget{std::move(new_children)}).unwrap(), ...);
  }

  /// takes ownership of the children
  void update_children(Span<Widget *const> new_children)
  {
    for (Widget *child : children)
    {
      delete child;
    }

    children.clear();
    children.extend(new_children).unwrap();
  }

  virtual Span<Widget *const> get_children(Context &ctx) override
  {
    return children;
  }

  virtual WidgetDebugInfo get_debug_info(Context &ctx) override
  {
    return WidgetDebugInfo{.type = "Stack"};
  }

  virtual void allocate_size(Context &ctx, Vec2 allocated_size,
                             Span<Vec2> children_allocation) override
  {
    children_allocation.fill(props.frame.resolve(allocated_size));
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   Span<Vec2 const> children_allocations,
                   Span<Vec2 const> children_sizes,
                   Span<Vec2>       children_positions) override
  {
    Vec2 size;

    for (Vec2 child_size : children_sizes)
    {
      size.x = max(size.x, child_size.x);
      size.y = max(size.y, child_size.y);
    }

    size = props.frame.resolve(size);

    for (usize i = 0; i < children_positions.size(); i++)
    {
      Vec2 extent           = size - children_sizes[i];
      children_positions[i] = props.alignment * extent;
    }

    return size;
  }

  virtual i32 z_stack(Context &ctx, i32 allocated_z_index,
                      Span<i32> children_allocation) override
  {
    i32 next_z_index = allocated_z_index + 1;

    for (i32 &z_index : children_allocation)
    {
      z_index = next_z_index;
      next_z_index += 256;
    }

    return allocated_z_index;
  }

  StackProps    props;
  Vec<Widget *> children;
};

}        // namespace gui
}        // namespace ash