#pragma once
#include "ashura/gfx.h"
#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/types.h"

namespace ash
{

// GUI blur for example needs to capture the whole scene at one point and then
// render to screen (Layer)
//
//
// Store last capture z-index + area, if non-intersecting and re-usable re-use
//
//
constexpr u32 MAXIMUM_SPOT_LIGHTS        = 64;
constexpr u32 MAXIMUM_POINT_LIGHTS       = 64;
constexpr u32 MAXIMUM_DIRECTIONAL_LIGHTS = 64;
constexpr u32 MAXIMUM_ID_SIZE            = 128;

typedef char                                Id[MAXIMUM_ID_SIZE];
typedef Vec4                                AmbientLight;
typedef struct DirectionalLight             DirectionalLight;
typedef struct PointLight                   PointLight;
typedef struct SpotLight                    SpotLight;
typedef struct OrthographicCamera           OrthographicCamera;
typedef struct PerspectiveCamera            PerspectiveCamera;
typedef struct Scene                        Scene;
typedef struct Pass_T                      *Pass;
typedef struct PassInterface                PassInterface;
typedef struct PassImpl                     PassImpl;
typedef struct Renderer                     Renderer;
typedef struct PBRPass                      PBRPass;
typedef struct BlurPass                     BlurPass;
typedef struct BloomPass                    BloomPass;
typedef struct TAAPass                      TAAPass;
typedef struct FXAAPass                     FXAAPass;
typedef struct BokehPass                    BokehPass;
typedef struct ChromaticAberrationPass      ChromaticAberrationPass;
typedef struct PassContext                  PassContext;
typedef stx::Fn<void(u64)>                  Task;
typedef stx::Fn<i32(Scene *, u64 a, u64 b)> ObjectSortKey;

// Skybox? : custom renderer at z-index 0?

struct PassContext
{
  gfx::Image     root_image;
  gfx::ImageDesc root_image_desc;
  // task executor
};

struct DirectionalLight
{
  Vec3 direction = {};
  Vec4 color     = {};
};

struct PointLight
{
  f32  attenuation = 0;
  Vec4 color       = {};
  Vec3 position    = {};
};

struct SpotLight
{
  Vec3 direction   = {};
  f32  cutoff      = 0;
  f32  attenuation = 0;
  Vec4 color       = {};
  Vec3 position    = {};
};

/// @x_mag: The floating-point horizontal magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @y_mag: The floating-point vertical magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @z_far: The floating-point distance to the far clipping plane. This value
/// MUST NOT be equal to zero. zfar MUST be greater than znear.
/// @z_near: The floating-point distance to the near clipping plane.
struct OrthographicCamera
{
  f32 x_mag  = 0;
  f32 y_mag  = 0;
  f32 z_far  = 0;
  f32 z_near = 0;
};

/// @aspect_ratio: The floating-point aspect ratio of the field of view.
/// @y_fov: The floating-point vertical field of view in radians. This value
/// SHOULD be less than π.
/// @z_far: The floating-point distance to the far clipping plane.
/// @z_near: The floating-point distance to the near clipping plane.
struct PerspectiveCamera
{
  f32 aspect_ratio = 0;
  f32 y_fov        = 0;
  f32 z_far        = 0;
  f32 z_near       = 0;
};

struct Camera
{
  Mat4Affine model      = {};
  Mat4Affine view       = {};
  Mat4Affine projection = {};
};

// should offscreen rendering be performed by objects??? shouldn't they be a
// different viewport? what if it is once per frame for example
//
// objects are pre-sorted by z-index (if z-index dependent)
// portals are subscenes
// camera data
// light data in resource manager
// we can cull lights
//
//
// A scene prepared for rendering
//
//
// if lights buffer dirty, update
// if cameras dirty, update
// buffer = encode(scene->get_lights());
// descriptor_set->ref(buffer);
//
//
// how object culling will work?
// particle pass
//
// i.e. add_pass("PBR Object Pass", [scene, descriptor_set](command_buffer,
// u64 const * cull_mask  ){
//
//  bind pbr pipeline
//  for(object : objects){
//  if(cull_mask[...] >> ... & 1){
//  bind_lights();
//  bind_transform(mvp, world, local);
//  bind_materials();
//  command_buffer->bind_descriptor_sets(descriptor_set);
//  command_buffer->draw();
//  }
// }
//
//
// bind particle pipeline
//  for(particle: particles){
//
// }
// for(object_pass: object_passes)
// object_pass(command_buffer, cull, ctx.camera, ctx.lights);
//
//
// // bind custom pipelines
// for(object: objects){
//  bind_lights();
//  bind_transform(mvp, world, local);
//  bind_materials();
//  custom_encode_function(scope...);
//  custom_draw_function(scope...);
// }
//
// }  );
//
// add_pass("bloom pass", [](){});
//
// add_pass("chromatic aberration", [](){});
//

// manages and uploads render resources to the GPU.
struct PassContext
{
  // sort by update frequency, per-frame updates, rare-updates
  // allocate temporary image for rendering
  // renderpasses, framebuffers, pipeline caches,async pipeline cache loader and
  // pipeline builder, buffer views
  //
  // resource manager
  // static buffer + streaming
  // dynamic buffers + streaming
  //
  //
  // UNIFORM COLOR Texture cache with image component swizzling
  //
  //
  Scene *scene = nullptr;
};

/// @init: add self and resources to renderer
/// @deinit: remove self and resources from renderer
/// @update: update internal data based on changed information in the scene
/// @encode: encode commands to be sent to the gpu
struct PassInterface
{
  void (*init)(Pass self, PassContext *ctx)   = nullptr;
  void (*deinit)(Pass self, PassContext *ctx) = nullptr;
  void (*update)(Pass self, PassContext *ctx) = nullptr;
  void (*encode)(Pass self, PassContext *ctx,
                 gfx::CommandEncoderImpl command_encoder, i64 z_index,
                 bool is_transparent, u64 first_scene_object,
                 u64 num_scene_objects)       = nullptr;
};

// can be loaded from a DLL i.e. C++ with C-linkage => DLL
//
// we consult the passes to execute, not the objects.
//
struct PassImpl
{
  Pass                 self      = nullptr;
  PassInterface const *interface = nullptr;
};

/// linearly-tilted tree node
/// @pass: pass to be used to render this object
struct ObjectNode
{
  u64 parent       = 0;
  u64 level        = 0;
  u64 next_sibling = 0;
  u64 first_child  = 0;
  u64 pass         = 0;
  u64 pass_object  = 0;
};

// world->[capture->world]->post-fx->hud->[capture->hud]
//
//
//
// i.e. world scene pass -> post-fx pass -> HUD pass
//
//
// used to represent dependency of passes
//
struct PassNode
{
  u64 pass = 0;
};

struct SceneObject
{
  Mat4Affine local_transform = {};
  Box        aabb            = {};
  i64        z_index         = 0;
  bool       is_transparent  = false;
};

// on change, on modification, on stream
// renderer inputs to the scene
//
// camera should be assumed to change every frame
//
// Passes are static and not per-object?
//
// NOTE: the Scene's memory usage never shrinks, only grows. it is re-used.
//
//
// Invocation Procedure
//
// - sort scene objects by z-index
// - for objects in the same z-index, sort by transparency (transparent objects
// drawn last)
// - sort transparent objects by AABB from camera frustum
// - for objects in the same z-index, sort by passes so objects in the same pass
// can be rendered together.
// - sort objects in the same pass by key from render pass (materials and
// textures and resources) to minimize pipeline state changes
// - for the z-index group sorted objects with the same passes
// - invoke the pass with the objects
//
//
//
// all objects belong to and respect this hierarchy
//
// objects_z_ordered will contain indices to the objects and will be sorted by a
// z_index key
//
//
// Area lights: https://learnopengl.com/Guest-Articles/2022/Area-Lights
//
//
struct Scene
{
  Camera            camera                      = {};
  AmbientLight      ambient_light               = {};
  DirectionalLight *directional_lights          = 0;
  PointLight       *point_lights                = 0;
  SpotLight        *spot_lights                 = 0;
  bool              lights_dirty_mask           = false;
  u32               num_directional_lights      = 0;
  u32               num_point_lights            = 0;
  u32               num_spot_lights             = 0;
  ObjectNode       *object_nodes                = nullptr;
  Mat4Affine       *object_local_transforms     = nullptr;
  Mat4Affine       *object_global_transforms    = nullptr;
  Box              *object_aabb                 = nullptr;
  i64              *object_z_index              = nullptr;
  u64              *object_transform_dirty_mask = nullptr;
  u64              *object_cull_mask            = nullptr;
  u64              *objects_z_ordered           = nullptr;
  u64              *object_transparency_mask    = nullptr;
  u64               num_objects                 = 0;

  // validity mask for object, node, light
  // free slots list for  object,node,light
  // id map?
  // free ids?

  u64  add_object(Span<char const> node_id, SceneObject const &);
  void remove_object(u64);
};

// sort objects by z-index, get min and max z-index
// for all objects in the z-index range, invoke the passes pass->encode(z_index,
// begin_objects, num_objects)
// NOPPPEEE 3d scene objects are all in z-index 0 ?? typically, HUD objects are
// all in z-index 1 and above this means objects in the same z-index can be
// batch-rendered
//
struct Renderer
{
  PassNode *pass_nodes = nullptr;
  PassImpl *passes     = nullptr;
  u64       num_passes = 0;
  Id       *passes_id  = nullptr;

  u64      add_pass(Span<char const> pass_id, PassImpl pass);
  PassImpl get_pass(u64);
  PassImpl get_pass_by_id(Span<char const> pass_id);

  // perform frustum and occlusion culling of objects and light (in the same
  // z-index) then cull by z-index???? Z-index not needed in culling
  // occlussion culling only happens when a fully-opaque object occludes another
  // object.
  //
  // cull lights by camera frustum
  //
  // TODO(lamarrr): how to represent directional lights not affecting an area.
  //
  void cull(PassContext *ctx);

  // vfx
  // add_light(); -> update buffers and descriptor sets, increase size of
  //
  // we need
  // the mesh and object render-data is mostly pre-configured or modified
  // outside the renderer we just need to implement the post-effects and
  // render-orders and add other passes on top of the objects
  //
  //
  // on frame begin, pending uploads are first performed
  //
  //
  // each scene is rendered and composited onto one another? can this possibly
  // work for portals?
  //
  void render(PassContext *ctx);
};

struct PBRMaterialSource
{
  ImageSpan<u8 const> albedo                   = {};
  ImageSpan<u8 const> ambient_occlusion        = {};
  ImageSpan<u8 const> emmissive                = {};
  ImageSpan<u8 const> metallic                 = {};
  ImageSpan<u8 const> normal                   = {};
  ImageSpan<u8 const> roughness                = {};
  Vec4                albedo_factor            = {};
  Vec4                ambient_occlusion_factor = {};
  Vec4                emmissive_factor         = {};
  Vec4                metallic_factor          = {};
  Vec4                normal_factor            = {};
  Vec4                roughness_factor         = {};
};

//
// TODO(lamarrr): let's make the pbr a shader function instead. the pass should
// be able to chain multiple properties atop of each other
// use shader functions
//
//
// i.e. single block Pbr in editor
//
//  CustomPbrOp{
//
//  properties: [emmissive, base_color, base_factor]
//
//
//  }
//
//  construct GLSL shader with Ops and macros to configure based on ops
//
//
//
//
// struct TextureLoc{
// // image
// // aspect
// // subset
// // size
// };
//
// textures need to be packed together by usage as much as possible
//
//
// U8, I8, ...
//
//
enum class PbrMaterialType : u8
{
  None    = 0,
  Bool    = 1,
  F32     = 2,
  Vec2    = 3,
  Vec3    = 4,
  Vec4    = 5,
  Texture = 6
};

// SEE: https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos
// SEE:
// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/textures.glsl
enum class PbrMaterialSlot : u8
{
  None                      = 0,
  BaseColorFactor           = 1,        // KHR's PBR metallic roughness
  BaseColorTexture          = 2,
  MetallicFactor            = 3,
  MetallicTexture           = 4,
  RoughnessFactor           = 5,
  RoughnessTexture          = 6,
  NormalTexture             = 7,
  OcclusionFactor           = 8,
  OcclusionTexture          = 9,
  EmmissiveFactor           = 10,
  EmmissiveTexture          = 11,
  AnisotropyStrength        = 12,        //  KHR_materials_anisotropy; 0.0
  AnistropyRotation         = 13,        // 0.0
  AnisotropyTexture         = 14,
  ClearcoatFactor           = 15,        //  KHR_materials_clearcoat; 0.0
  ClearcoatTexture          = 16,
  ClearcoatRoughnessFactor  = 17,        // 0.0
  ClearcoatRoughnessTexture = 18,
  ClearcoatNormalTexture    = 19,
  EmissiveStrength          = 20,        // KHR_materials_emissive_strength; 1.0
  IndexOfRefraction         = 21,        // KHR_materials_ior; 1.5
  IridescenceFactor         = 22,        // KHR_materials_iridescence 0.0
  IridescenceTexture        = 23,
  IridescenceIndexOfRefraction = 24,        // 1.3
  IridescenceThicknessMinimum  = 25,        // 100.0
  IridescenceThicknessMaximum  = 26,        // 400.0
  IridescenceThicknessTexture  = 27,
  SheenColorFactor             = 28,        // KHR_materials_sheen; (0,0,0)
  SheenColorTexture            = 29,
  SheenRoughnessFactor         = 30,        // 0
  SheenRoughnessTexture        = 31,
  SpecularFactor               = 32,        // KHR_materials_specular;1.0
  SpecularTexture              = 33,
  SpecularColorFactor          = 34,        // (1,1,1)
  SpecularColorTexture         = 35,
  TransmissionFactor           = 36,        // KHR_materials_transmission: 0
  TransmissionTexture          = 37,
  Unlit                        = 38        // KHR_materials_unlit
};

struct PbrMaterialNameType
{
  char const     *name;
  PbrMaterialType type;
};

constexpr PbrMaterialNameType pbr_material_slot_name_type[] = {
    {"None", PbrMaterialType::None}, {"None", PbrMaterialType::None},
    {"None", PbrMaterialType::None}, {"None", PbrMaterialType::None},
    {"None", PbrMaterialType::None}, {"None", PbrMaterialType::None},
    {"None", PbrMaterialType::None}, {"None", PbrMaterialType::None},
};

struct PBRVertex
{
  f32 x = 0, y = 0, z = 0;
  f32 u = 0, v = 0;
};

// height map + other shader functions?
struct PBRMesh
{
  u32 buffer      = 0;
  u32 first_index = 0;
  u32 num_indices = 0;
};

struct PBRObject
{
  PBRMaterial material   = {};
  PBRMesh     mesh       = {};
  i64         z_index    = 0;
  u64         scene_node = 0;
};

struct PBRPass
{
  PBRObject               *objects               = nullptr;
  u64                      num_objects           = 0;
  PassContext             *ctx                   = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::PipelineCache       pipeline_cache        = nullptr;
  gfx::Sampler             sampler               = nullptr;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;
  gfx::DescriptorHeapImpl  descriptor_heap       = {};
  gfx::RenderPass          render_pass           = nullptr;
  gfx::Framebuffer         framebuffer           = nullptr;

  // add AABB to scene and init material for rendering
  // upload data to gpu, setup scene for it, add AABB, add to pass list
  u64 add_object(Span<char const> id, Span<PBRVertex const> vertices,
                 PBRMaterialSource const &material)
  {
    // build mesh tree (static and dynamic)
    // build textures
    // add object to scene hierarchy for transforms
    // add aabb to scene for frustum culling
    // GPU-based frustum culling + transform calculations?
  }

  void remove_object(u64);

  static void init(Pass self_, PassContext *ctx)
  {
    // create pipeline descriptor sets
    // create sampler
    // create descriptor heap
    // create renderpass
    // fetch pipeline cache
    // async build pipeline
    PBRPass *pbr_pass = (PBRPass *) self_;
  }

  static void deinit(Pass self_, PassContext *ctx)
  {
  }

  static void update(Pass self_, PassContext *ctx)
  {
    // re-build renderpass and framebuffer if needed
  }

  static void encode(Pass self_, PassContext *ctx,
                     gfx::CommandEncoderImpl command_encoder)
  {
  }

  static PassInterface const interface{
      .init = init, .deinit = deinit, .update = update, .encode = encode};
};

// use for:
// fill, stroke, fill and stroke, images, rects, squares, round rects,
// linear-gradients, linear-gradient blended images can be instanced
// TODO(lamarrr): clipping
// TODO(lamarrr): temp-offscreen rendering: request scratch image with maximum
// size of the viewport size and specific format, not released until execution
// completes, the pass doesn't need to release it so other passes can re-use it
// if needed.
//
//
//
// quad + instanced + transformed + antialiased
//
// Quad is an object without a type or specification. a unit square with just a
// transformation matrix.
//
struct RRect
{
  Vec3 center           = {};
  Vec3 half_extent      = {};
  f32  border_thickness = 0;
  Vec4 border_radii     = {};
  Vec2 uv0              = {};
  Vec2 uv1              = {};
  Vec4 tint_uv0         = {};
  Vec4 tint_uv1         = {};
  Vec4 border_color     = {};
  // i64 z_index?
};

struct RRectMaterialSource
{
  ImageSpan<u8 const> ambient;
};

struct RRectMaterial
{
  gfx::ImageView ambient;
};

struct RRectObject
{
  RRect         rrect;
  RRectMaterial material;
};

// normal 2d objects
// occlusion culling + batching
// custom objects + custom passes?
// offscreen passes
//
//
// CUSTOM SHADERS + CUSTOM PASSES
//
//
struct RRectPass
{
  RRectObject             *objects               = nullptr;
  u64                      num_objects           = 0;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::PipelineCache       pipeline_cache        = nullptr;
  gfx::Sampler             sampler               = nullptr;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;
  gfx::DescriptorHeapImpl  descriptor_heap       = {};
  gfx::RenderPass          render_pass           = nullptr;
  gfx::Framebuffer         framebuffer           = nullptr;

  u64  add_object(Span<char const> id, RRect const &,
                  RRectMaterialSource const &);
  void remove_object(u64);
};

struct ChromaticAberrationPass
{
};

struct FXAAPass
{
};

// TODO(lamarrr): we can create a single image and just re-use it depending on
// the components/aspects we need to use for each type of pass
//
//
// object-clip space blur
//
// - capture scene at object's screen-space area, dilate by the blur extent
// - reserve scratch stencil image with at least size of the dilated area
// - blur captured area
// - render object to offscreen scratch image stencil only
// - using rendered stencil, directly-write (without blending) onto scene again
struct BlurPass
{
};

struct BloomPass
{
};

};        // namespace ash
