#pragma once
#include "ashura/gfx.h"
#include "ashura/image.h"
#include "ashura/types.h"

namespace ash
{

// TODO(lamarrr): offscreen rendering
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

typedef struct Box                     Box;
typedef char                           Id[MAXIMUM_ID_SIZE];
typedef Vec4                           AmbientLight;
typedef struct DirectionalLight        DirectionalLight;
typedef struct PointLight              PointLight;
typedef struct SpotLight               SpotLight;
typedef struct OrthographicCamera      OrthographicCamera;
typedef struct PerspectiveCamera       PerspectiveCamera;
typedef struct Scene                   Scene;
typedef struct Pass_T                 *Pass;
typedef struct PassInterface           PassInterface;
typedef struct PassImpl                PassImpl;
typedef struct Renderer                Renderer;
typedef struct PBRPass                 PBRPass;
typedef struct BlurPass                BlurPass;
typedef struct BloomPass               BloomPass;
typedef struct TAAPass                 TAAPass;
typedef struct FXAAPass                FXAAPass;
typedef struct BokehPass               BokehPass;
typedef struct ChromaticAberrationPass ChromaticAberrationPass;

// Skybox? : custom renderer at z-index 0?

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

// TODO(lamarrr): light materials, shader?
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
// TODO(lamarrr): multi-pass object rendering?
//
// A scene prepared for rendering
//
//
// TODO(lamarrr): make it passes instead? culled by specifications
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
struct PassResourceManager
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
};

// TODO(lamarrr): pass task executor
///
///
/// @init: add self and resources to renderer
/// @deinit: remove self and resources from renderer
/// @update: update internal data based on changed information in the scene
/// @encode: encode commands to be sent to the gpu
struct PassInterface
{
  void (*init)(Pass self, Scene *scene, PassResourceManager *mgr)   = nullptr;
  void (*deinit)(Pass self, Scene *scene, PassResourceManager *mgr) = nullptr;
  void (*update)(Pass self, Scene *scene, PassResourceManager *mgr) = nullptr;
  void (*encode)(Pass self, Scene *scene, PassResourceManager *mgr,
                 gfx::CommandEncoderImpl command_encoder, i64 z_index,
                 u64 const *scene_objects, u64 num_scene_objects)   = nullptr;
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
// TODO(lamarrr): this still doesn't solve the custom shader problem?
// helps with structuring dependency of passes
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
// NOTE: the Scene never shrinks, only grows
//
//
// TODO(lamarrr): invocation procedure. on a pass-by-pass basis? how about
// object relationship? won't that affect the pass procedure?
//
// all objects belong to and respect this hierarchy
//
//
// TODO(lamarrr): when objects are sorted by z-index
//
// objects_z_ordered will contain indices to the objects and will be sorted by a
// z_index key
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
  Id               *directional_lights_id       = nullptr;
  Id               *point_lights_id             = nullptr;
  Id               *spot_lights_id              = nullptr;
  Id               *object_nodes_id             = nullptr;

  // validity mask for object, node, light
  // free slots list for  object,node,light

  u64  add_object(char const *node_id, SceneObject const &);
  void remove_object(u64);
};

// sort objects by z-index, get min and max z-index
// for all objects in the z-index range, invoke the passes pass->encode(z_index,
// begin_objects, num_objects)
// 3d scene objects are all in z-index 0, HUD objects are all in z-index 1 and
// above this means objects in the same z-index can be batch-rendered
//
struct Renderer
{
  PassNode *pass_nodes = nullptr;
  PassImpl *passes     = nullptr;
  u64       num_passes = 0;
  Id       *passes_id  = nullptr;

  u64      add_pass(char const *pass_id, PassImpl pass);
  PassImpl get_pass(u64);
  PassImpl get_pass_by_id(char const *pass_id);

  // perform frustum and occlusion culling of objects and light (in the same
  // z-index) then cull by z-index
  // occlussion culling only happens when a fully-opaque object occludes another
  // object.
  //
  // cull lights by camera frustum
  //
  void cull(Scene &scene);

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
  // bounding box or frustum culling
  // scenes
  // sort by z-index
  // mesh + shaders
  //
  // each scene is rendered and composited onto one another? can this possibly
  // work for portals?
  //
  //
  void render(Scene &scene, gfx::Framebuffer);
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

struct PBRMaterial
{
  gfx::ImageView albedo                   = nullptr;
  gfx::ImageView ambient_occlussion       = nullptr;
  gfx::ImageView emmissive                = nullptr;
  gfx::ImageView metallic                 = nullptr;
  gfx::ImageView normal                   = nullptr;
  gfx::ImageView roughness                = nullptr;
  Vec4           albedo_factor            = {};
  Vec4           ambient_occlusion_factor = {};
  Vec4           emmissive_factor         = {};
  Vec4           metallic_factor          = {};
  Vec4           normal_factor            = {};
  Vec4           roughness_factor         = {};
};

struct PBRVertex
{
  f32 x = 0, y = 0, z = 0;
  f32 u = 0, v = 0;
};

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
  // descriptor group
};

//
//
// TODO(lamarrr): custom passes?
//
//
// problem is:
// - add custom pass
// - add object to pass
//    - add object to scene tree
// - how to invoke passes for each object from the scene store? and manage
// updates
//   this will mean that we can't make batched draw calls easily as we
//   previously did
//
// well, what if the passes have screen-space effects? must be done in a
// separate post-scene effect
//
// - we can't invoke from pbr passes since they are not the only passes in the
// scene, so it must be scheduled
//
// TODO(lamarrr): what texture to render to
// PBR meshes are always static
// static scene mesh?
//
// TODO(lamarrr): sort opaque objects by materials and textures and resources to
// minimize pipeline state changes
//
// for renderer 2d, we can perform batching of draw shapes of the same type
//
struct PBRPass
{
  PBRObject               *opaque_objects          = nullptr;
  u64                      num_opaque_objects      = 0;
  PBRObject               *transparent_objects     = nullptr;
  u64                      num_transparent_objects = 0;
  Scene                   *scene                   = nullptr;
  PassResourceManager     *manager                 = nullptr;
  gfx::GraphicsPipeline    pipeline                = nullptr;
  gfx::PipelineCache       pipeline_cache          = nullptr;
  gfx::Sampler             sampler                 = nullptr;
  gfx::DescriptorSetLayout descriptor_set_layout   = nullptr;
  gfx::DescriptorHeapImpl  descriptor_heap         = {};
  gfx::RenderPass          render_pass             = nullptr;
  gfx::Framebuffer         framebuffer             = nullptr;

  // add AABB to scene and init material for rendering
  // upload data to gpu, setup scene for it, add AABB, add to pass list
  u64 add_object(char const *id, Span<PBRVertex const> vertices,
                 PBRMaterialSource const &material)
  {
    // build mesh
    // build textures
    // add object to scene hierarchy for transforms
    // add aabb to scene for frustum culling
    // GPU-based frustum culling + transform calculations?
  }

  void remove_object(u64);

  static void init(Pass self_, Scene *scene, PassResourceManager *mgr)
  {
    // create pipeline descriptor sets
    // create sampler
    // create descriptor heap
    // create renderpass
    // fetch pipeline cache
    // async build pipeline
    PBRPass *pbr_pass = (PBRPass *) self_;
  }

  static void deinit(Pass self_, Scene *scene, PassResourceManager *mgr)
  {
  }

  static void update(Pass self_, Scene *scene, PassResourceManager *mgr)
  {
    // re-build renderpass and framebuffer if needed
  }

  static void encode(Pass self_, Scene *scene, PassResourceManager *mgr,
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
//
// TODO(lamarrr): temp-offscreen rendering
//
// antialiased
//
struct RRectObject
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
};

struct RRectMaterialSource
{
  ImageSpan<u8 const> ambient;
};

struct RRectMaterial
{
  gfx::ImageView ambient;
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
// MUST BE rendered in a z-index independent order
// need a way to map from z-index to depth
//
//
// first sort by z_index key from the scene
//
struct RRectPass
{
  u64  add_object(char const *id, RRectObject const &,
                  RRectMaterialSource const &);
  void remove_object(u64);
};

struct ChromaticAberrationPass
{
};

struct FXAAPass
{
};

struct BloomPass
{
};

};        // namespace ash
