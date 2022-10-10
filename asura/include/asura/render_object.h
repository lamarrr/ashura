#include <atomic>
#include <cinttypes>

#include "asura/primitives.h"
#include "stx/vec.h"

namespace asr {
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
struct PbrMaterial {
  Texture albedo;
  Texture normal;
  Texture metalic;
  Texture roughness;
  Texture ambient_occlusion;
  Texture emissive;
};
// 2D rendering, TODO(shadow?)
struct ColorMaterial {
  Color color;
};
struct Material {};
Span<Material const> materials;
Shader vertex_shader;
Shader fragment_shader;
struct Mesh {
  FixedVec<Vec4 const> vertices;
  FixedVec<uint32_t const> indices;
};
struct Entity {
  uint64_t id = 0;
};
struct EntitySystem {
  Entity create_entity() {
    return Entity{next_id.fetch_add(1, std::memory_order_relaxed)};
  }
  std::atomic<uint64_t> next_id{1};
};
struct Scene {
  void add_component(Component component);
};
struct RendererSystem {
  void render(Scene);
};
struct Typeface {};
// path shader, line shader,round rect shader, circle shader, etc
struct Canvas {
  virtual void path();
  virtual void line(float x1, float y1, float x2, float y2, bool antialias,
                    Color color, float thickness);
  virtual void rect(float x, float y, float width, float height, bool antialias,
                    Color color, float thickness, bool stroke);
  virtual void round_rect(float x, float y, float width, float height,
                          float tl_radius, float tr_radius, float bl_radius,
                          float br_radius, bool antialias, Color color,
                          float thickness, bool stroke);
  virtual void circle(float x, float y, float radius, bool antialias,
                      float thickness, Color color);
  virtual void text(char const *text, Typeface const &typeface);
  virtual void image(uint8_t *pixels, float x, float y, float width,
                     float height, uint8_t blend_mode);
};
struct Transform {
  Vec3 translation{0.0f, 0.0f, 0.0f};
  Vec3 rotation{0.0f, 0.0f, 0.0f};
  Vec3 scale{1.0f, 1.0f, 1.0f};
  Vec3 position{0.0f, 0.0f, 0.0f};
};
struct RenderObject {
  Transform transform;
  Mesh mesh;
};
}  // namespace asr
