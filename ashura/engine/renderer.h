#pragma once
#include "ashura/engine/error.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/box.h"
#include "ashura/std/dict.h"
#include "ashura/std/image.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/sparse_vec.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct Texture          Texture;
typedef Vec4                    AmbientLight;
typedef struct DirectionalLight DirectionalLight;
typedef struct PointLight       PointLight;
typedef struct SpotLight        SpotLight;
typedef struct AreaLight        AreaLight;
typedef struct Orthographic     Orthographic;
typedef struct Perspective      Perspective;
typedef struct Camera           Camera;
typedef struct PassBinding_T   *PassBinding;
typedef struct Pass_T          *Pass;
typedef struct PassBeginInfo    PassBeginInfo;
typedef struct PassEncodeInfo   PassEncodeInfo;
typedef struct PassEndInfo      PassEndInfo;
typedef struct PassInterface    PassInterface;
typedef struct PassImpl         PassImpl;
typedef struct PassGroup        PassGroup;
typedef struct SceneNode        SceneNode;
typedef struct SceneObjectDesc  SceneObjectDesc;
typedef struct Scene            Scene;
typedef struct SceneGroup       SceneGroup;
typedef struct Attachment       Attachment;
typedef struct View             View;
typedef struct ViewGroup        ViewGroup;
typedef struct RenderServer     RenderServer;

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

struct MSAAConfig
{
  gfx::SampleCount sample_count = gfx::SampleCount::None;
};

struct FXAAConfig
{
};

enum class AATechnique : u8
{
  None = 0,
  MSAA = 1,
  FXAA = 2
};

struct AAConfig
{
  union
  {
    MSAAConfig msaa;
    FXAAConfig fxaa;
    char       none_ = 0;
  };
  AATechnique technique = AATechnique::None;
};

/// E' = Blur(E)
/// D' = Blur(D) + E'
/// C' = Blur(C) + D'
/// B' = Blur(B) + C'
/// A' = Blur(A) + B'
// downsample to mip chains of 5 total
// perform gaussian blur of the image
// addittive composite back unto the first mip
struct BloomConfig
{
  u32  blur_radius          = 4;
  f32  strength             = 1;
  f32  radius               = 1;
  Vec3 default_color        = {};
  f32  default_opacity      = 0.7f;
  f32  luminosity_threshold = 0.75f;
  f32  smooth_width         = 0.01f;
};

struct ViewConfig
{
  gfx::Extent extent               = {};
  gfx::Format color_format         = gfx::Format::Undefined;
  gfx::Format depth_stencil_format = gfx::Format::Undefined;
  AAConfig    aa                   = {};
  BloomConfig bloom                = {};
  f32         chromatic_aberration = 0;
};

struct PassInterface
{
  void (*init)(Pass self, RenderServer *server, uid32 id) = nullptr;
  void (*deinit)(Pass self, RenderServer *server)         = nullptr;
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

  void                       transform();
  Option<AmbientLight *>     get_ambient_light();
  Option<DirectionalLight *> get_directional_light(uid32 id);
  Option<PointLight *>       get_point_light(uid32 id);
  Option<SpotLight *>        get_spot_light(uid32 id);
  Option<AreaLight *>        get_area_light(uid32 id);
  Option<uid32> add_directional_light(DirectionalLight const &light);
  Option<uid32> add_point_light(PointLight const &light);
  Option<uid32> add_spot_light(SpotLight const &light);
  Option<uid32> add_area_light(AreaLight const &light);
  void          remove_directional_light(uid32 id);
  void          remove_point_light(uid32 id);
  void          remove_spot_light(uid32 id);
  void          remove_area_light(uid32 id);
};

struct SceneGroup
{
  Vec<Scene>     scenes = {};
  SparseVec<u32> id_map = {};
};

struct View
{
  Span<char const> name              = {};
  Camera           camera            = {};
  uid32            scene             = 0;
  BitVec<u64>      is_object_visible = {};
  Vec<u32>         sort_indices      = {};
  ViewConfig       config            = {};

  Result<Void, Error> frustum_cull();
};

struct ViewGroup
{
  Vec<View>      views     = {};
  SparseVec<u32> id_map    = {};
  uid32          root_view = UID32_INVALID;
};

/// @remove_scene: remove all pass resources associated with a scene object.
/// @acquire_screen_color_image:
///
/// @add_object: once an object is added to the scene, if it is not at the end
/// of the tree, then the tree should be re-sorted based on depth, sort indices,
/// resize object cull masks for all views
/// @remove_object: remove object and all its children
struct RenderServer
{
  AllocatorImpl      allocator      = default_allocator;
  gfx::DeviceImpl    device         = {};
  gfx::PipelineCache pipeline_cache = nullptr;
  gfx::FrameContext  frame_context  = nullptr;
  gfx::Swapchain     swapchain      = nullptr;

  // Option<Scene *> get_scene(uid32 scene);
  // void            remove_scene(uid32 scene);
  // Option<uid32>   add_view(uid32 scene, Span<char const> name,
  //                          Camera const &camera);
  // Option<View *>  get_view(uid32 view);
  // void            remove_view(uid32 view);
  // Option<uid32>   add_object(uid32 pass, uid32 pass_object_id, uid32 scene,
  //                            uid32 parent, SceneObjectDesc const &desc);
  // void            remove_object(uid32 scene, uid32 object);

  void tick();
};

namespace rdg
{

struct Attachment
{
  gfx::Image     image = nullptr;
  gfx::ImageView view  = nullptr;
  gfx::ImageDesc desc  = {};
};

enum class PassFlags : u8
{
  None     = 0x00,
  Render   = 0x01,
  Compute  = 0x02,
  Transfer = 0x04,
  Mesh     = 0x08
};

ASH_DEFINE_ENUM_BIT_OPS(PassFlags)

struct RenderGraph
{
  // images resized when swapchain extents changes
  Option<Attachment> request_scratch_attachment(gfx::ImageDesc const &desc);
  void               release_scratch_attachment(Attachment const &attachment);
  Option<gfx::RenderPass> get_render_pass(gfx::RenderPassDesc const &desc);
  Option<gfx::Shader>     get_shader(Span<char const> name);
  void                    queue_delete(u64 last_use_tick);

  template <typename Reg, typename Exe>
  void add_pass(Span<char const> name, PassFlags flags, Reg &&registration,
                Exe &&execution);
};

}        // namespace rdg

}        // namespace ash
