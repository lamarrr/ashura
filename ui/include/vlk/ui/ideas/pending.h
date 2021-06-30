#pragma once

#include <chrono>
#include <string_view>
#include <vector>

#include "stx/source_location.h"

namespace vlk {
namespace ui {

struct EventTraceEntry {
  std::string_view event_name;
  std::chrono::steady_clock::time_point timepoint;
  bool begin;
};

struct ScalarTraceEntry {
  std::string_view scalar_name;
  std::chrono::steady_clock::time_point timepoint;
  double scalar;
};

// trace sink not thread-safe. not required since rendering is always on a
// single thread and it won't be used across multiple threads anyway
// TUDO(lamarrr): use a cycling-iterator with a max extent.
struct TraceSink {
  std::vector<EventTraceEntry> events;
  std::vector<ScalarTraceEntry> scalars;
  std::string_view name;
};

struct ScopeEventTrace {
  ScopeEventTrace(TraceSink &sink,
                  stx::SourceLocation location = stx::SourceLocation::current())
      : sink_{&sink}, event_name_{location.function_name()} {
    sink_->events.push_back(
        EventTraceEntry{event_name_, std::chrono::steady_clock::now(), true});
  }

  ~ScopeEventTrace() {
    sink_->events.push_back(
        EventTraceEntry{event_name_, std::chrono::steady_clock::now(), false});
  }

 private:
  TraceSink *sink_;
  std::string_view event_name_;
};

#define VLK_TRACE_SINK_FUNC_NAME(sink_name) \
  VLK_TRACE_API_get_tracer_trace_sink__##sink_name
#define VLK_DECLARE_TRACE_SINK(sink_name) \
  extern ::vlk::ui::TraceSink &VLK_TRACE_SINK_FUNC_NAME(sink_name)()
#define VLK_DEFINE_TRACE_SINK(sink_name)                               \
  extern ::vlk::ui::TraceSink &VLK_TRACE_SINK_FUNC_NAME(sink_name)() { \
    static ::vlk::ui::TraceSink sink{{}, {}, #sink_name};              \
    return sink;                                                       \
  }

#define VLK_SCOPE_EVENT_TRACE_TO_SINK(sink_name)                                                                       \
  ::vlk::ui::ScopeEventTrace                                                                                           \
      VLK_ScopedEventTrace_must_be_unique_for_sink__##sink_name##__sink_source_per_scope_else_you_are_doing_it_wrong { \
    VLK_TRACE_SINK_FUNC_NAME(sink_name)()                                                                              \
  }

// TUDO(lamarrr): consider tracing to file instead using chrome's tracing format

#define VLK_SCALAR_TRACE_TO_SINK(scalar, sink_name)           \
  do {                                                        \
    double value = static_cast<double>(scalar);               \
    VLK_TRACE_SINK_FUNC_NAME(sink_name)                       \
    ().scalars.push_back(::vlk::ui::ScalarTraceEntry{         \
        #scalar, ::std::chrono::steady_clock::now(), value}); \
  } while (false)

}  // namespace ui
}  // namespace vlk

// TUDO(lamarrr): remove vector and log to file or something else

sk_sp<SkImageFilter> filters[] = {
    nullptr,
    SkImageFilters::DropShadow(7.0f, 0.0f, 0.0f, 3.0f, SK_ColorBLUE, nullptr),
    SkImageFilters::DropShadow(0.0f, 7.0f, 3.0f, 0.0f, SK_ColorBLUE, nullptr),
    SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr),
    SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
                               std::move(cfif)),
    SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr,
                               &cropRect),
    SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr,
                               &bogusRect),
    SkImageFilters::DropShadowOnly(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
                                   nullptr),
};
struct BoxShadow {
  // greater than or equal to 0
  float blur_radius;
  // greater than 0
  float blur_sigma;
};

stx::Span<BoxShadow const> const &shadows = {}

struct Gradient {
  // linear, radial, sweep
};

stx::Span<TextShadow const> const &shadows = {}

struct TextShadow {
  Color color = colors::Black;
  IOffset offset = IOffset{0, 0};
  double blur_radius = 0.0;
};

/*
for (auto const& shadow : shadows)
  text_style.addShadow(sktext::TextShadow{
      shadow.color.to_argb(), SkPoint::Make(shadow.offset.x, shadow.offset.y),
      shadow.blur_radius});

struct ImageEffect {
  // blur etc
  std::unique_ptr<ImageEffect> child;

  static std::unique_ptr<ImageEffect> blur();

  // image filters
  virtual void pre_canvas(Canvas &);
  virtual void post_canvas(Canvas &);

  virtual ~ImageEffect() = 0;
};

struct OpacityEffect {
  float opacity;
};

// a bundle is more or less a subsystem
/*struct Bundle {
  uint64_t cache_limit_bytes = ...;
  alignas(std::hardware_destructive_interference_size)
      std::atomic<uint64_t> cache_bytes = 0;
  struct MetaInfo {
    std::string_view name;
  };
  // std::unique_ptr<std::thread> worker_thread;
  virtual ~Bundle() = 0;
};
*/

// icons are static and must be loaded at once

// TUDO(laamrrr): provide removal of optionals in paremeters. props should
// be trivial

// static image?
// TUDO(lamarrr): size hinting?

// handling resizing and predefined extents and aspect ratios?
// placeholder if defined needs to take a certain position
// use own thread for loading resources, sleep and await new resource adding
// to the queue once all resources are loaded or the Bundle is destroyed
// TUDO(lamarrr): handling null sized images and the likes
/*
struct BundleContext {
  ImageBundle &image_bundle();
};
*/

// async cached image source/provider/spec that is ticked by the global
// asset manager. we can have a global asset rendering registry that ticks
// resources and discards as necessary and increases last access ticks
//
// AspectRatio via trimmer
//

namespace ops {
struct Fused : public Widget {
  Widget *first;
  Widget *second;

  Widget *operator|(Widget const &) { return nullptr; }
};

struct Blend {
  Widget *b;
};

struct Clip {
  struct Shape {};
};

// ease-in ease-out
// fade-in fade-out
// TUDO(lamarrr): we need a widget inhibitor, the translate widget will now
// take inputs on behalf of the Widget type.
// TUDO(lamarrr): i.e. type hint for Opacity widget for Button will be
// "Opacity for Button"
// TUDO(lamarr): Concrete structs for ops and then an Effect widget that takes
// the ops as an argument, we'll be able to avoid the virtual function
// overhead. alsom how do we map fusing them to a concrete type? Fuse<Clip,
// Blend, Translate> {   void draw(){  // for each, draw   };       } output
// of one is passed to another
//  Translate | Rotate => Fused<Translate, Rotate, Clip, Draw>

struct Translate {};
struct Rotate {};

};  // namespace ops

//
//
//
// if(!new_clip_rect.visible())
// widget->mark_left_view()
// if(previous_clip_rect.visibile()) widget->mark_enter_view()

if (flex.main_align != MainAlign::Start) {
  if (flex.direction == Direction::Row) {
    VLK_ENSURE(self_extent.width.max != i64_max,
               "Flex widget with a space-based main-axis alignment (i.e. "
               "MainAlign::End, MainAlign::SpaceBetween, "
               "MainAlign::SpaceAround, or MainAlign::SpaceEvenly) must "
               "have a fixed maximum extent (`Constrain::max`)",
               widget);
  } else {
    VLK_ENSURE(self_extent.height.max != i64_max,
               "Flex widget with a space-based main-axis alignment (i.e. "
               "MainAlign::End, MainAlign::SpaceBetween, "
               "MainAlign::SpaceAround, or MainAlign::SpaceEvenly) must "
               "have a fixed maximum extent (`Constrain::max`)",
               widget);
  }
}


  // this is used for preloading some of the tiles
  // constant throughout lifetime.
  // Extent focus_extension = Extent{0, 0};


  // focusing helps us preload a part of the screen into the tiles
  // rename focus rect and focus extension to something more descriptive
  // also move this function out as we'll need to test it
  /*IRect get_focus_rect() const {
    uint32_t const focus_x = focus_extension.width / 2;
    uint32_t const focus_y = focus_extension.height / 2;

    int64_t const x_min = viewport_scroll_offset.x - focus_x;
    int64_t const y_min = viewport_scroll_offset.y - focus_y;

    return IRect{IOffset{x_min, y_min},
                 Extent{viewport_extent.width + focus_extension.width,
                        viewport_extent.height + focus_extension.height}};
  }
  */

   SkVector const inner_border_radii[] = {
      SkVector::Make(border_radius.top_left + border_width_left,
                     border_radius.top_left + border_width_top),
      SkVector::Make(border_radius.top_right + border_width_right,
                     border_radius.top_right + border_width_top),
      SkVector::Make(border_radius.bottom_left + border_width_left,
                     border_radius.bottom_left + border_width_bottom),
      SkVector::Make(border_radius.bottom_right + border_width_right,
                     border_radius.bottom_right + border_width_bottom),
  };

  if (root_node.type == WidgetType::View) {
        VLK_ENSURE(root_node.view_extent.width < u32_max,
                   "root widget has infinite resolved view width",
                   *root_node.widget);
        VLK_ENSURE(root_node.view_extent.height < u32_max,
                   "root widget has infinite resolved view height",
                   *root_node.widget);
      }

      VLK_ENSURE(root_node.self_extent.width < u32_max,
                 "root widget has infinite resolved self width",
                 *root_node.widget);
      VLK_ENSURE(root_node.self_extent.height < u32_max,
                 "root widget has infinite resolved self height",
                 *root_node.widget);


/*
inline SkColorType win_surface__to_skia(VkFormat format) {
  switch (format) {
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_B8G8R8A8_UNORM:
      return kRGBA_8888_SkColorType;

    default:
      VLK_PANIC("Unsupported VkFormat", format);
  }
}

inline sk_sp<SkColorSpace> win_surface__to_skia(VkColorSpaceKHR color_space) {
  switch (color_space) {
    case VK_COLORSPACE_SRGB_NONLINEAR_KHR:
      return SkColorSpace::MakeSRGB();
    default:
      VLK_PANIC("Unsupported VkColorSpaceKHR");
  }
}
*/

/*
// i.e. disable mouse acceleration if supported
  Raw mouse motion is closer to the actual motion of the mouse across a surface.
It is not affected by the scaling and acceleration applied to the motion of the
desktop cursor. That processing is suitable for a cursor while raw motion is
better for controlling for example a 3D camera. Because of this, raw mouse
motion is only provided when the cursor is disabled.

Call glfwRawMouseMotionSupported to check if the current machine provides raw
motion and set the GLFW_RAW_MOUSE_MOTION input mode to enable it. It is disabled
by default.

if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
If supported, raw mouse motion can be enabled or disabled per-window and at any
time but it will only be provided when the cursor is disabled.
*/

// if previous mouse event was a click without modifieirs and the present
  // one is also a click without modifiers within a fixed time interval, then
  // we consider it a double click
  //
  //
  // how do we merge across refresh?
  //
  //
  //
  //    - proper double click support
  //    - order of processing or dispatching events

  // static constexpr std::chrono::milliseconds double_click_timeout =
  //    std::chrono::milliseconds{250};
 // interleaved?
  // std::chrono::steady_clock::time_point last_mouse_double_click_start;
  // events processed at refresh rate.

  

    glfwSetWindowSizeCallback(handle->window, Window::resize_callback);
    glfwSetFramebufferSizeCallback(handle->window,
                                   Window::surface_resize_callback);

    // glfwGetWindowMonitor
    // we might need to handle disconnection/connection of monitors
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    VLK_ENSURE(primary_monitor != nullptr, "No monitors found");
    GLFWvidmode const* primary_monitor_video_mode =
        glfwGetVideoMode(primary_monitor);
    VLK_ENSURE(primary_monitor_video_mode != nullptr,
               "Unable to get monitor video mode");

    // makes the window into windowed mode if primary_monitor is not null
    // otherwise into non-windowed mode
    // setting should otherwise only be used during monitor setting
    glfwSetWindowMonitor(window, primary_monitor, 0, 0, 1920, 1080,
                         primary_monitor_video_mode->refreshRate);

    // might need to be updated
    VLK_LOG("Monitor is at {}hz", primary_monitor_video_mode->refreshRate);

    handle->cfg = cfg;

    WindowSurface surface;
    surface.handle =
        std::shared_ptr<WindowSurfaceHandle>(new WindowSurfaceHandle{});
    surface.handle->instance = instance;

    // creates and binds the window surface (back buffer, render target
    // surface) to the glfw window
    VLK_MUST_SUCCEED(
        glfwCreateWindowSurface(instance.handle->instance, handle->window,
                                nullptr, &surface.handle->surface),
        "Unable to Create Window Surface");

    handle->surface = std::move(surface);

    return stx::Some(Window{std::move(handle)});
  }

  void publish_events() {
    for (InputPawn* pawn : handle->input_pawns) {
      InputPawnSystemProxy::system_tick(*pawn, handle->event_queue.mouse_events,
                                        handle->event_queue.keyboard_events);
    }
  }

  static void resize_callback(GLFWwindow* window, int width, int height) {
    auto handle = static_cast<WindowHandle*>(glfwGetWindowUserPointer(window));
    handle->resize(
        Extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
  }

  static void surface_resize_callback(GLFWwindow* window, int width,
                                      int height) {
    auto handle = static_cast<WindowHandle*>(glfwGetWindowUserPointer(window));
    handle->resize_surface(
        Extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
  }

  static void cursor_position_callback(GLFWwindow* window, double x, double y) {
    // measured in screen coordinates
  }

  static void mouse_button_callback(GLFWwindow* window, int button, int action,
                                    int modifier) {
    auto handle = static_cast<WindowHandle*>(glfwGetWindowUserPointer(window));

    MouseEvent event;

    switch (button) {
      case GLFW_MOUSE_BUTTON_LEFT:
        event.button = MouseButton::Primary;
        break;
      case GLFW_MOUSE_BUTTON_RIGHT:
        event.button = MouseButton::Secondary;
        break;
      case GLFW_MOUSE_BUTTON_MIDDLE:
        event.button = MouseButton::Middle;
        break;
      case GLFW_MOUSE_BUTTON_4:
        event.button = MouseButton::A1;
        break;
      case GLFW_MOUSE_BUTTON_5:
        event.button = MouseButton::A2;
        break;
      case GLFW_MOUSE_BUTTON_6:
        event.button = MouseButton::A3;
        break;
      case GLFW_MOUSE_BUTTON_7:
        event.button = MouseButton::A4;
        break;
      case GLFW_MOUSE_BUTTON_8:
        event.button = MouseButton::A5;
        break;
      default:
        VLK_PANIC("Unimplemented mouse button", static_cast<int>(button));
    }

    switch (action) {
      case GLFW_PRESS:
        event.action = MouseAction::Press;
        break;
      case GLFW_RELEASE:
        event.action = MouseAction::Release;
        break;
      default:
        VLK_PANIC("Unimplemented mouse action", static_cast<int>(action));
    }

    if (modifier & GLFW_MOD_SHIFT) {
      event.modifiers |= MouseModifier::Shift;
    }

    if (modifier & GLFW_MOD_CONTROL) {
      event.modifiers |= MouseModifier::Ctrl;
    }

    if (modifier & GLFW_MOD_ALT) {
      event.modifiers |= MouseModifier::Alt;
    }

    if (modifier & GLFW_MOD_SUPER) {
      event.modifiers |= MouseModifier::Super;
    }

    if (modifier & GLFW_MOD_CAPS_LOCK) {
      event.modifiers |= MouseModifier::CapsLock;
    }

    if (modifier & GLFW_MOD_NUM_LOCK) {
      event.modifiers |= MouseModifier::NumLock;
    }

    handle->event_queue.add_raw(event);
  }

  static void scroll_callback(GLFWwindow* window, double x, double y) {}

  bool should_close() const { }

  void request_close() const {  }

  void request_attention() const {  }

 private:


 #pragma once

#include "stx/span.h"
#include "vlk/ui/event.h"

namespace vlk {
namespace ui {

struct EventPawn {
  friend struct EventPawnSystemProxy;

  virtual void tick(stx::Span<MouseButtonEvent const> mouse_button_events) {
    (void)mouse_button_events;
  }

 private:
  void system_tick(stx::Span<MouseButtonEvent const> mouse_button_events) {
    tick(mouse_button_events);
  }
};

struct EventPawnSystemProxy {
  static void system_tick(
      EventPawn& pawn, stx::Span<MouseButtonEvent const> mouse_button_events) {
    pawn.system_tick(mouse_button_events);
  }
};

}  // namespace ui
}  // namespace vlk
