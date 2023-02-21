#pragma once

#include <algorithm>
#include <string>
#include <utility>

#include "ashura/widget.h"
#include "fmt/format.h"
#include "stx/vec.h"

namespace ash {

struct Flex : public Widget {
  template <typename... DerivedWidget>
  Flex(FlexProps iprops, DerivedWidget... ichildren) : props{iprops} {
    (children.push(new DerivedWidget{std::move(ichildren)}).unwrap(), ...);
  }

  // TODO(lamarrr): add span overload

  virtual ~Flex() override {
    for (Widget* child : children) {
      delete child;
    }
  }

  // update_children(DerivedWidget...)

  void update_children(stx::Span<Widget* const> new_children) {
    for (Widget* child : children) {
      delete child;
    }

    children.clear();
    children.extend(new_children).unwrap();
  }

  virtual stx::Span<Widget* const> get_children() override { return children; }

  virtual WidgetInfo get_info() override {
    return WidgetInfo{.type = "Flex", .id = Widget::id};
  }

  virtual Layout layout(rect area) {
    return Layout{
        .flex = props,
        .area = rect{.offset = area.offset,
                     .extent = vec2{props.width.resolve(area.extent.x),
                                    props.height.resolve(area.extent.y)}}};
  }

  virtual simdjson::dom::element save(WidgetContext & context, simdjson::dom::parser& parser) {
    stx::Vec<u64> children_ids{stx::os_allocator};
    for (Widget* child : children) {
      children_ids.push_inplace(child->id).unwrap();
    }

    WidgetInfo info = Widget::get_info();

    std::string json = fmt::format(
        fmt::runtime(R"({{"id": {},
        "type": "{}",
    "direction" : {},
    "wrap": {},
    "main_align": {},
    "cross_align": {},
    "main_fit": {},
    "cross_fit": {},
    "width_bias": {},
    "width_scale": {},
    "width_min": {},
    "width_max": {},
    "width_min_rel": {},
    "width_max_rel": {},
    "height_bias": {},
    "height_scale": {},
    "height_min": {},
    "height_max": {},
    "height_min_rel": {},
    "height_max_rel": {},
    "children": [{}]}})"),
        info.id, info.type, AS_U32(props.direction), AS_U32(props.wrap),
        AS_U32(props.main_align), AS_U32(props.cross_align),
        AS_U32(props.main_fit), AS_U32(props.cross_fit), props.width.bias,
        props.width.scale, props.width.min, props.width.max,
        props.width.min_rel, props.width.max_rel, props.height.bias,
        props.height.scale, props.height.min, props.height.max,
        props.height.min_rel, props.height.max_rel,
        fmt::join(children_ids, ", "));

    return parser.parse(json.data(), json.size());
  }

  virtual void restore(WidgetContext & context,simdjson::dom::element const& element) {
    children.clear();

    Widget::id = element["id"].get_uint64();
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
    simdjson::dom::array children_array = element["children"].get_array();

    for (simdjson::dom::array::iterator it = children_array.begin();
         it < children_array.end(); it++) {
      simdjson::dom::element child = *it;
      child.get_uint64();
    }
  }

  FlexProps props;
  stx::Vec<Widget*> children{stx::os_allocator};
};

}  // namespace ash
