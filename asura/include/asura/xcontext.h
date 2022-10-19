#include <cinttypes>

#include "asura/primitives.h"
#include "stx/string.h"
#include "stx/vec.h"

// See: https://html.spec.whatwg.org/multipage/canvas.html

// TODO(lamarrr): we'll acrtually generate 3D vertices with them so they can
// play well with 3D animations

namespace asr {

// TODO(lamarrr): this should be a GPU texture/image
struct VkImage {};

struct Image {
  VkImage image;
};

// TODO(lamarrr): child must inherit parent's transformation and opacity

struct TextMetrics {
  // x-direction
  f32 width = 0.0f;
  f32 actual_bounding_box_left = 0.0f;
  f32 actual_bounding_box_right = 0.0f;

  // y-direction
  f32 font_bounding_box_ascent = 0.0f;
  f32 font_bounding_box_descent = 0.0f;
  f32 actual_bounding_box_ascent = 0.0f;
  f32 actual_bounding_box_descent = 0.0f;
  f32 em_height_ascent = 0.0f;
  f32 em_height_descent = 0.0f;
  f32 hanging_baseline = 0.0f;
  f32 alphabetic_baseline = 0.0f;
  f32 ideographic_baseline = 0.0f;
};

enum class ImageSmoothingQuality : u8 { Low, Medium, High };

enum class BlendMode : u8 { SrcOver, Src };

struct Compositing {
  f32 global_alpha = 1.0f;
  BlendMode blend_mode = BlendMode::SrcOver;
};

struct ImageSmoothing {
  bool image_smoothing_enabled = true;
  ImageSmoothingQuality image_smoothing_quality = ImageSmoothingQuality::Medium;
};

struct Shadow {
  f32 offset_x = 0.0f;
  f32 offset_y = 0.0f;
  f32 blur_radius = 0.0f;
  Color color = colors::BLACK;
};

struct Filter { /* None by default */
};

// the type of endings that UAs will place on the end of lines
enum class LineCap : u8 { Butt, Round, Square };

// the type of corners that UAs will place where two lines meet
enum class LineJoin : u8 { Round, Bevel, Miter };

enum class TextAlign : u8 {
  Start /* detect locale and other crap */,
  End,
  Left,
  Right,
  Center
};

enum class TextBaseline : u8 {
  Top,
  Hanging,
  Middle,
  Alphabetic,
  Ideographic,
  Bottom
};

enum class TextDirection : u8 { Inherit, Ltr, Rtl, Tbrl };

enum class FontKerning : u8 { Auto, Normal, None };

enum class FontStretch : u8 {
  UltraCondensed,
  ExtraCondensed,
  Condensed,
  SemiCondensed,
  Normal,
  SemiExpanded,
  Expanded,
  ExtraExpanded,
  UltraExpanded
};

struct PathStyle {
  f32 line_width = 1.0f;
  LineCap line_cap = LineCap::Butt;
  LineJoin line_join = LineJoin::Miter;
  f32 miter_limit = 10.0f;
};

struct Typeface {};

enum class FontWeight : u32 {
  Thin = 100,
  ExtraLight = 200,
  Light = 300,
  Normal = 400,
  Medium = 500,
  Semi = 600,
  Bold = 700,
  ExtraBold = 800,
  Black = 900,
  ExtraBlack = 950
};

// TODO(lamarrr): embed font into a cpp file

struct TextStyle {
  stx::String font_family = stx::string::make_static("SF Pro");
  FontWeight font_weight = FontWeight::Normal;
  u32 font_size = 10;
  TextAlign align = TextAlign::Start;
  TextBaseline baseline = TextBaseline::Alphabetic;
  TextDirection direction = TextDirection::Inherit;
  u32 letter_spacing = 0;
  FontKerning font_kerning = FontKerning::Auto;
  FontStretch font_stretch = FontStretch::Normal;
  u32 word_spacing = 0;
};

struct Brush {
  bool fill = true;
  Color color = colors::BLACK;
  stx::Option<stx::Rc<Image*>> pattern;
  PathStyle path_style;
  TextStyle text_style;
  Filter filter;
  Shadow shadow;
  ImageSmoothing smoothing;
  Compositing compositing;
};

struct Shader {};

struct Transform {
  vec3 rotation{.x = 0.0f, .y = 0.0f, .z = 0.0f};
  vec3 translation{.x = 0.0f, .y = 0.0f, .z = 0.0f};
  vec3 scale{.x = 1.0f, .y = 1.0f, .z = 1.0f};

  constexpr mat4x4 as_mat() const;
};

struct DrawCommand {
  u64 indices_offset = 0;
  u64 num_triangles = 0;
  Transform transform;
  stx::Rc<Shader*> frag_shader;  // - clip options will apply in the fragment
                                 // and vertex shaders
                                 // - blending
  stx::Rc<Shader*> vert_shader;
  Color color = colors::BLACK;
  stx::Option<stx::Rc<Image*>> texture;
};

struct DrawList {
  stx::Vec<vec3> vertices{stx::os_allocator};
  stx::Vec<u64> indices{stx::os_allocator};
  stx::Vec<DrawCommand> commands{stx::os_allocator};
};

struct CanvasState {
  Transform transform;
  // clips
};

struct Canvas {
  vec3 position;
  ExtentF extent;
  Brush brush;
  stx::Vec<CanvasState> state_stack{stx::os_allocator};
  DrawList draw_list;

  // rect clip, rrect clip
  void save();     // push state on state stack
  void restore();  // pop state stack and restore state
  void reset();    // reset the rendering context to its default state

  void clear(Color color) {
    save();
    // reset rotation, translation, and scale
    // rotation should be zero
    //
    // set style to fill, with color color
    rect(0.0f, 0.0f, width, height);
    restore();
  }

  // HELPERS
  void clip_rect();
  void clip_rrect();
  void clip_slanted_rect();

  // TEXT API
  void text(stx::String text, OffsetF position);

  // IMAGE API
  void draw_image(stx::Rc<Image*> image, OffsetF offset);
  void draw_image(stx::Rc<Image*> image, RectF area);
  void draw_image(stx::Rc<Image*> image, RectF portion, RectF target);

  // SHARED PATH API
  void close_path();
  void move_to(OffsetF point);
  void line_to(OffsetF point);
  void quadratic_curve_to(f32 cpx, f32 cpy, f32 x, f32 y);
  void bezier_curve_to(f32 cp1x, f32 cp1y, f32 cp2x, f32 cp2y, f32 x, f32 y);
  void arc_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 radius);

  // PRIMITIVES
  void rect(RectF r);
  void round_rect(RectF area, vec4 radii);
  void slanted_rect(RectF area);
  void arc(f32 x, f32 y, f32 radius, f32 start_angle, f32 end_angle);
  void circle(OffsetF center, f32 radius);
  void ellipse(OffsetF center, f32 rx, f32 ry, f32 rotation, f32 start_angle,
               f32 end_angle);
};

};  // namespace asr
