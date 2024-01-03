#pragma once
#include "ashura/gfx.h"
#include "ashura/types.h"

namespace ash
{

// shader and mesh authoring and consuming
// TODO(lamarrr): Box and Rect for canvas 2.0 must have half extents and be from
// the center
constexpr u32 MAXIMUM_SPOT_LIGHTS        = 64;
constexpr u32 MAXIMUM_POINT_LIGHTS       = 64;
constexpr u32 MAXIMUM_DIRECTIONAL_LIGHTS = 64;

typedef struct Box                 Box;
typedef u64                        Pipeline;
typedef u64                        Shader;
typedef u64                        Texture;
typedef u64                        Mesh;
typedef i64                        ZIndex;
typedef struct Material            Material;
typedef struct DirectionalLight    DirectionalLight;
typedef struct PointLight          PointLight;
typedef struct SpotLight           SpotLight;
typedef struct OrthographicCamera  OrthographicCamera;
typedef struct PerspectiveCamera   PerspectiveCamera;
typedef struct Scene               Scene;
typedef struct ReflectionProbe     ReflectionProbe;
typedef struct Bokeh               Bokeh;
typedef struct TAA                 TAA;
typedef struct FXAA                FXAA;
typedef struct Bloom               Bloom;
typedef struct ChromaticAberration ChromaticAberration;

// Skybox?

struct Box
{
  Vec3 begin;
  Vec3 end;
};

struct DirectionalLight
{
  Vec3 direction = {};
};

struct PointLight
{
  Vec3 position    = {};
  f32  attenuation = 0;
};

struct SpotLight
{
  Vec3 postion     = {};
  Vec3 direction   = {};
  f32  cutoff      = 0;
  f32  attenuation = 0;
};
// TODO(lamarrr): light materials, shader?

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
  Vec3       position   = {};
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

// objects are pre-sorted by z-index (if z-index dependent)
// portals are subscenes
// camera data
// light data in resource manager
// we can cull lights
//
//
// TODO(lamarrr): multi-pass object rendering?
//
//
//
// Mat4                local_to_world_transform = {};
// Mat4                world_to_local_transform = {};
//
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
struct RenderResourceManager
{
  // sort by update frequency, per-frame updates, rare-updates, once updates
  // allocate temporary image for rendering
  // renderpasses, framebuffers, pipeline caches, buffer views
  //
  // resource manager
  // static buffer + streaming
  // dynamic buffers + streaming
};
struct SceneGraph
{
  OrthographicCamera *orthographic_cameras     = nullptr;
  PerspectiveCamera  *perspective_cameras      = nullptr;
  DirectionalLight   *directional_lights       = 0;
  PointLight         *point_lights             = 0;
  SpotLight          *spot_lights              = 0;
  u32                 num_orthographic_cameras = 0;
  u32                 num_perspective_cameras  = 0;
  u32                 num_directional_lights   = 0;
  u32                 num_point_lights         = 0;
  u32                 num_spot_lights          = 0;
  u64                 num_objects              = 0;
  u32                 active_camera            = 0;
  u32                 active_camera_type       = 0;
};

typedef struct Pass_T *Pass;
struct PassInterface
{
  void (*init)(Pass self, Scene *scene,
               RenderResourceManager const *mgr)                 = nullptr;
  void (*encode)(Pass self, Scene *scene, RenderResourceManager const *mgr,
                 gfx::CommandEncoderImpl const *command_encoder) = nullptr;
  void (*tick)(Pass self, Scene *scene,
               RenderResourceManager const *mgr)                 = nullptr;
};

struct PassImpl
{
  Pass                 self      = nullptr;
  PassInterface const *interface = nullptr;
};

struct ChromaticAberration
{
};
struct FXAA
{
};

struct Bloom
{
};

// on change, on modification, on stream
// renderer inputs to the scene
struct Scene
{
  Camera              camera                           = {};
  Box                *entities_aabb                    = nullptr;
  u64                 num_entities_aab                 = 0;
  u64                *entities_aab_mask                = nullptr;
  DirectionalLight   *lights                           = nullptr;
  u32                 num_lights                       = 0;
  PassImpl const     *passes                           = nullptr;
  u32                 num_passes                       = 0;
  ChromaticAberration chromatic_abberation             = {};
  FXAA                fxaa                             = {};
  Bloom               bloom                            = {};
  bool                chromatic_abberation_enabled : 1 = false;
  bool                fxaa_enabled : 1                 = false;
  bool                bloom_enabled : 1                = false;

  // TODO(lamarrr): bloom for example requires that the shaders all cooperate
  // and output their emmisive samples to a second output attachment

  u64  add_entity_aab(Box);
  void update_entity_aab(u64);
  void remove_entity_aabb(u64);

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
//

// bounding box or frustum culling
struct RendererTree
{
  // scenes
  // sort by z-index
  // mesh + shaders
  void render(Scene const &scene);
};

};        // namespace ash
