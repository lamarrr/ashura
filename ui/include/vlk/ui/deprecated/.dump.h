/*
struct WidgetGraph {
  std::vector<impl::WidgetLifetimeProxy *> widgets_;
  std::vector<bool> statefulness_;

  // The WidgetType determines how the function methods will be used, since we
  // are constructing it ourself and not using an external reference, we can
  // take shortcuts on the inheritance hierarchy, this will help prevent
  // excessive vtable look-up in the rendering code. we know the exact type of
  // the widget hence we only pay at compile time with many functions.
  template <typename WidgetType,
            std::enable_if_t<std::is_base_of_v<StatefulWidget, WidgetType> ||
                                 std::is_base_of_v<StatelessWidget, WidgetType>,
                             int> = 0,
            typename... ConstructorArguments>
  WidgetGraph &add_widget(ConstructorArguments &&... arguments) & {
    static_assert(!std::is_reference_v<WidgetType>);
    static_assert(!std::is_const_v<WidgetType>);
    // std::unique_ptr because std::vector::push_back can throw and a
    // memory leak would occur if raw memory
    std::unique_ptr<WidgetType> widget{
        new WidgetType{std::forward<ConstructorArguments &&>(arguments)...}};

    constexpr bool stateful = std::is_base_of_v<StatefulWidget, WidgetType>;

    widgets_.push_back(widget.get());
    statefulness_.push_back(stateful);

    widget.release();

    return *this;
  }

  template <typename WidgetType,
            std::enable_if_t<std::is_base_of_v<StatefulWidget, WidgetType> ||
                                 std::is_base_of_v<StatelessWidget, WidgetType>,
                             int> = 0,
            typename... ConstructorArguments>
  WidgetGraph &add_widget(ConstructorArguments &&... arguments) && = delete;

  ~WidgetGraph() {
    for (auto *child : widgets_) {
      // uses the WidgetLifetimeProxy
      delete child;
    }
  }
};
*/

#if VLK_ENABLE_DEBUG_CHECKS
#define VLK_DEBUG_ENSURE(expr, ...) VLK_ENSURE(expr, __VA_ARGS__)
#else
#define VLK_DEBUG_ENSURE(expr, ...)
#endif

struct Coordinates3D {
  float data[3];

  constexpr float x() const noexcept;
  constexpr float y() const noexcept;
  constexpr float z() const noexcept;

  constexpr void set_x(float) noexcept;
  constexpr void set_y(float) noexcept;
  constexpr void set_z(float) noexcept;
};

#if VLK_ENABLE_DEBUG_CHECKS
VLK_ENSURE(widget != nullptr, "Widget is nullptr");
auto *container = dynamic_cast<WidgetChildrenProxy *>(widget);
VLK_ENSURE(container != nullptr, "Invalid Dynamic Cast");
#else
auto *container = reinterpret_cast<WidgetChildrenProxy *>(widget);
#endif
stx::Span children = container->get_children();
#if VLK_ENABLE_DEBUG_CHECKS
VLK_ENSURE(std::any_of(children.begin(), children.end(),
                       [](NewWidgetProxy *widget) {
                         return widget->is_stateful();
                       }));
#endif

if (widget->is_stateful() &&
    stateful_cache_.entries() < maximum_stateful_cache_entries_) {
  stateful_cache_.push_back(widget);
} else if (!widget->is_stateful() &&
           stateless_cache_.entries() <= maximum_stateless_cache_entries_) {
  stateless_residuals_.push_back(widget);
} else if (widget->is_stateful()) {
  if (widget->has_children()) {
    WidgetChildrenProxy *proxy =
        reinterpret_cast<WidgetChildrenProxy *>(widget);
    occupy_cache_(proxy->get_children());
  } else {
    stateful_cache_.add_entry(std::move(stub_entry));
  }
} else {
  stateless_cache_.add_entry(std::move(stub_entry));
}
}

/* stx::Option<stx::Ref<CacheEntry>> get_widget_entry(
NewWidget *const widget) const noexcept {
xTRACE_SCOPE;
auto pos = std::find_if(
 base::begin(), base::end(),
 [widget](CacheEntry const &entry) { return entry.widget == widget;
}); bool const hit = pos == base::end(); xTRACE_CACHE(hit); return hit ?
stx::Option(stx::some_ref(pos->snapshot)) : stx::None;
}
*/

struct ListenRegistration {
  bool keyboard;
  bool mouse;
  bool screen;
};

struct AutoExtent {
  stx::Option<uint32_t> width;   // auto-fit if None
  stx::Option<uint32_t> height;  // auto-fit if None
};

struct BoxModel {
  Border border;
  AutoExtent extent;
  Padding padding;
  Margin margin;
};

struct Coordinates {
  Offset begin;
  Offset end;
};

struct RelativeCoordinates {
  RelativeOffset begin;
  RelativeOffset end;
};

// TODO(lamarrr): better allocation behaviour than having new everywhere

// dynamic library to get ui description and doesn't contain the actual contents
// of the engine itself. The engine is always loaded but gets ui description
// from DLL. It's okay to use virtual inheritance here since it is not in a
// real-time loop and it only gets the UI desription

// the whole 2d ui performs only one flushandsubmit call to skia

/*
template <typename ChildDeleter = DefaultChildDeleter,
          typename PointerAllocator = std::allocator<Widget *>>
struct BasicFloatingArea
    : public impl::WrapperWidgetBase<ChildDeleter, PointerAllocator> {
  // floating widgets will disrupt the whole widget cache, and require constant
  // re-rendering
};

using FloatingArea = BasicFloatingArea<>;
*/

// Dynamic Stack
// struct Stack : public impl::ContainerWidgetBase {
// private:
//  std::vector<size_t>
//      z_indexes_;  // determines which renders before which when overlapping
// };

// Same goes for 3D?
// DLL loading?

// struct FontProviderProxy {};
//
// struct FontProvider : public FontProviderProxy {};
// Widget builder with custom deleter?
//

// struct EventHandler {};

// struct ListenerWidget {
//  virtual ListenRegistration register_for_listening() {}
//};

// spin up separate thread for event handling? in with queue to process events
// SPSC queue

// struct MouseEventListener {};
// struct KeyBoardEventListener {};

// Align, Center, Widgets etc.
// template <typename ChildDeleter = DefaultChildDeleter,
//          typename PointerAllocator = std::allocator<Widget *>>
// struct WrapperWidgetBase
//    : public ContainerWidgetBase<ChildDeleter, PointerAllocator> {
//  explicit WrapperWidgetBase(Widget *const widget)
//      : ContainerWidgetBase<ChildDeleter, PointerAllocator>({widget}) {
//    VLK_ENSURE(widget != nullptr, "gotten nullptr child widget");
//  }
// };

//
virtual Rect get_area(
    Extent const &allotted_extent,
    [[maybe_unused]] std::function<stx::Option<Extent>(Widget *const)> const
        &occupied_extent_cache) const {
  Rect area{};
  area.offset.x = 0;
  area.offset.y = 0;
  area.extent = allotted_extent;
  return area;
}

bool has_children() const noexcept { return !get_children().empty(); }

virtual stx::Span<Widget *const> get_children() const { return {}; }

// used for positioning children within this parent widget.
// `requested_extent` is the extent returned by `get_area`.
// the parent widget will provide the area the child widget should occupy
// within itself. `children_area.size()` is always equal to
// `get_children().size()`
virtual void get_children_area(
    [[maybe_unused]] Extent const &requested_extent,
    [[maybe_unused]] stx::Span<Rect> const &children_area,
    [[maybe_unused]] std::function<stx::Option<Extent>(Widget *const)> const
        &occupied_extent_cache) const {
  // no-op, all zeroed out by default.
}

// STX_FORCE_INLINE void tick_all_widgets(stx::Span<CacheEntry> const &entries,
//                                       std::chrono::nanoseconds const
//                                       &interval) {
//  xTRACE_SCOPE;
//  for (auto &entry : entries) entry.snapshot.widget()->tick(interval);
//}

/* {
// no-op
SkRect rect =
SkRect::MakeWH(requested_extent.width, requested_extent.height);
SkPaint paint;
paint.setStyle(SkPaint::Style::kFill_Style);
paint.setColor(colors::Red.argb());
auto sk_canvas = canvas.as_skia();
sk_canvas->drawRect(rect, paint);
}*/

template <typename ChildDeleter = DefaultWidgetChildDeleter,
          typename PointerAllocator = std::allocator<Widget *>>
struct BasicColumn
    : public PositioningWidgetBase<false, ChildDeleter, PointerAllocator> {
  using base = PositioningWidgetBase<false, ChildDeleter, PointerAllocator>;

  using base::base;

  virtual Rect compute_area(
      Extent const &allotted_extent,
      [[maybe_unused]] stx::Span<Rect> const &children_area) const
      noexcept override {
    return Rect{Offset{0, 0}, allotted_extent};
  }

  virtual bool is_dirty() const noexcept override { return false; }
};

using Column = BasicColumn<>;

virtual Rect compute_area(Extent const &allotted_extent,
                          [[maybe_unused]] stx::Span<Rect> const &children_area,
                          std::map<Widget *, Rect> &cache) noexcept override {
  auto this_widget = static_cast<Widget *>(const_cast<BasicColumn *>(this));
  auto pos = cache.find(this_widget);

  Offset offset{0, 0};

  // if (pos == cache.end()) {
  //      VLK_LOG("cache miss");

  auto children = this->get_children();
  uint32_t num_children = children.size();
  // Height property: if the child will use all of the parent's allotted
  // height, allow it. the child widget is however constrained by this
  // widget's width.

  auto max_children_height = std::accumulate(
      children.begin(), children.end(), static_cast<uint32_t>(0),
      [&cache, num_children, allotted_extent](uint32_t max_height,
                                              Widget *child) {
        auto child_extent = Extent{allotted_extent.width / num_children,
                                   allotted_extent.height};

        std::vector<Rect> vec;
        vec.resize(child->get_children().size());
        return std::max(
            max_height,
            child->compute_area(child_extent, vec, cache).extent.height);
      });

  for (uint32_t i = 0; i < num_children; i++) {
    children_area[i].extent.width = allotted_extent.width / num_children;
    children_area[i].extent.height = max_children_height;

    children_area[i].offset.x = children_area[i].extent.width * i;
    children_area[i].offset.y = 0;
  }

  Extent extent{};
  extent.width = allotted_extent.width;
  extent.height = max_children_height;
  Rect area{offset, extent};

  // cache.emplace(this_widget, area);

  return area;

  //} else {
  // the children layout will not be filled since it has previously been
  // filled
  // VLK_LOG("cache hit");
  // return Rect{offset, pos->second.extent};
  //}
}

// VLK_DEBUG_CODE(auto const repeating_child_pos =
//                   std::find_if(widget_surface_area_map.begin(),
//                   widget_surface_area_map.end(),
//                                [child](auto const &entry) { return
//                                entry.first == child; }));
// VLK_DEBUG_CODE(VLK_ENSURE(repeating_child_pos ==
// widget_surface_area_map.end(),
//                          "Found repeating Widget",
//                          repeating_child_pos->first->get_name()));

bool font_found = false;

std::string font_family = properties.font_family();
SkString font{font_family.c_str()};
for (int i = 0; i < font_mgr->countFamilies(); i++) {
  SkString font_family_name;
  font_mgr->getFamilyName(i, &font_family_name);
  if (font_found = font_family_name.equals(font)) break;
}
VLK_ENSURE(font_found, "Unable to find font");




inline void children_flex_layout_no_factor(
    Flex const &flex, Extent const &allotted,
    stx::Span<WidgetLayoutTree::Node> const &child_nodes) {
  switch (flex.direction) {
    case Flex::Direction::Column: {
      switch (flex.wrap) {
        case Flex::Wrap::None: {
          switch (flex.main_align) {
            case Flex::MainAlign::End: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::End, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::End, Flex::CrossAlign::End>(allotted,
                                                                   child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::End, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::End, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceAround: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceBetween: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceEvenly: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::Start: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::Start, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::Start, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::Start, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::None,
                      Flex::MainAlign::Start, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
          }
        } break;

        case Flex::Wrap::Wrap: {
          switch (flex.main_align) {
            case Flex::MainAlign::End: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::End, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::End, Flex::CrossAlign::End>(allotted,
                                                                   child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::End, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::End, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceAround: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceBetween: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceEvenly: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::Start: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::Start, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::Start, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::Start, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Column, Flex::Wrap::Wrap,
                      Flex::MainAlign::Start, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
          }
        } break;
      }
    } break;

    case Flex::Direction::Row: {
      switch (flex.wrap) {
        case Flex::Wrap::None: {
          switch (flex.main_align) {
            case Flex::MainAlign::End: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::End, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::End, Flex::CrossAlign::End>(allotted,
                                                                   child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::End, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::End, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceAround: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceBetween: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceEvenly: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::Start: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::Start, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::Start, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::Start, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::None,
                      Flex::MainAlign::Start, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
          }
        } break;

        case Flex::Wrap::Wrap: {
          switch (flex.main_align) {
            case Flex::MainAlign::End: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::End, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::End, Flex::CrossAlign::End>(allotted,
                                                                   child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::End, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::End, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceAround: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceAround, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceBetween: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceBetween, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::SpaceEvenly: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::SpaceEvenly, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
            case Flex::MainAlign::Start: {
              switch (flex.cross_align) {
                case Flex::CrossAlign::Center: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::Start, Flex::CrossAlign::Center>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::End: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::Start, Flex::CrossAlign::End>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Start: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::Start, Flex::CrossAlign::Start>(
                      allotted, child_nodes);
                } break;
                case Flex::CrossAlign::Stretch: {
                  children_flex_layout_no_factor_<
                      Flex::Direction::Row, Flex::Wrap::Wrap,
                      Flex::MainAlign::Start, Flex::CrossAlign::Stretch>(
                      allotted, child_nodes);
                } break;
              }
            } break;
          }
        } break;
      }
    } break;
  }
}
