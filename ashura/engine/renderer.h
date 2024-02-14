#pragma once
#include "ashura/engine/errors.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/box.h"
#include "ashura/std/dict.h"
#include "ashura/std/image.h"
#include "ashura/std/option.h"
#include "ashura/std/sparse_vec.h"
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
typedef struct PassSortInfo       PassSortInfo;
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
  Mat4       projection = {};
};

/// @brief Arguments to encode the commands to render a batch of objects in a
/// scene.
struct PassEncodeInfo
{
  gfx::CommandEncoderImpl command_encoder = {};
  bool                    is_transparent  = false;
  i64                     z_index         = 0;
  Span<u32>               indices         = {};
};

// TODO(lamarrr): multi-recursive passes, and how to know when to begin and end
// passes, i.e. begin render pass, end render pass
//
//
// TODO(lamarrr): view pass recursive render
//
//
// full-screen post-fx passes are full-screen quads with dependency
// determined by their z-indexes. HUD is a full-screen quad of a view-pass
// (another scene).
//
// world->[capture->world]->post-fx->hud->[capture->hud]
// how to project from object-space to full-screen space
//
// i.e. world scene pass -> post-fx pass -> HUD pass
//
// how to queue screen image resize
//
//
/// @init: add self and resources to server
/// @deinit: remove self and resources
/// @sort: sort scene objects belonging to passes for efficient batching
/// @encode: encode compute/graphics commands to gpu command encoder
/// @acquire_scene: @acquire_view: new scene/view was added, add resources or
/// begin tracking it
/// @release_scene: @release_view: scene/view was removed, remove associated
/// resources or stop tracking it
///
///
/// @begin: allocate new resources or update existing resources
/// based on the changed state of the scene. called at the beginning of the
/// frame. i.e. camera buffers, transform buffers, staging buffers.
/// can also be used for resource management, i.e. ring buffers of per-frame
/// resources.
struct PassInterface
{
  void (*init)(Pass self, RenderServer *server, uid32 id)             = nullptr;
  void (*deinit)(Pass self, RenderServer *server)                     = nullptr;
  void (*acquire_scene)(Pass self, RenderServer *server, uid32 scene) = nullptr;
  void (*release_scene)(Pass self, RenderServer *server, uid32 scene) = nullptr;
  void (*acquire_view)(Pass self, RenderServer *server, uid32 view)   = nullptr;
  void (*release_view)(Pass self, RenderServer *server, uid32 view)   = nullptr;
  void (*release_object)(Pass self, RenderServer *server, uid32 scene,
                         uid32 object)                                = nullptr;
  void (*begin)(Pass self, RenderServer *server, uid32 view,
                gfx::CommandEncoderImpl const *encoder)               = nullptr;
  void (*encode)(Pass self, RenderServer *server, uid32 view,
                 PassEncodeInfo const *info)                          = nullptr;
  void (*end)(Pass self, RenderServer *server, uid32 view,
              gfx::CommandEncoderImpl const *encoder)                 = nullptr;
};

/// can be loaded from a DLL i.e. C++ with C-linkage => DLL
struct PassImpl
{
  Span<char const>     name      = {};
  Pass                 self      = nullptr;
  PassInterface const *interface = nullptr;
};

struct PassGroup
{
  Vec<PassImpl>  passes = {};
  SparseVec<u32> id_map = {};
};

/// linearly-tilted tree node
/// @depth: depth of the tree this node belongs to. there's ever only one root
/// node at depth 0
/// @pass: pass to be used to render this object
struct SceneNode
{
  uid32 parent       = UID32_INVALID;
  uid32 next_sibling = UID32_INVALID;
  uid32 first_child  = UID32_INVALID;
  u32   depth        = 0;
  uid32 pass         = UID32_INVALID;
  uid32 pass_object  = UID32_INVALID;
};

struct SceneObjectDesc
{
  Mat4Affine transform      = {};
  Box        aabb           = {};
  i64        z_index        = 0;
  bool       is_transparent = false;
};

struct SceneObjects
{
  Vec<SceneNode>   node             = {};
  Vec<Mat4Affine>  local_transform  = {};
  Vec<Mat4Affine>  global_transform = {};
  Vec<Box>         aabb             = {};
  Vec<i64>         z_index          = {};
  BitVec<u64>      is_transparent   = {};
  SparseVec<uid32> id_map           = {};
};

struct Scene
{
  Span<char const>      name                      = {};
  AmbientLight          ambient_light             = {};
  Vec<DirectionalLight> directional_lights        = {};
  SparseVec<u32>        directional_lights_id_map = {};
  Vec<PointLight>       point_lights              = {};
  SparseVec<u32>        point_lights_id_map       = {};
  Vec<SpotLight>        spot_lights               = {};
  SparseVec<u32>        spot_lights_id_map        = {};
  Vec<AreaLight>        area_lights               = {};
  SparseVec<u32>        area_lights_id_map        = {};
  SceneObjects          objects                   = {};
};

struct SceneGroup
{
  Vec<Scene>     scenes = {};
  SparseVec<u32> id_map = {};
};

/// @sub_batch_sizes: this is the number of sub-batches of an object determined
/// by the pass
/// @batch_sizes: number of sub-batches in a batch of a pass, these are indices
/// of objects sharing common passes
struct View
{
  Span<char const>   name              = {};
  Camera             camera            = {};
  uid32              scene             = 0;
  BitVec<u64>        is_object_visible = {};
  Vec<u32>           sort_indices      = {};
  StrHashMap<void *> resources         = {};
};

struct ViewGroup
{
  Vec<View>      views     = {};
  SparseVec<u32> id_map    = {};
  uid32          root_view = UID32_INVALID;
};

// TODO(lamarrr):
// sort by update frequency, per-frame updates, rare-updates
//
// resource manager
// static buffer + streaming
// dynamic buffers + streaming
//
// mapping of color and depth components?
//
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
/// @acquire_screen_color_image:
///
///
/// TODO(lamarrr): how to cache the framebuffer and
/// renderpass and not allocate it for every time the renderpass and
/// framebuffers are requested
/// @add_object: once an object is added to the scene, if it is not at the end
/// of the tree, then the tree should be re-sorted based on depth, sort indices,
/// resize object cull masks for all views
/// @remove_object: remove object and all its children
struct RenderServer
{
  gfx::DeviceImpl    device         = {};
  gfx::PipelineCache pipeline_cache = nullptr;
  PassGroup          pass_group     = {};
  SceneGroup         scene_group    = {};
  ViewGroup          view_group     = {};

  Option<PassImpl> get_pass(uid32 pass);
  Option<uid32>    get_pass_id(Span<char const> name);
  Option<uid32>    register_pass(PassImpl pass);
  Option<uid32>    add_scene(Span<char const> name);
  Option<Scene *>  get_scene(uid32 scene);
  void             remove_scene(uid32 scene);
  Option<uid32>    add_view(uid32 scene, Span<char const> name,
                            Camera const &camera);
  Option<View *>   get_view(uid32 view);
  void             remove_view(uid32 view);
  Option<uid32>    add_object(uid32 pass, uid32 pass_object_id, uid32 scene,
                              uid32 parent, SceneObjectDesc const &desc);
  void             remove_object(uid32 scene, uid32 object);
  Option<uid32>    add_directional_light(uid32                   scene,
                                         DirectionalLight const &light);
  Option<uid32>    add_point_light(uid32 scene, PointLight const &light);
  Option<uid32>    add_spot_light(uid32 scene, SpotLight const &light);
  Option<uid32>    add_area_light(uid32 scene, AreaLight const &light);
  Option<AmbientLight *>     get_ambient_light(uid32 scene);
  Option<DirectionalLight *> get_directional_light(uid32 scene, uid32 id);
  Option<PointLight *>       get_point_light(uid32 scene, uid32 id);
  Option<SpotLight *>        get_spot_light(uid32 scene, uid32 id);
  Option<AreaLight *>        get_area_light(uid32 scene, uid32 id);
  void                       remove_directional_light(uid32 scene, uid32 id);
  void                       remove_point_light(uid32 scene, uid32 id);
  void                       remove_spot_light(uid32 scene, uid32 id);
  void                       remove_area_light(uid32 scene, uid32 id);

  void                      transform();
  Result<Void, RenderError> frustum_cull();
  Result<Void, RenderError>
      encode_view(uid32 view, gfx::CommandEncoderImpl const &command_encoder);
  Result<Void, RenderError>
       render(gfx::CommandEncoderImpl const &command_encoder);
  void tick();
};

}        // namespace ash
