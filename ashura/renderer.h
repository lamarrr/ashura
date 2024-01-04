#pragma once
#include "ashura/gfx.h"
#include "ashura/image.h"
#include "ashura/types.h"

namespace ash
{

// shader and mesh authoring and consuming
constexpr u32 MAXIMUM_SPOT_LIGHTS        = 64;
constexpr u32 MAXIMUM_POINT_LIGHTS       = 64;
constexpr u32 MAXIMUM_DIRECTIONAL_LIGHTS = 64;
constexpr u32 MAXIMUM_ID_LENGTH          = 64;

typedef struct Box                     Box;
typedef struct DirectionalLight        DirectionalLight;
typedef struct PointLight              PointLight;
typedef struct SpotLight               SpotLight;
typedef struct OrthographicCamera      OrthographicCamera;
typedef struct PerspectiveCamera       PerspectiveCamera;
typedef struct Scene                   Scene;
typedef struct Renderer                Renderer;
typedef struct PBRPass                 PBRPass;
typedef struct BlurPass                BlurPass;
typedef struct BloomPass               BloomPass;
typedef struct TAAPass                 TAAPass;
typedef struct FXAAPass                FXAAPass;
typedef struct BokehPass               BokehPass;
typedef struct ChromaticAberrationPass ChromaticAberrationPass;
typedef char                           Id[MAXIMUM_ID_LENGTH];

// Skybox?

struct DirectionalLight
{
  Vec3 direction = {};
  Vec4 color     = {};
};

struct PointLight
{
  Vec3 position    = {};
  f32  attenuation = 0;
  Vec4 color       = {};
};

// TODO(lamarrr): light materials, shader?
struct SpotLight
{
  Vec3 position    = {};
  Vec3 direction   = {};
  f32  cutoff      = 0;
  f32  attenuation = 0;
  Vec4 color       = {};
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
// struct Object
// {
// mesh, shaders, pipelines, buffers, images, imageviews, bufferviews
// Mat4 transform = {};        // local ? global? // bounding box instead?
// Vec4 position  = {};        // should contain id of buffer and index into
// buffer to source data from. only data for culling is needed?
// clip texture? compute pre-pass, etc
// };
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
  // sort by update frequency, per-frame updates, rare-updates, once updates
  // allocate temporary image for rendering
  // renderpasses, framebuffers, pipeline caches, buffer views
  //
  // resource manager
  // static buffer + streaming
  // dynamic buffers + streaming
  //
  //
  // UNIFORM COLOR Texture cache
  //
  //
};

typedef struct Pass_T       *Pass;
typedef struct PassInterface PassInterface;
typedef struct PassImpl      PassImpl;

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
                 gfx::CommandEncoderImpl command_encoder)           = nullptr;
};

struct PassImpl
{
  Pass                 self      = nullptr;
  PassInterface const *interface = nullptr;
};

/// tilted tree
struct ObjectNode
{
  u64 parent       = 0;
  u64 level        = 0;
  u64 next_sibling = 0;
  u64 first_child  = 0;
};

// TODO(lamarrr): this still doesn't solve the custom shader problem?
struct PassNode
{
  u64 pass  = 0;
  u64 level = 0;
  u64 child = 0;
};

// on change, on modification, on stream
// renderer inputs to the scene
//
// camera should be assumed to change every frame
//
// Passes are static and not per-object?
struct Scene
{
  Camera              camera                      = {};
  OrthographicCamera *orthographic_cameras        = nullptr;
  PerspectiveCamera  *perspective_cameras         = nullptr;
  u32                 num_orthographic_cameras    = 0;
  u32                 num_perspective_cameras     = 0;
  DirectionalLight   *directional_lights          = 0;
  PointLight         *point_lights                = 0;
  SpotLight          *spot_lights                 = 0;
  u32                 num_directional_lights      = 0;
  u32                 num_point_lights            = 0;
  u32                 num_spot_lights             = 0;
  PassImpl           *passes                      = nullptr;
  PassNode           *pass_nodes                  = nullptr;
  u64                 num_passes                  = 0;
  ObjectNode         *object_nodes                = nullptr;
  Mat4Affine         *object_local_transforms     = nullptr;
  Mat4Affine         *object_global_transforms    = nullptr;
  Mat4Affine         *object_effective_transforms = nullptr;
  Box                *object_aabb                 = nullptr;
  u64                *object_transform_dirty_mask = nullptr;
  u64                *object_cull_mask            = nullptr;
  u64                 num_objects                 = 0;
  Id                 *orthographic_cameras_id     = nullptr;
  Id                 *perspective_cameras_id      = nullptr;
  Id                 *directional_lights_id       = nullptr;
  Id                 *point_lights_id             = nullptr;
  Id                 *spot_lights_id              = nullptr;
  Id                 *passes_id                   = nullptr;
  Id                 *object_nodes_id             = nullptr;
  bool                lights_dirty_mask           = false;

  u64      add_aabb(Box);
  Box     &get_aabb(u64);
  void     remove_aabb(u64);
  u64      add_pass(char const *pass_id, PassImpl pass);
  PassImpl get_pass(u64);
  PassImpl get_pass_by_id(char const *pass_id);
  // void     frustum_cull();
  // void occlusion culling
  // scene lights_culling by camera
  // object lights_culling if affected by light
  //
  // vfx
  // add_light(); -> update buffers and descriptor sets, increase size of

  // we need
  // the mesh and object render-data is mostly pre-configured or modified
  // outside the renderer we just need to implement the post-effects and
  // render-orders and add other passes on top of the objects
  //
  //
  // what if the shader or source needs the transforms or whatever
  //
  // on frame begin, pending uploads are first performed
  //
  // bounding box or frustum culling
  // scenes
  // sort by z-index
  // mesh + shaders
  void render();
};

struct PBRMaterial
{
  ImageView<u8 const> albedo             = {};
  ImageView<u8 const> ambient            = {};
  ImageView<u8 const> emmissive          = {};
  ImageView<u8 const> metallic_roughness = {};
  ImageView<u8 const> normal             = {};
};

struct PBRTextures
{
  gfx::ImageView albedo             = nullptr;
  gfx::ImageView ambient            = nullptr;
  gfx::ImageView emmissive          = nullptr;
  gfx::ImageView metallic_roughness = nullptr;
  gfx::ImageView normal             = nullptr;
};

struct PBRVertex
{
  f32 x = 0, y = 0, z = 0;
  f32 s = 0, t = 0;
};

struct PBRMesh
{
  u32 buffer      = 0;
  u32 first_index = 0;
  u32 num_indices = 0;
};

struct PBRObject
{
  PBRTextures textures   = {};
  PBRMesh     mesh       = {};
  u64         scene_node = 0;
};

//
//
// TODO(lamarrr): custom passes?
//
//
//
//
// TODO(lamarrr): what texture to render to
// PBR meshes are always static
// static scene mesh?
//
// TODO(lamarrr): sort opaque objects by materials and textures and resources to
// minimize pipeline state changes
//
// for renderer 2d, we can sort by z-index and also perform batching of draw
// shapes of the same type
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
  u64 add_object(PBRVertex const *vertices, u64 num_vertices,
                 PBRMaterial const &material)
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

// can be loaded from a DLL i.e. C++ with C-linkage => Clang IR => DLL
// can be per-object or global
//
// we consult the passes to execute, not the objects. what about with culling on
// others?
//
struct CustomPass
{
  // add_object
  // encode, etc.
};

// needs to be 3d as we need to add to be able to add it to the scene
struct UIObject
{
};

// normal 2d objects
// z_index mapped to clip space, occlusion culling + batching
// custom objects + custom passes?
// offscreen passes
//
// we can't use a linear draw list anymore
// just straight-up frustum culling + occlusion culling +
//
//
// CUSTOM SHADERS + CUSTOM PASSES
//
struct UIPass
{
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
