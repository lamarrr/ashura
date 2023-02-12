#pragma once

#include <string>

#include "ashura/widget.h"
#include "fmt/format.h"
#include "stx/vec.h"

namespace ash {

struct FlexProps {
  Direction direction = Direction::Row;

  Wrap wrap = Wrap::Wrap;

  MainAlign main_align = MainAlign::Start;

  CrossAlign cross_align = CrossAlign::Start;

  Fit main_fit = Fit::Shrink;

  Fit cross_fit = Fit::Shrink;

  constraint width;

  constraint height;
};

struct Flex : public Widget {
  Flex(FlexProps aprops, std::initializer_list<WidgetImpl> achildren)
      : props{aprops} {
    children.extend(achildren).unwrap();
  }

  virtual ~Flex() override {
    for (WidgetImpl child : children) {
      delete child.impl;
    }
  }

  void update_children(stx::Span<WidgetImpl const> new_children) {
    for (WidgetImpl child : children) {
      delete child.impl;
    }

    children.clear();
    children.extend(new_children).unwrap();
  }

  virtual stx::Span<WidgetImpl const> get_children() override {
    return children;
  }

  virtual WidgetInfo get_debug_info() override {}

  virtual rect layout(rect target, stx::Span<rect> children_allocation) {
    for (WidgetImpl child : children) {
      // child->layout(        )
    }
    return {};
  }

  virtual simdjson::dom::element save(simdjson::dom::parser& parser) {
    std::string data = fmt::format(
        R"({
    direction : {},
    wrap: {},
    main_align: {},
    cross_align: {},
    main_fit: {},
    cross_fit: {},
    width_bias: {},
    width_scale: {},
    width_min: {},
    width_max: {},
    width_min_rel: {},
    width_max_rel: {},
    height_bias: {},
    height_scale: {},
    height_min: {},
    height_max: {},
    height_min_rel: {},
    height_max_rel: {}})",
        stx::enum_uv(props.direction), stx::enum_uv(props.wrap),
        stx::enum_uv(props.main_align), stx::enum_uv(props.cross_align),
        stx::enum_uv(props.main_fit), stx::enum_uv(props.cross_fit),
        props.width.bias, props.width.scale, props.width.min, props.width.max,
        props.width.min_rel, props.width.max_rel, props.height.bias,
        props.height.scale, props.height.min, props.height.max,
        props.height.min_rel, props.height.max_rel);

    return parser.parse(data.data(), data.size());
  }

  //
  virtual void restore(simdjson::dom::element const& element) {
    props.direction = AS(Direction, AS_U8(element["direction"].get_uint64()));
    props.wrap = AS(Wrap, AS_U8(element["wrap"].get_uint64()));
    props.main_align = AS(MainAlign, AS_U8(element["main_align"].get_uint64()));
    props.cross_align =
        AS(CrossAlign, AS_U8(element["cross_align"].get_uint64()));
    props.main_fit = AS(Fit, AS_U8(element["main_fit"].get_uint64()));
    props.cross_fit = AS(Fit, AS_U8(element["cross_fit"].get_uint64()));
    props.width.bias = AS_F32(element["width_bias"].get_double());
    props.width.scale = AS_F32(element["width_scale"].get_double());
    props.width.min = AS_F32(element["width_min"].get_double());
    props.width.max = AS_F32(element["width_max"].get_double());
    props.width.min_rel = AS_F32(element["width_min_rel"].get_double());
    props.width.max_rel = AS_F32(element["width_max_rel"].get_double());
    props.height.bias = AS_F32(element["height_bias"].get_double());
    props.height.scale = AS_F32(element["height_scale"].get_double());
    props.height.min = AS_F32(element["height_min"].get_double());
    props.height.max = AS_F32(element["height_max"].get_double());
    props.height.min_rel = AS_F32(element["height_min_rel"].get_double());
    props.height.max_rel = AS_F32(element["height_max_rel"].get_double());
  }

  FlexProps props;
  stx::Vec<WidgetImpl> children{stx::os_allocator};
};

}  // namespace ash
