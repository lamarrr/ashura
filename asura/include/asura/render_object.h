#include <atomic>
#include <chrono>
#include <cinttypes>
#include <map>

#include "asura/event.h"
#include "asura/primitives.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/span.h"
#include "stx/vec.h"

namespace asr {

// TODO(lamarrr): runtime antialiasing setting

// global resource identifier
using UUID = uint64_t;
struct Material {};
struct Image {};
struct Shader {};
struct Texture {};
struct DirectionalLight {};
struct SpotLight {};
struct PointLight {};
struct Camera {
  Mat4x4 projection;
  Vec3 position;
  // transform, rotation, model, view, far_plane
};
struct Component {};
// 3D rendering

enum class MaterialType {
  Albedo,
  Normal,
  Metalic,
  Roughness,
  AmbientOcclusion,
  Emissive
};

struct PbrMaterial {
  std::map<MaterialType, Texture> materials;
};
// 2D rendering, TODO(shadow?)
struct ColorMaterial {
  Color color;
};
struct Material {};
struct Mesh {
  FixedVec<Vec4 const> vertices;
  FixedVec<uint32_t const> indices;
};
struct Entity {
  uint64_t id = 0;
  stx::String identifier = stx::string::make_static("unnamed");
};
struct EntitySystem {
  Entity create_entity() {
    return Entity{next_id.fetch_add(1, std::memory_order_relaxed)};
  }
  std::atomic<uint64_t> next_id{1};
};
struct Scene {
  void add_component(Component component);
  // GPU jobs stx::Vec<stx::UniqueFn<void()>>;
};
struct RendererSystem {
  void render(Scene);
  //
};
struct Typeface {};

struct Transform {
  Vec3 translation{0.0f, 0.0f, 0.0f};
  Vec3 rotation{0.0f, 0.0f, 0.0f};
  Vec3 scale{1.0f, 1.0f, 1.0f};
  Vec3 position{0.0f, 0.0f, 0.0f};
};
struct RenderObject {
  Transform transform;
  Mesh mesh;
  Shader vertex_shader;
  Shader fragment_shader;
  Material material;  // or 2D material?
};
struct Rect {};
struct WidgetInfo {};
struct Offset {
  float x, y;
};
using ZIndex = uint64_t;
struct System {};
// TODO(lamarrr): this must work well for 3D animations, might need transforms
// and the likes
// should parents be able to transform, clip, and rotate children and
// themselves?
//
// TODO(lamarrr): clipping, etc
//
// components that can be spawned and placed in a scene
struct Actor;
// gives commands to the actor, translation, rotation
struct Pawn;

enum class Visibility : uint8_t { Shown, Hidden };

struct Context {
  int plugins, window;
};

struct JsonObject {};

struct Widget {
  // + handling floating, relative, sticky and fixed positioned elements
  virtual Rect layout(Extent allotted_extent);  // min, max, available
  // + other properties that will make its rendering work well with other
  // widgets
  virtual void draw(Canvas &);
  //
  // virtual void draw_child();
  virtual void tick(std::chrono::nanoseconds interval, Context &ctx);
  virtual stx::Span<Widget *const> get_children();
  virtual WidgetInfo get_debug_info();
  virtual Visibility get_visibility();
  virtual stx::Option<ZIndex> get_z_index();
  // events
  virtual void on_click(MouseButton btn, Offset pos);
  virtual void on_double_click(MouseButton button, Offset pos);
  virtual void on_mouse_scroll(Offset translation, float precise_x,
                               float precise_y);
  virtual void on_mouse_move();
  virtual void on_hover(Offset pos);
  virtual void on_mouse_down();
  virtual void on_mouse_up();
  virtual void on_mouse_enter();
  virtual void on_mouse_leave();
  virtual void on_mouse_out();
  virtual void on_mouse_over();
  virtual void on_enter();  // ?
  virtual void on_tap();
  virtual void on_drag();
  virtual void on_drag_start();
  virtual void on_drag_end();
  virtual void on_focus();
  virtual void on_focus_in();
  virtual void on_focus_out();
  virtual void on_scroll(Offset translation, float precision_x,
                         float precision_y);  // scroll of this widget's content
  virtual void on_enter_view();
  virtual void on_leave_view();
  // virtual void on_full_screen_change();
  // virtual void on_keydown();
  // virtual void on_keyup();
  // virtual void on_input(); - input widget
  virtual void tooltip();
  virtual void accessibility_navigate();
  virtual void accessibility_info();
  // bind to keyboard
  // virtual void on_keyboard();
  // state saving - just bytes, left to the widget to decide how to save and
  // restore state
  virtual JsonObject save();
  virtual void restore(JsonObject const &);

  //
  stx::UniqueFn<void()> mark_needs_redraw =
      stx::fn::rc::make_unique_static([]() {});
  stx::UniqueFn<void()> mark_needs_relayout =
      stx::fn::rc::make_unique_static([]() {});
  stx::UniqueFn<void()> mark_children_changed =
      stx::fn::rc::make_unique_static([]() {});
};

struct GlobalEvent {
  void on_mouse_click();
  void on_key();
  void on_shutdown_requested();
};

}  // namespace asr
