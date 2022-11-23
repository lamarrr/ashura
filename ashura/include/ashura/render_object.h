#include <atomic>
#include <chrono>
#include <cinttypes>
#include <map>

#include "ashura/event.h"
#include "ashura/primitives.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/span.h"
#include "stx/vec.h"

namespace asr {
/*
// TODO(lamarrr): runtime antialiasing setting

// global resource identifier

struct Material {};
struct Image {};
struct Shader {};
struct Texture {};
struct DirectionalLight {};
struct SpotLight {};
struct PointLight {};
struct Camera {
  mat4 projection;
  vec3 position;
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
  stx::FixedVec<vec4> vertices;
  stx::FixedVec<u32> indices;
};
struct Entity {
  u64 id = 0;
  stx::String identifier = stx::string::make_static("unnamed");
};
struct EntitySystem {
  Entity create_entity() {
    return Entity{next_id.fetch_add(1, std::memory_order_relaxed)};
  }
  std::atomic<u64> next_id{1};
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

struct Transform;
struct RenderObject {
  Transform *transform;
  vec3 position{0.0f, 0.0f, 0.0f};
  Mesh mesh;
  Shader vertex_shader;
  Shader fragment_shader;
  Material material;  // or 2D material?
};


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


struct Context {
  int plugins, window;
};

*/
}  // namespace asr
