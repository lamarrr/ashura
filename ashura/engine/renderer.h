#pragma once
#include "ashura/engine/utils.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/box.h"
#include "ashura/std/fn.h"
#include "ashura/std/image.h"
#include "ashura/std/rect.h"
#include "ashura/std/sparse_set.h"
#include "ashura/std/trivial_vec.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct Texture            Texture;
typedef Vec4                      AmbientLight;
typedef struct DirectionalLight   DirectionalLight;
typedef struct PointLight         PointLight;
typedef struct SpotLight          SpotLight;
typedef struct AreaLight          AreaLight;
typedef struct OrthographicCamera OrthographicCamera;
typedef struct PerspectiveCamera  PerspectiveCamera;
typedef struct RenderServer       RenderServer;
typedef struct Pass_T            *Pass;
typedef struct PassInterface      PassInterface;
typedef struct PassImpl           PassImpl;
typedef struct PassGroup          PassGroup;
typedef struct SceneNode          SceneNode;
typedef struct Scene              Scene;
typedef struct SceneGroup         SceneGroup;
typedef struct View               View;
typedef struct ViewGroup          ViewGroup;
typedef struct Renderer           Renderer;

struct Texture
{
  gfx::ImageView view = nullptr;
  Vec2           uv0  = {};
  Vec2           uv1  = {};
};

struct DirectionalLight
{
  Vec3 direction = {};
  Vec4 color     = {};
};

struct PointLight
{
  Vec4 color       = {};
  Vec3 position    = {};
  f32  attenuation = 0;
};

struct SpotLight
{
  Vec3 direction   = {};
  f32  cutoff      = 0;
  f32  attenuation = 0;
  Vec4 color       = {};
  Vec3 position    = {};
};

/// SEE: https://learnopengl.com/Guest-Articles/2022/Area-Lights
struct AreaLight
{
  Vec3 color     = {};
  Vec3 position  = {};
  Vec3 extent    = {};
  f32  intensity = 0;
  bool two_sided = false;
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

/// @brief: Arguments to allocate new resources or update existing resources
/// based on the changed state of the scene. called at the beginning of the
/// frame. i.e. camera buffers, transform buffers, staging buffers.
/// can also be used for resource management, i.e. ring buffers of per-frame
/// resources.
struct PassUpdateInfo
{
  RenderServer           *server          = nullptr;
  gfx::CommandEncoderImpl command_encoder = {};
};

/// @brief Arguments to encode the commands to render a batch of objects in a
/// scene.
/// @first_scene_object: pull from z_ordered index
struct PassEncodeInfo
{
  RenderServer           *server          = nullptr;
  gfx::CommandEncoderImpl command_encoder = {};
  uid32                   view            = 0;
  bool                    is_transparent  = false;
  i64                     z_index         = 0;
  Span<u64>               object_indices  = {};
};

/// @init: add self and resources
/// @deinit: remove self and resources
struct PassInterface
{
  void (*init)(Pass self, RenderServer *server, uid32 id)             = nullptr;
  void (*deinit)(Pass self, RenderServer *server)                     = nullptr;
  void (*sort_objects)(Pass, RenderServer *server, uid32 scene,
                       Span<u64> object_indices)                      = nullptr;
  void (*update)(Pass self, PassUpdateInfo const &args)               = nullptr;
  void (*encode)(Pass self, PassEncodeInfo const &args)               = nullptr;
  void (*acquire_scene)(Pass self, RenderServer *server, uid32 scene) = nullptr;
  void (*release_scene)(Pass self, RenderServer *server, uid32 scene) = nullptr;
  void (*acquire_view)(Pass self, RenderServer *server, uid32 view)   = nullptr;
  void (*release_view)(Pass self, RenderServer *server, uid32 view)   = nullptr;
};

/// can be loaded from a DLL i.e. C++ with C-linkage => DLL
struct PassImpl
{
  Pass                 self      = nullptr;
  PassInterface const *interface = nullptr;
};

struct PassGroup
{
  PassImpl      *passes     = nullptr;
  char const   **pass_names = nullptr;
  SparseSet<u32> id_map     = {};
};

// full-screen post-fx passes are full-screen quads with dependency determined
// by their z-indexes.
// HUD is a full-screen quad of a view-pass (another scene).
//
// world->[capture->world]->post-fx->hud->[capture->hud]
// how to project from object-space to full-screen space
//
// i.e. world scene pass -> post-fx pass -> HUD pass
//
/// linearly-tilted tree node
/// @depth: depth of the tree this node belongs to. there's ever only one root
/// node at depth 0
/// @pass: pass to be used to render this object
struct SceneNode
{
  uid64 parent         = INVALID_UID64;
  uid64 next_sibling   = INVALID_UID64;
  uid64 first_child    = INVALID_UID64;
  u32   depth          = 0;
  uid32 pass           = INVALID_UID32;
  uid64 pass_object_id = INVALID_UID64;
};

// TODO(lamarrr): dispatch sorting by passes?
struct RenderObjectDesc
{
  Mat4Affine transform      = {};
  Box        aabb           = {};
  i64        z_index        = 0;
  bool       is_transparent = false;
};

// A scene repared for rendering
//
// Unit is -1 to +1 for x,y,z. will help with objects that cover the whole
// scene.
// will be scaled to the screen dimensions eventually.
struct Scene
{
  AmbientLight      ambient_light               = {};
  DirectionalLight *directional_lights          = nullptr;
  PointLight       *point_lights                = nullptr;
  SpotLight        *spot_lights                 = nullptr;
  AreaLight        *area_lights                 = nullptr;
  SceneNode        *object_nodes                = nullptr;
  Mat4Affine       *object_local_transforms     = nullptr;
  Mat4Affine       *object_global_transforms    = nullptr;
  Box              *object_aabb                 = nullptr;
  i64              *object_z_index              = nullptr;
  u64              *object_transparency_mask    = nullptr;
  u64              *objects_sorted              = nullptr;
  u16               directional_lights_capacity = 0;
  u16               point_lights_capacity       = 0;
  u16               spot_lights_capacity        = 0;
  u16               area_lights_capacity        = 0;
  u64               objects_capacity            = 0;
  SparseSet<u16>    directional_lights_id_map   = {};
  SparseSet<u16>    point_lights_id_map         = {};
  SparseSet<u16>    spot_lights_id_map          = {};
  SparseSet<u16>    area_lights_id_map          = {};
  SparseSet<u64>    objects_id_map              = {};
  uid64             root_object                 = INVALID_UID64;
};

struct SceneGroup
{
  Scene         *scenes = nullptr;
  char const   **names  = nullptr;
  SparseSet<u32> id_map = {};
};

struct View
{
  Camera camera           = {};
  uid32  scene            = 0;
  u64   *object_cull_mask = nullptr;
  // todo(lamarrr): effective object clip space volume, used for frustum culling
};

struct ViewGroup
{
  View          *views  = nullptr;
  char const   **names  = nullptr;
  SparseSet<u32> id_map = {};
};

// sort by update frequency, per-frame updates, rare-updates
//
// resource manager
// static buffer + streaming
// dynamic buffers + streaming
//
// mapping of color and depth components?
//
// it should have a buffer of MAX_SWAPCHAIN_IMAGES it cycles from to prevent
// stalling
//
// full-screen depth stencil image
// full-screen color image
// scratch full-screen depth stencil image
// scratch full-screen color image
// allocate/deallocate - for re-use by others
//
// usage tracking
// - we can create a single image and just re-use it depending on the
// components/aspects we need to use for each type of pass
//
// UNIFORM COLOR Texture cache with image component swizzling. Only 1 white
// RGBA texture is needed.
//
// on frame begin, pending uploads are first performed
//
/// Manages and uploads render resources to the GPU.
///
/// @remove_scene: remove all pass resources associated with a scene object.
struct RenderServer
{
  gfx::DeviceImpl device      = {};
  AllocatorImpl   allocator   = {};
  PassGroup      *pass_group  = nullptr;
  SceneGroup     *scene_group = nullptr;
  ViewGroup      *view_group  = nullptr;

  // TODO(lamarrr): how to cache the framebuffer and renderpass and not allocate
  // it for every time the renderpass and framebuffers are requested
  void acquire_screen_color_image();
  void acquire_screen_depth_stencil_image();
  void release_screen_color_image();
  void release_screen_depth_stencil_image();

  PassImpl const *get_pass(uid32 pass);
  uid32           get_pass_id(char const *name);
  char const     *get_pass_name(uid32 pass);
  PassImpl const *get_pass_by_name(char const *name);

  uid32 add_scene(Scene const &, char const *name);
  void  remove_scene(uid32 scene);
  uid32 add_view(View const &, char const *name);
  void  remove_view(uid32 scene);

  Camera *get_view_camera(uid32 view);

// once an object is added to the scene, if it is not at the end of the tree, then the tree should be re-sorted based on depth
  uid64 add_object(uid32 scene, RenderObjectDesc const &, uid64 parent);
  RenderObjectDesc *get_object(uid32 scene, uid64 object);
  // remove object and all its children
  void              remove_object(uid32 scene, uid64 object);
  uid32             add_directional_light(uid32 scene);
  uid32             add_point_light(uid32 scene);
  uid32             add_spot_light(uid32 scene);
  uid32             add_area_light(uid32 scene);
  DirectionalLight *get_directional_light(uid32 scene, uid32 light);
  PointLight       *get_point_light(uid32 scene, uid32 light);
  SpotLight        *get_spot_light(uid32 scene, uid32 light);
  AreaLight        *get_area_light(uid32 scene, uid32 light);
  void              remove_directional_light(uid32 scene, uid32 light);
  void              remove_point_light(uid32 scene, uid32 light);
  void              remove_spot_light(uid32 scene, uid32 light);
  void              remove_area_light(uid32 scene, uid32 light);
};

/// transform->frustum_cull->occlusion_cull->sort->render
///
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
// - for the z-index group sorted objects with the same passes, sort using the
// PassCmp key
// - for each partition, invoke the pass with the objects
struct Renderer
{
  // transform views from object-space to world space then to clip space using
  // view's camera
  static void transform(RenderServer *server);

  // perform frustum and occlusion culling of objects and light (in the same
  // z-index) then cull by z-index???? Z-index not needed in culling
  // occlussion culling only happens when a fully-opaque object occludes another
  // object.
  //
  // cull lights by camera frustum
  //
  // https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron/blob/b92d559bd083f44df9f8f42a6ad149c1584ae94c/src/common/Misc/Misc.cpp#L265
  static void frustum_cull(RenderServer *server);
  static void occlussion_cull(RenderServer *server);

  // sort objects by z-index, get min and max z-index
  // for all objects in the z-index range, invoke the passes
  // pass->encode(z_index, begin_objects, num_objects)
  //
  // also calls PassObjectCmp to sort all objects belonging to a pass invocation
  // sort by z-index
  // sort by transparency, transparent objects last
  // sort by pass sort key
  // sort by depth?
  static void sort(RenderServer *server);

  // we need the mesh and object render-data is mostly pre-configured or
  // modified outside the renderer we just need to implement the post-effects
  // and render-orders and add other passes on top of the objects
  //
  // each scene is rendered and composited onto one another? can this possibly
  // work for portals?
  static void render(RenderServer *server);
};

}        // namespace ash
