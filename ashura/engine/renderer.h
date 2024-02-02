#pragma once
#include "ashura/engine/errors.h"
#include "ashura/engine/utils.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/box.h"
#include "ashura/std/image.h"
#include "ashura/std/option.h"
#include "ashura/std/sparse_set.h"
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

/// @x_mag: The horizontal magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @y_mag: The vertical magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @z_far: The distance to the far clipping plane. This value
/// MUST NOT be equal to zero. zfar MUST be greater than znear.
/// @z_near: The distance to the near clipping plane.
struct Orthographic
{
  f32 x_mag  = 0;
  f32 y_mag  = 0;
  f32 z_far  = 0;
  f32 z_near = 0;
};

/// @aspect_ratio: The aspect ratio of the field of view.
/// @y_fov: The vertical field of view in radians. This value
/// SHOULD be less than π.
/// @z_far: The distance to the far clipping plane.
/// @z_near: The distance to the near clipping plane.
struct Perspective
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
  gfx::CommandEncoderImpl command_encoder = {};
};

/// @brief Arguments to encode the commands to render a batch of objects in a
/// scene.
struct PassEncodeInfo
{
  gfx::CommandEncoderImpl command_encoder = {};
  uid32                   view            = 0;
  bool                    is_transparent  = false;
  i64                     z_index         = 0;
  Span<u64>               object_indices  = {};
};

/// @init: add self and resources to server
/// @deinit: remove self and resources
/// @sort: sort scene objects belonging to passes for efficient batching
/// @encode: encode compute/graphics commands to gpu command encoder
/// @acquire_scene: @acquire_view: new scene/view was added, add resources or
/// begin tracking it
/// @release_scene: @release_view: scene/view was removed, remove associated
/// resources or stop tracking it
struct PassInterface
{
  void (*init)(Pass self, RenderServer *server, uid32 id)             = nullptr;
  void (*deinit)(Pass self, RenderServer *server)                     = nullptr;
  void (*sort)(Pass, RenderServer *server, uid32 scene,
               Span<u64> object_indices)                              = nullptr;
  void (*update)(Pass self, RenderServer *server,
                 PassUpdateInfo const *args)                          = nullptr;
  void (*encode)(Pass self, RenderServer *server,
                 PassEncodeInfo const *args)                          = nullptr;
  void (*acquire_scene)(Pass self, RenderServer *server, uid32 scene) = nullptr;
  void (*release_scene)(Pass self, RenderServer *server, uid32 scene) = nullptr;
  void (*acquire_view)(Pass self, RenderServer *server, uid32 view)   = nullptr;
  void (*release_view)(Pass self, RenderServer *server, uid32 view)   = nullptr;
};

/// can be loaded from a DLL i.e. C++ with C-linkage => DLL
struct PassImpl
{
  char const          *name      = nullptr;
  Pass                 self      = nullptr;
  PassInterface const *interface = nullptr;
};

struct PassGroup
{
  PassImpl      *passes = nullptr;
  SparseSet<u32> id_map = {};
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

struct RenderObjectDesc
{
  Mat4Affine transform      = {};
  Box        aabb           = {};
  i64        z_index        = 0;
  bool       is_transparent = false;
};

struct SceneObjects
{
  SceneNode     *node                       = nullptr;
  Mat4Affine    *local_transform            = nullptr;
  Mat4Affine    *global_transform           = nullptr;
  Box           *aabb                       = nullptr;
  i64           *z_index                    = nullptr;
  u64           *transparency_mask          = nullptr;
  u64           *sort_index                 = nullptr;
  u64            node_capacity              = 0;
  u64            local_transform_capacity   = 0;
  u64            global_transform_capacity  = 0;
  u64            aabb_capacity              = 0;
  u64            z_index_capacity           = 0;
  u64            transparency_mask_capacity = 0;
  u64            sort_index_capacity        = 0;
  SparseSet<u64> id_map                     = {};
};

// A scene repared for rendering
struct Scene
{
  char const       *name                        = nullptr;
  AmbientLight      ambient_light               = {};
  DirectionalLight *directional_lights          = nullptr;
  SparseSet<u32>    directional_lights_id_map   = {};
  PointLight       *point_lights                = nullptr;
  SparseSet<u32>    point_lights_id_map         = {};
  SpotLight        *spot_lights                 = nullptr;
  SparseSet<u32>    spot_lights_id_map          = {};
  AreaLight        *area_lights                 = nullptr;
  SparseSet<u32>    area_lights_id_map          = {};
  u32               directional_lights_capacity = 0;
  u32               point_lights_capacity       = 0;
  u32               spot_lights_capacity        = 0;
  u32               area_lights_capacity        = 0;
  SceneObjects      objects                     = {};

  constexpr u64 num_objects() const
  {
    return objects.id_map.num_valid();
  }
};

struct SceneGroup
{
  Scene         *scenes = nullptr;
  SparseSet<u32> id_map = {};

  constexpr u32 num_scenes() const
  {
    return id_map.num_valid();
  }
};

struct View
{
  char const *name                           = nullptr;
  Camera      camera                         = {};
  uid32       scene                          = 0;
  u64        *object_cull_mask               = nullptr;
  u64        *point_light_cull_mask          = nullptr;
  u64        *spot_light_cull_mask           = nullptr;
  u64        *area_light_cull_mask           = nullptr;
  u64         object_cull_mask_capacity      = 0;
  u32         point_light_cull_mask_capacity = 0;
  u32         spot_light_cull_mask_capacity  = 0;
  u32         area_light_cull_mask_capacity  = 0;
};

struct ViewGroup
{
  View          *views  = nullptr;
  SparseSet<u32> id_map = {};

  constexpr u32 num_views() const
  {
    return id_map.num_valid();
  }
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
/// @acquire_screen_color_image: TODO(lamarrr): how to cache the framebuffer and
/// renderpass and not allocate it for every time the renderpass and
/// framebuffers are requested
/// @add_object: once an object is added to the scene, if it is not at the end
/// of the tree, then the tree should be re-sorted based on depth, resize and
/// iota-fill sort indices, sort indices, resize object cull masks for all views
/// @remove_object: remove object and all its children
struct RenderServer
{
  gfx::DeviceImpl device      = {};
  AllocatorImpl   allocator   = {};
  PassGroup       pass_group  = {};
  SceneGroup      scene_group = {};
  ViewGroup       view_group  = {};

  void                       acquire_screen_color_image();
  void                       acquire_screen_depth_stencil_image();
  void                       release_screen_color_image();
  void                       release_screen_depth_stencil_image();
  Option<PassImpl const *>   get_pass(uid32 pass);
  Option<uid32>              get_pass_id(char const *name);
  Option<uid32>              register_pass(PassImpl pass);
  Option<uid32>              create_scene(char const *name);
  Option<Scene *>            get_scene(uid32 scene);
  void                       remove_scene(uid32 scene);
  Option<uid32>              add_view(uid32 scene, char const *name);
  Option<View *>             get_view(uid32 view);
  void                       remove_view(uid32 view);
  Option<uid64>              add_object(uid32 scene, uid64 parent,
                                        RenderObjectDesc const &desc);
  Option<RenderObjectDesc *> get_object(uid32 scene, uid64 object);
  void                       remove_object(uid32 scene, uid64 object);
  Option<uid32>              add_directional_light(uid32                   scene,
                                                   DirectionalLight const &light);
  Option<uid32>          add_point_light(uid32 scene, PointLight const &light);
  Option<uid32>          add_spot_light(uid32 scene, SpotLight const &light);
  Option<uid32>          add_area_light(uid32 scene, AreaLight const &light);
  Option<AmbientLight *> get_ambient_light(uid32 scene);
  Option<DirectionalLight *> get_directional_light(uid32 scene, uid32 id);
  Option<PointLight *>       get_point_light(uid32 scene, uid32 id);
  Option<SpotLight *>        get_spot_light(uid32 scene, uid32 id);
  Option<AreaLight *>        get_area_light(uid32 scene, uid32 id);
  void                       remove_directional_light(uid32 scene, uid32 id);
  void                       remove_point_light(uid32 scene, uid32 id);
  void                       remove_spot_light(uid32 scene, uid32 id);
  void                       remove_area_light(uid32 scene, uid32 id);
  void                       transform_();
  Result<Void, RenderError>  frustum_cull_();
  Result<Void, RenderError>  sort_();
  Result<Void, RenderError>  render_();
};

}        // namespace ash
