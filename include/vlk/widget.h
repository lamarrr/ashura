#include <chrono>
#include <cinttypes>
#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "stx/option.h"
#include "vlk/image.h"
#include "vlk/utils.h"

namespace vlk {

// Used for PIMPL
struct CanvasBackend2D {};

// might need caching for resources?
struct CanvasRecorder2D {};

struct ScreenState {
  uint32_t width;
  uint32_t height;

  float aspect_ratio() const;
};

struct Context2D {
  ScreenState screen_state;
};

struct Coordinates2D {
  float data[2];

  float x() const;
  float y() const;

  void set_x();
  void set_y();
};

struct Coordinates3D {
  float data[3];

  float x() const;
  float y() const;
  float z() const;

  void set_x();
  void set_y();
  void set_z();
};

struct AutoExtent2D {
  stx::Option<uint32_t> width;   // auto-fit if None
  stx::Option<uint32_t> height;  // auto-fit if None
};

struct Color {
  uint32_t rgba;
  static constexpr Color FromRgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint32_t pixel = static_cast<uint32_t>(r) << 24 ||
                     static_cast<uint32_t>(g) << 16 ||
                     static_cast<uint32_t>(b) << 8 || a;

    return Color{pixel};
  }

  static constexpr Color FromRgb(uint8_t r, uint8_t g, uint8_t b) {
    return Color::FromRgba(r, g, b, 0xFFU);
  }

  constexpr Color with_red(uint8_t) const noexcept;
  constexpr Color with_green(uint8_t) const noexcept;
  constexpr Color with_blue(uint8_t) const noexcept;
  constexpr Color with_alpha(uint8_t a) const noexcept {
    return Color{(rgba & 0xFFFFFF00U) | a};
  }
};

namespace colors {
static constexpr auto Red = Color::FromRgb(0xFF, 0x00, 0x00);
static constexpr auto White = Color::FromRgb(0xFF, 0xFF, 0xFF);
static constexpr auto Black = Color::FromRgb(0x00, 0x00, 0x00);
static constexpr auto Blue = Color::FromRgb(0x00, 0x00, 0xFF);
static constexpr auto Green = Color::FromRgb(0x00, 0xFF, 0x00);
}  // namespace colors

// to be inherited from
struct TopBottomLeftRight {
  uint32_t top = 0, bottom = 0, right = 0, left = 0;
  static TopBottomLeftRight uniform(uint32_t);
  static TopBottomLeftRight xy(uint32_t, uint32_t);
  static TopBottomLeftRight tblr(uint32_t, uint32_t, uint32_t, uint32_t);
};

struct Border : public TopBottomLeftRight {
  using TopBottomLeftRight::TopBottomLeftRight;
};

struct Padding : public TopBottomLeftRight {
  using TopBottomLeftRight::TopBottomLeftRight;
};

struct Margin : public TopBottomLeftRight {
  using TopBottomLeftRight::TopBottomLeftRight;
};

struct BackgroundImage {
  desc::Image2D image;
  Coordinates2D coordinates;
  uint8_t opacity;  // uint8?
};

struct BoxModel {
  std::variant<Color, BackgroundImage> background;
  Border border;
  AutoExtent2D extent;
  Padding padding;
  Margin margin;
};

// namespace ui

namespace impl {
// how to iterate over array of widgets?
// we are hereby avoiding unneeded hierachies via this proxy
struct WidgetEventProxy {
  template <typename... ParamTypes>
  using EventHandler = void (*)(void *, ParamTypes...);

  using TickHandler = EventHandler<std::chrono::nanoseconds const &>;
  using DrawHandler = EventHandler<Context2D const &, CanvasRecorder2D &>;
  using Handler = EventHandler<>;
  using QueryHandler = bool (*)(void *);

 public:
  WidgetEventProxy(void *widget, bool is_stateful)
      : widget_{widget}, is_stateful_{is_stateful} {}

  TickHandler on_tick;
  DrawHandler on_draw;

  Handler on_click;
  Handler on_hover;
  Handler on_enter_view;
  Handler on_leave_view;

  QueryHandler is_dirty;

  void *widget() const noexcept { return widget_; }

  bool is_stateful() const noexcept { return is_stateful_; }

 private:
  void *widget_;
  bool is_stateful_;
};

/************ These widget graphs will always be held in memory because
 * WidgetProxy will be referencing them *********/

// The widgets will be stored as a pointer to this type, this will enable the
// widget's appropriate destructor to be called
struct WidgetDestructorProxy {
  virtual ~WidgetDestructorProxy() noexcept {}
};

// For feeding the widgets events
struct WidgetEventBase {
  void on_click() {}       // may or may not be defined
  void on_hover() {}       // may or may not be defined
  void on_enter_view() {}  // may or may not be defined
  void on_leave_view() {}  // may or may not be defined
};

struct WidgetDrawBase {
  void draw([[maybe_unused]] Context2D const &context,
            [[maybe_unused]] CanvasRecorder2D &canvas) {
    // no-op
  }
};

struct WidgetTickBase {
  // for UI related processing, all other-processing should be done
  // asynchronously
  void tick([[maybe_unused]] std::chrono::nanoseconds const &interval) {
    // no-op
  }
};

struct WidgetStatefulnessBase {
  bool is_dirty() { return true; }
};

}  // namespace impl

// virtual function calls here are not actually used, but helps to guide the
// user, the only virtual function call used here is the destructor
struct StatelessWidget : public impl::WidgetDestructorProxy,
                         public impl::WidgetDrawBase,
                         public impl::WidgetEventBase,
                         public impl::WidgetTickBase {
  virtual ~StatelessWidget() noexcept override {}
};

struct StatefulWidget : public impl::WidgetDestructorProxy,
                        public impl::WidgetDrawBase,
                        public impl::WidgetEventBase,
                        public impl::WidgetTickBase,
                        public impl::WidgetStatefulnessBase {
  virtual ~StatefulWidget() noexcept override {}
};

struct WidgetGraph {
  std::vector<impl::WidgetDestructorProxy *> children_;
  std::vector<impl::WidgetEventProxy> event_proxies_;

  // The WidgetType determines how the function methods will be used, since we
  // are constructing it ourself and not using an external reference, we can
  // take shortcuts on the inheritance hierarchy, this will help prevent
  // excessive vtable look-up in the rendering code. we know the exact type of
  // the widget hence we only pay at compile time with many functions.
  template <
      typename WidgetType,
      std::enable_if_t<std::is_base_of_v<StatefulWidget, WidgetType>, int> = 0,
      typename... ConstructorArguments>
  WidgetGraph &add_child(ConstructorArguments &&... arguments) & {
    static_assert(!std::is_reference_v<WidgetType>);
    static_assert(!std::is_const_v<WidgetType>);
    // std::unique_ptr because std::vector::push_back can throw and a
    // memory leak would occur if raw memory
    std::unique_ptr<WidgetType> widget{
        new WidgetType{std::forward<ConstructorArguments &&>(arguments)...}};

    impl::WidgetEventProxy proxy{widget.get(), true};

    proxy.on_tick = [](void *widget_ptr,
                       std::chrono::nanoseconds const &interval) {
      auto widget = reinterpret_cast<WidgetType *>(widget_ptr);
      widget->tick(interval);
    };

    proxy.on_draw = [](void *widget_ptr, Context2D const &context,
                       CanvasRecorder2D &canvas) {
      auto widget = reinterpret_cast<WidgetType *>(widget_ptr);
      widget->draw(context, canvas);
    };

    proxy.on_click = [](void *widget_ptr) {
      auto widget = reinterpret_cast<WidgetType *>(widget_ptr);
      widget->on_click();
    };

    proxy.on_hover = [](void *widget_ptr) {
      auto widget = reinterpret_cast<WidgetType *>(widget_ptr);
      widget->on_hover();
    };

    proxy.on_enter_view = [](void *widget_ptr) {
      auto widget = reinterpret_cast<WidgetType *>(widget_ptr);
      widget->on_enter_view();
    };

    proxy.on_leave_view = [](void *widget_ptr) {
      auto widget = reinterpret_cast<WidgetType *>(widget_ptr);
      widget->on_leave_view();
    };

    proxy.is_dirty = [](void *widget_ptr) {
      auto widget = reinterpret_cast<WidgetType *>(widget_ptr);
      return widget->is_dirty();
    };

    children_.push_back(widget.get());
    event_proxies_.push_back(proxy);

    widget.release();

    return *this;
  }

  // FIX
  WidgetGraph &add_child() && = delete;

  ~WidgetGraph() {
    for (auto *child : children_) {
      // uses the WidgetDestructorProxy
      delete child;
    }
  }
};

struct Button : StatefulWidget {};

// dynamic library to get ui description and doesn't contain the actual contents
// of the engine itself. The engine is always loaded but gets ui description
// from DLL. It's okay to use virtual inheritance here since it is not in a
// real-time loop and it only gets the UI desription

// the whole 2d ui performs only one flushandsubmit call to skia

}  // namespace vlk

// Same goes for 3D?

extern "C" void *vlk_get_func(char const *name) {
  // get widgets description
  // get whole 2d context information required for the engine
  return {};
}
