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
typedef struct Object              Object;
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
  Vec3 center;
  Vec3 half_extent;
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
  Mat4Affine model;
  Mat4Affine view;
  Mat4Affine projection;
};

struct ComputePass
{
};

struct RenderPass
{
};

struct Pass
{
  u64  buffers[2];
  u64  images[2];
  u64  shaders[2];
  u64  pre;
  u64  post;
  bool is_compute = false;
};

// should offscreen rendering be performed by objects??? shouldn't they be a
// different viewport? what if it is once per frame for example
struct Object
{
  // mesh, shaders, pipelines, buffers, images, imageviews, bufferviews
  Mat4 transform = {};        // local ? global? // bounding box instead?
  Vec4 position  = {};        // should contain id of buffer and index into
  // buffer to source data from. only data for culling is needed?
  // clip texture? compute pre-pass, etc
  u64 pass;
};

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
// buffer = encode(scene->get_lights());
// descriptor_set->ref(buffer);
//
// 
// how object culling will work?
//
// i.e. add_object_pass( [scene, descriptor_set](command_buffer,
// u64 const * cull_mask  ){
//  
//  if(cull_mask[...] >> ... & 1){
//  command_buffer->bind_descriptor_sets(descriptor_set);
//  command_buffer->draw();
//  }
//
//
// }  );
//
//
//
struct Scene
{
  // camera positions
  OrthographicCamera *orthographic_cameras     = nullptr;
  PerspectiveCamera  *perspective_cameras      = nullptr;
  DirectionalLight   *directional_lights       = 0;
  PointLight         *point_lights             = 0;
  SpotLight          *spot_lights              = 0;
  Object             *objects                  = nullptr;
  u32                 num_orthographic_cameras = 0;
  u32                 num_perspective_cameras  = 0;
  u32                 num_directional_lights   = 0;
  u32                 num_point_lights         = 0;
  u32                 num_spot_lights          = 0;
  u64                 num_objects              = 0;
  u32                 active_camera            = 0;
  u32                 active_camera_type       = 0;
  Scene              *portals                  = nullptr;
  u32                 num_portals              = 0;
  // portal camera (positions + directions)
  // portal bounding boxes
  // objects -> mesh + material
  // vfx

  // add_light();
  // add_camera();
  // add_object();
  // remove_light();
  // remove_camera();
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

struct SceneManager
{
  Scene                 *render_scene;
  RenderResourceManager *resource_manager;
};

// bounding box or frustum culling
struct Renderer
{
  // scenes
  // sort by z-index
  // mesh + shaders
  void render(Scene const &scene);
};

};        // namespace ash
