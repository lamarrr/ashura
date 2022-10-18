#include <cinttypes>

#include "stx/string.h"
#include "stx/vec.h"

// See: https://html.spec.whatwg.org/multipage/canvas.html

// TODO(lamarrr): we'll acrtually generate 3D vertices with them so they can
// play well with 3D animations

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

struct vec4 {
  f32 x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
};
struct vec2 {
  f32 x = 0.0f, y = 0.0f;
};
struct vec3 {
  f32 x = 0.0f, y = 0.0f, z = 0.0f;
};
struct mat2d {
  vec2 data[2]{};
};
struct mat3d {
  vec3 data[3]{};
};
struct mat4d {
  vec4 data[4]{};
};

// TODO(lamarrr): child must inherit parent's transformation and opacity

struct TextMetrics {
  // x-direction
  f32 width = 0.0f;  // advance width
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

// struct Path {
//   void add_path(Path path, mat3d transform);
//
//   bool is_closed = false;
//   stx::Option<stx::Unique<Path*>> next;
//   vec3 last_point;
//   stx::Vec<vec3> fill_vertices{stx::os_allocator};
//   stx::Vec<vec3> stroke_vertices{stx::os_allocator};
// };

enum class ImageSmoothingQuality : u8 { Low, Medium, High };

// struct CanvasState {
//   void save();     // push state on state stack
//   void restore();  // pop state stack and restore state
//   void reset();    // reset the rendering context to its default state
// };

enum class BlendMode : u8 { SourceOver, Src };

struct Compositing {
  // compositing
  f32 global_alpha = 1.0f;
  BlendMode blend_mode = BlendMode::SourceOver;
};

struct ImageSmoothing {
  // image smoothing
  bool image_smoothing_enabled = true;
  ImageSmoothingQuality image_smoothing_quality = ImageSmoothingQuality::Medium;
};

struct GradientOrCanvasPattern {};

struct FillStrokeStyles {
  // colors and styles (see also the CanvasPathDrawingStyles and
  // CanvasTextDrawingStyles structs)
  GradientOrCanvasPattern stroke_style;  // (default black)
  GradientOrCanvasPattern fill_style;    // (default black)
};

struct Color {
  u8 r = 0, g = 0, b = 0, a = 0;
};

struct Shadow {
  f32 offset_x = 0.0f;
  f32 offset_y = 0.0f;
  f32 blur = 0.0f;
  Color color{};
};

struct CanvasFilters {
  stx::String filter = stx::string::make_static("none");
};

// struct CanvasRect {
//   void clear_rect(f32 x, f32 y, f32 w, f32 h);
//   void fill_rect(f32 x, f32 y, f32 w, f32 h);
//   void stroke_rect(f32 x, f32 y, f32 w, f32 h);
// };

enum class FillRule : u8 {};

// TODO(lamarrr): this should be a GPU texture/image
struct ImageSource {};

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

struct PathDrawingStyle {
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
// TODO(lamarrr): embed font into
// a cpp file
struct TextDrawingStyle {
  stx::String font_family = stx::string::make_static("SF Pro");
  FontWeight font_weight = FontWeight::Normal;
  u32 font_size = 10;
  TextAlign align = TextAlign::Start;
  TextBaseline baseline = TextBaseline::Alphabetic;
  TextDirection direction = TextDirection::Inherit;
  u32 letter_spacing = 0;
  FontKerning font_kerning = FontKerning::Auto;
  FontStretch font_stretch = FontStretch::Normal;
  u32 wordSpacing = 0;
};

struct Brush {
  PathDrawingStyle drawing_style;
  TextDrawingStyle text_style;
  CanvasFilters filter;
  Shadow shadow;
  ImageSmoothing smoothing;
  Compositing compositing;
};

struct Canvas {
  Brush brush;
  vec3 rotation;
  vec3 translation;
  vec3 scale;
  // rect clip, rrect clip

  void save();     // push state on state stack
  void restore();  // pop state stack and restore state
  void reset();    // reset the rendering context to its default state

  void clear(Color color);

  // PATH API
  //   void begin_path();
  //   void fill(FillRule fill_rule);
  //   void fill(Path path, FillRule fill_rule);
  //   void stroke();
  //   void stroke(Path path);
  //   void clip(FillRule fill_rule);
  //   void clip(Path path, FillRule fill_rule);

  // HELPERS
  void clip_rect();
  void clip_rrect();
  void clip_slanted_rect();

  // TEXT API
  void fill_text(stx::String text, f32 x, f32 y, f32 max_width);
  void stroke_text(stx::String text, f32 x, f32 y, f32 max_width);

  // IMAGE API
  void draw_image(ImageSource image, f32 dx, f32 dy);
  void draw_image(ImageSource image, f32 dx, f32 dy, f32 dw, f32 dh);
  void draw_image(ImageSource image, f32 sx, f32 sy, f32 sw, f32 sh, f32 dx,
                  f32 dy, f32 dw, f32 dh);

  // SHARED PATH API
  void close_path();
  void move_to(f32 x, f32 y);
  void line_to(f32 x, f32 y);
  void quadratic_curve_to(f32 cpx, f32 cpy, f32 x, f32 y);
  void bezier_curve_to(f32 cp1x, f32 cp1y, f32 cp2x, f32 cp2y, f32 x, f32 y);
  void arc_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 radius);
  void rect(f32 x, f32 y, f32 w, f32 h);
  void round_rect(f32 x, f32 y, f32 w, f32 h, vec4 radii);
  void slanted_rect(f32 x, f32 y, f32 width, f32 height);
  void arc(f32 x, f32 y, f32 radius, f32 start_angle, f32 end_angle);
  void ellipse(f32 x, f32 y, f32 radius_x, f32 radius_y, f32 rotation,
               f32 start_angle, f32 end_angle);
};

struct Shader {};

struct DrawCommand {
  u64 index_start = 0;
  u64 num_triangles = 0;
  stx::Rc<Shader*> frag_shader;
  stx::Rc<Shader*> vert_shader;
  //   VkImage texture_albedo;
  //   VkImage texture_ambient_occlusion;
};

struct DrawList {
  stx::Vec<vec3> vertices{stx::os_allocator};
  stx::Vec<u64> indices{stx::os_allocator};
  stx::Vec<DrawCommand> commands_{stx::os_allocator};
};