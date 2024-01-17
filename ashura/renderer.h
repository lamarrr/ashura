#pragma once
#include "ashura/gfx.h"
#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/sparse_set.h"
#include "ashura/trivial_vec.h"
#include "ashura/types.h"
#include "ashura/uid.h"

namespace ash
{
namespace gfx
{

constexpr u32   MAX_SPOT_LIGHTS        = 64;
constexpr u32   MAX_POINT_LIGHTS       = 64;
constexpr u32   MAX_DIRECTIONAL_LIGHTS = 64;
constexpr u32   MAX_NAME_LENGTH        = 128;
constexpr u8    RECT_TOP_LEFT          = 0;
constexpr u8    RECT_TOP_RIGHT         = 1;
constexpr u8    RECT_BOTTOM_RIGHT      = 2;
constexpr u8    RECT_BOTTOM_LEFT       = 3;
constexpr usize MEMORY_POOL_SIZE       = 4096;

typedef char                      Name[MAX_NAME_LENGTH];
typedef struct Texture            Texture;
typedef Vec4                      AmbientLight;
typedef struct DirectionalLight   DirectionalLight;
typedef struct PointLight         PointLight;
typedef struct SpotLight          SpotLight;
typedef struct OrthographicCamera OrthographicCamera;
typedef struct PerspectiveCamera  PerspectiveCamera;
typedef struct ResourceManager    ResourceManager;
typedef struct Pass_T            *Pass;
typedef struct PassInterface      PassInterface;
typedef struct PassImpl           PassImpl;
typedef struct RenderObject       RenderObject;
typedef struct Scene              Scene;
typedef struct SceneGroup         SceneGroup;
typedef struct View               View;
typedef struct ViewGroup          ViewGroup;
typedef struct Renderer           Renderer;
typedef i8 (*RenderObjectCmp)(Pass, SceneGroup *, u32, u64 a, u64 b);

struct Texture
{
  ImageView view = nullptr;
  Vec2      uv0  = {};
  Vec2      uv1  = {};
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

/// Manages and uploads render resources to the GPU.
struct ResourceManager
{
  AllocatorImpl allocator = {};
  // sort by update frequency, per-frame updates, rare-updates
  // allocate temporary image for rendering
  // renderpasses, framebuffers, pipeline caches,async pipeline cache loader and
  // pipeline builder, buffer views
  //
  //
  // resource manager
  // static buffer + streaming
  // dynamic buffers + streaming
  //
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
  //
  // usage tracking
  // - we can create a single image and just re-use it depending on
  // - the components/aspects we need to use for each type of pass
  //
  // UNIFORM COLOR Texture cache with image component swizzling. Only 1 white
  // RGBA texture is needed.
  //
  //
  // on frame begin, pending uploads are first performed
  //
};

/// @brief: Arguments to allocate new resources or update existing resources
/// based on the changed state of the scene. called at the beginning of the
/// frame. i.e. camera buffers, transform buffers, staging buffers.
/// can also be used for resource management, i.e. ring buffers of per-frame
/// resources.
struct PassUpdateInfo
{
  ResourceManager   *mgr             = nullptr;
  CommandEncoderImpl command_encoder = {};
  ViewGroup const   *view_group      = nullptr;
};

/// @brief Arguments to encode the commands to render a batch of objects in a
/// scene.
/// @first_scene_object: pull from z_ordered index
struct PassEncodeInfo
{
  ResourceManager   *mgr                = nullptr;
  CommandEncoderImpl command_encoder    = {};
  ViewGroup const   *view_group         = nullptr;
  uid32              view               = 0;
  i64                z_index            = 0;
  bool               is_transparent     = false;
  u64                first_scene_object = 0;
  u64                num_scene_objects  = 0;
};

/// @init: add self and resources
/// @deinit: remove self and resources
struct PassInterface
{
  Pass (*create)(ResourceManager *mgr)                  = nullptr;
  void (*destroy)(Pass self, ResourceManager *mgr)      = nullptr;
  void (*init)(Pass self, ResourceManager *mgr)         = nullptr;
  RenderObjectCmp (*get_cmp)(Pass self)                 = nullptr;
  void (*deinit)(Pass self, ResourceManager *mgr)       = nullptr;
  void (*update)(Pass self, PassUpdateInfo const &args) = nullptr;
  void (*encode)(Pass self, PassEncodeInfo const &args) = nullptr;
};

/// can be loaded from a DLL i.e. C++ with C-linkage => DLL
struct PassImpl
{
  Pass                 self      = nullptr;
  PassInterface const *interface = nullptr;
};

// full-screen post-fx passes are full-screen quads with dependency determined
// by their z-indexes.
// HUD is a full-screen quad of a view-pass (another scene).
//
// world->[capture->world]->post-fx->hud->[capture->hud]
// use z-indexes with full screen quad?
// how to project from object-space to full-screen space
//
// i.e. world scene pass -> post-fx pass -> HUD pass
//
/// linearly-tilted tree node
/// @pass: pass to be used to render this object. only one pass is responsible
/// for rendering an object.
struct RenderObject
{
  uid64 parent       = 0;
  uid64 next_sibling = 0;
  uid64 first_child  = 0;
  u32   level        = 0;
  uid32 pass         = 0;
  uid64 pass_object  = 0;
};

/// @is_camera_space: if the object's coordinates are in camera space.???????
struct RenderObjectDesc
{
  Mat4Affine local_transform = {};
  Box        aabb            = {};
  i64        z_index         = 0;
  bool       is_camera_space = false;
  bool       is_transparent  = false;
};

// A scene repared for rendering
//
// NOTE: the Scene's memory usage never shrinks, only grows. it is re-used.
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
// - for the z-index group sorted objects with the same passes, sort using the
// PassCmp key
// - for each partition, invoke the pass with the objects
//
// Area lights: https://learnopengl.com/Guest-Articles/2022/Area-Lights
//
// Unit is -1 to +1 for x,y,z. will help with objects that cover the whole
// scene.
// will be scaled to the screen dimensions eventually.
//
struct Scene
{
  AmbientLight      ambient_light               = {};
  DirectionalLight *directional_lights          = 0;
  PointLight       *point_lights                = 0;
  SpotLight        *spot_lights                 = 0;
  bool              lights_dirty_mask           = false;
  u32               num_directional_lights      = 0;
  u32               num_point_lights            = 0;
  u32               num_spot_lights             = 0;
  RenderObject     *object_nodes                = nullptr;
  Mat4Affine       *object_local_transforms     = nullptr;
  u64              *camera_space_mask           = nullptr;
  Box              *object_aabb                 = nullptr;
  i64              *object_z_index              = nullptr;
  u64              *object_transform_dirty_mask = nullptr;
  u64              *object_transparency_mask    = nullptr;
  u64              *objects_sorted              = nullptr;
  u64              *object_ids_map              = nullptr;
  uid64            *free_object_ids             = nullptr;
  u64               num_objects                 = 0;

  uid64 add_object(RenderObjectDesc const &);
  void  remove_object(uid64);
  void  change_ambient_light();
  void  add_directional_light();
  void  add_point_light();
  void  add_spot_light();
  void  remove_directional_light();
  void  remove_point_light();
  void  remove_spot_light();
};

// scene dependency? not explicitly expressed. left to the pass processor to
// decide which scene to render and when scene pointer and ids can be re-used
struct SceneGroup
{
  Scene *scenes = nullptr;
  Name  *names  = nullptr;
  u32   *id_map = nullptr;
  u32    num    = 0;
};

// sized to screen size or lower if specified??? STILL Not good. we need to
// prevent resizing as much as possible. this will also help with zooming for
// example
struct RenderTarget
{
  Format      color_format         = Format::Undefined;
  Image       color_image          = nullptr;
  Format      depth_stencil_format = Format::Undefined;
  Image       depth_stencil_image  = nullptr;
  RenderPass  render_pass          = nullptr;
  Framebuffer framebuffer          = nullptr;
};

// each view can have attachments for each pass
/// camera should be assumed to change every frame
struct View
{
  Camera      camera           = {};
  SceneGroup *scene_group      = nullptr;
  uid32       scene            = 0;
  u64        *object_cull_mask = nullptr;
};

struct ViewGroup
{
  View         *views          = nullptr;
  RenderTarget *render_targets = nullptr;
  Name         *names          = nullptr;
  u32          *id_map         = nullptr;
  u32           num            = 0;
};

// sort objects by z-index, get min and max z-index
// for all objects in the z-index range, invoke the passes pass->encode(z_index,
// begin_objects, num_objects)
///
/// passes are built at program startup and never change.
///
///
struct Renderer
{
  PassImpl        *passes             = nullptr;
  RenderObjectCmp *render_object_cmps = nullptr;
  u32             *id_map             = nullptr;
  Name            *pass_names         = nullptr;
  u32              num_passes         = 0;

  void            init(Renderer *renderer, Span<char const *const> pass_names,
                       Span<PassImpl const> passes);
  PassImpl const *get_pass(uid32);
  Name const     *get_pass_name(uid32);
  PassImpl const *get_pass_by_name(char const *pass_name);

  // we need the mesh and object render-data is mostly pre-configured or
  // modified outside the renderer we just need to implement the post-effects
  // and render-orders and add other passes on top of the objects
  //
  // each scene is rendered and composited onto one another? can this possibly
  // work for portals?
  void render(ResourceManager *mgr, ViewGroup const *group, u32 view);

  // remove all resources associated with a scene object.
  // void remove_scene
  // void add_scene
  // void add_view
  // void remove_view
  //
  //
  // perform frustum and occlusion culling of objects and light (in the same
  // z-index) then cull by z-index???? Z-index not needed in culling
  // occlussion culling only happens when a fully-opaque object occludes another
  // object.
  //
  // cull lights by camera frustum
  //
  // https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron/blob/b92d559bd083f44df9f8f42a6ad149c1584ae94c/src/common/Misc/Misc.cpp#L265
  void cull(ResourceManager *mgr, ViewGroup const *group);

  // also calls PassObjectCmp to sort all objects belonging to a pass invocation
  void sort(ResourceManager *mgr, SceneGroup const *scene_group);
};

// needed because we need to be able to render a view that is part of another
// view without adding the elements of the view to the root view
struct ViewPass
{
  // render to view's frame buffer and then composite onto the present view
  // there must be no recursion happening here
  uid32 view;
};

}        // namespace gfx
}        // namespace ash
