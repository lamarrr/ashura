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

typedef struct Box                     Box;
typedef struct DirectionalLight        DirectionalLight;
typedef struct PointLight              PointLight;
typedef struct SpotLight               SpotLight;
typedef struct OrthographicCamera      OrthographicCamera;
typedef struct PerspectiveCamera       PerspectiveCamera;
typedef struct Scene                   Scene;
typedef struct Renderer                Renderer;
typedef struct BokehPass               BokehPass;
typedef struct TAAPass                 TAAPass;
typedef struct FXAAPass                FXAAPass;
typedef struct BlurPass                BlurPass;
typedef struct BloomPass               BloomPass;
typedef struct ChromaticAberrationPass ChromaticAberrationPass;
typedef struct PBRPass                 PBRPass;

// Skybox?

struct Box
{
  Vec3 offset;
  Vec3 extent;

  constexpr Vec3 center() const
  {
    return offset + (extent / 2);
  }

  constexpr Vec3 end() const
  {
    return offset + extent;
  }
};

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

/// @create: initialize on adding to the renderer
/// @destroy: uninitialize on removing from the renderer
/// @tick: update internal data based on changed information in the scene
/// @encode: encode commands to be sent to the gpu
struct PassInterface
{
  void (*create)(Pass self, Scene *scene, PassResourceManager *mgr)  = nullptr;
  void (*destroy)(Pass self, Scene *scene, PassResourceManager *mgr) = nullptr;
  void (*tick)(Pass self, Scene *scene, PassResourceManager *mgr)    = nullptr;
  void (*encode)(Pass self, Scene *scene, PassResourceManager *mgr,
                 gfx::CommandEncoderImpl command_encoder)            = nullptr;
};

struct PassImpl
{
  Pass                 self      = nullptr;
  PassInterface const *interface = nullptr;
};

struct SceneNode
{
  u64 parent       = 0;
  u64 level        = 0;
  u64 next_sibling = 0;
  u64 first_child  = 0;
};

// on change, on modification, on stream
// renderer inputs to the scene
struct Scene
{
  Camera              camera                    = {};
  OrthographicCamera *orthographic_cameras      = nullptr;
  PerspectiveCamera  *perspective_cameras       = nullptr;
  u32                 num_orthographic_cameras  = 0;
  u32                 num_perspective_cameras   = 0;
  DirectionalLight   *directional_lights        = 0;
  PointLight         *point_lights              = 0;
  SpotLight          *spot_lights               = 0;
  u32                 num_directional_lights    = 0;
  u32                 num_point_lights          = 0;
  u32                 num_spot_lights           = 0;
  PassImpl           *passes                    = nullptr;
  u32                 num_passes                = 0;
  SceneNode          *nodes                     = nullptr;
  Mat4Affine         *node_local_transforms     = nullptr;
  Mat4Affine         *node_global_transforms    = nullptr;
  Mat4Affine         *node_effective_transforms = nullptr;
  Box                *node_aabb                 = nullptr;
  u64                *node_frustum_mask         = nullptr;
  u64                *transform_dirty_mask      = nullptr;
  u64                 num_nodes                 = 0;

  u64  add_entity_aab(Box);
  Box &get_entity_aab(u64);
  void remove_entity_aabb(u64);

  u64      add_pass(PassImpl pass);
  PassImpl get_pass(u64);

  // objects -> mesh + material
  // vfx
  // add_light(); -> update buffers and descriptor sets, increase size of
  // buffers add_camera(); add_object(); remove_light(); remove_camera();
  // remove_object();
  // &get_light();
  // &get_camera();
  // &get_object();
};

// we need
// the mesh and object render-data is mostly pre-configured or modified outside
// the renderer we just need to implement the post-effects and render-orders and
// add other passes on top of the objects
//
//
// what if the shader or source needs the transforms or whatever
//
// on frame begin, pending uploads are first performed
//
// bounding box or frustum culling
struct Renderer
{
  // scenes
  // sort by z-index
  // mesh + shaders
  void render(Scene const &scene);
};

struct PBRMaterial
{
  ImageView<u8 const> ambient           = {};
  ImageView<u8 const> albedo            = {};
  ImageView<u8 const> emmissive         = {};
  ImageView<u8 const> metalic_roughness = {};
  ImageView<u8 const> normal            = {};
};

struct PBRMaterialTextures
{
  gfx::ImageView ambient           = nullptr;
  gfx::ImageView albedo            = nullptr;
  gfx::ImageView emmissive         = nullptr;
  gfx::ImageView metalic_roughness = nullptr;
  gfx::ImageView normal            = nullptr;
};

struct PBRVertex
{
  f32 x = 0, y = 0, z = 0;
  f32 s = 0, t = 0;
};

/// @mesh: static or dynamic?
struct PBREntity
{
  PBRMaterialTextures textures = {};
  u64                 mesh     = 0;
  u64                 aabb     = 0;
};

struct PBRPass
{
  PBREntity               *entities              = nullptr;
  u64                      num_entities          = 0;
  Scene                   *scene                 = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::PipelineCache       pipeline_cache        = nullptr;
  gfx::Sampler             sampler               = nullptr;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;
  gfx::DescriptorHeapImpl  descriptor_heap       = {};
  gfx::RenderPass          render_pass           = nullptr;
  gfx::Framebuffer         framebuffer           = nullptr;

  // add AABB to scene and init material for rendering
  // upload data to gpu, setup scene for it, add AABB, add to pass list
  void add_entity(PBRVertex const *vertices, u64 num_vertices,
                  PBRMaterial const &material);
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
