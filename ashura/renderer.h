#pragma once
#include "ashura/gfx.h"
#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/types.h"

namespace ash
{

constexpr u32 MAXIMUM_SPOT_LIGHTS        = 64;
constexpr u32 MAXIMUM_POINT_LIGHTS       = 64;
constexpr u32 MAXIMUM_DIRECTIONAL_LIGHTS = 64;
constexpr u32 MAXIMUM_ID_SIZE            = 128;

typedef char                           Id[MAXIMUM_ID_SIZE];
typedef Vec4                           AmbientLight;
typedef struct DirectionalLight        DirectionalLight;
typedef struct PointLight              PointLight;
typedef struct SpotLight               SpotLight;
typedef struct OrthographicCamera      OrthographicCamera;
typedef struct PerspectiveCamera       PerspectiveCamera;
typedef struct Scene                   Scene;
typedef struct Texture                 Texture;
typedef struct ResourceManager         ResourceManager;
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
typedef i32 (*PassObjectCmp)(void *, Scene *, u64 a, u64 b);

struct Texture
{
  gfx::ImageView view = nullptr;
  Vec2           uv0  = {};
  Vec2           uv1  = {};
};

struct RenderTarget
{
  u64         generation = 0;
  gfx::Extent extent     = {};
  gfx::Format format     = gfx::Format::Undefined;
  gfx::Image  image      = nullptr;
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
  RenderTarget root_render_target = {};

  void init();

  /// TODO(lamarrr): whatttt??
  ///
  u64  add_scene(Scene *);
  void get_scene_resource(u64, Scene *);
  /// TODO(lamarrr): destroy all resources added by the pass for the scene
  void remove_scene(u64);
};

/// @init: add self and resources to renderer
/// @deinit: remove self and resources from renderer
/// @update: update internal data based on changed information in the scene
/// @encode: encode commands to be sent to the gpu
struct PassInterface
{
  void (*init)(Pass self, ResourceManager *mgr)   = nullptr;
  void (*deinit)(Pass self, ResourceManager *mgr) = nullptr;
  void (*update)(Pass self, ResourceManager *mgr) = nullptr;
  void (*encode)(Pass self, ResourceManager *mgr, Scene *scene,
                 gfx::CommandEncoderImpl command_encoder, i64 z_index,
                 bool is_transparent, u64 first_scene_object,
                 u64 num_scene_objects)           = nullptr;
};

// can be loaded from a DLL i.e. C++ with C-linkage => DLL
struct PassImpl
{
  Pass                 self      = nullptr;
  PassInterface const *interface = nullptr;
};

// world->[capture->world]->post-fx->hud->[capture->hud]
// use z-indexes with full screen quad?
// how to project from object-space to full-screen space
//
// i.e. world scene pass -> post-fx pass -> HUD pass

/// linearly-tilted tree node
/// @pass: pass to be used to render this object
struct SceneObject
{
  u64 parent       = 0;
  u64 level        = 0;
  u64 next_sibling = 0;
  u64 first_child  = 0;
  u64 pass         = 0;
  u64 pass_object  = 0;
};

// A scene repared for rendering
//
// camera should be assumed to change every frame
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
// - for the z-index group sorted objects with the same passes
// - invoke the pass with the objects
//
//
// objects_z_ordered will contain indices to the objects and will be sorted by a
// z_index key
//
// Area lights: https://learnopengl.com/Guest-Articles/2022/Area-Lights
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
  SceneObject      *object_nodes                = nullptr;
  Mat4Affine       *object_local_transforms     = nullptr;
  Mat4Affine       *object_global_transforms    = nullptr;
  u64              *camera_space_mask           = nullptr;
  Box              *object_aabb                 = nullptr;
  i64              *object_z_index              = nullptr;
  u64              *object_transform_dirty_mask = nullptr;
  u64              *object_cull_mask            = nullptr;
  u64              *objects_z_ordered           = nullptr;
  u64              *object_transparency_mask    = nullptr;
  u64              *object_ids_map              = nullptr;
  u64               num_objects                 = 0;

  u64  add_object(Mat4Affine const &local_transform, Box const &aabb,
                  i64 z_index, bool is_transparent);
  void remove_object(u64);        // remove and shift objects only
  // add_light();
  //
  // perform frustum and occlusion culling of objects and light (in the same
  // z-index) then cull by z-index???? Z-index not needed in culling
  // occlussion culling only happens when a fully-opaque object occludes another
  // object.
  //
  // cull lights by camera frustum
  void cull();

  // calls PassObjectCmp to sort all objects belonging to a pass invocation
  void sort();
};

// sort objects by z-index, get min and max z-index
// for all objects in the z-index range, invoke the passes pass->encode(z_index,
// begin_objects, num_objects)
//
// Unit is -1 to +1 for x,y,z
// will be scaled to the screen dimensions
//
//
struct Renderer
{
  PassImpl      *passes          = nullptr;
  PassObjectCmp *pass_object_cmp = nullptr;
  u64            num_passes      = 0;
  Id            *passes_id       = nullptr;

  u64      add_pass(Span<char const> pass_id, PassImpl pass);
  PassImpl get_pass(u64);
  PassImpl get_pass_by_id(Span<char const> pass_id);

  // we need the mesh and object render-data is mostly pre-configured or
  // modified outside the renderer we just need to implement the post-effects
  // and render-orders and add other passes on top of the objects
  //
  // each scene is rendered and composited onto one another? can this possibly
  // work for portals?
  void render(ResourceManager *mgr, Scene *scene);

  // remove all resources associated with a scene object.
  void remove_scene(ResourceManager *mgr, Scene *scene);
};

/// SEE: https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos
/// SEE:
/// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/textures.glsl
struct PBRMaterial
{
  Texture base_color_texture = {};
  Texture metallic_texture   = {};
  Texture roughness_texture  = {};
  Texture normal_texture     = {};
  Texture occlusion_texture  = {};
  Texture emissive_texture   = {};
  Vec4    base_color_factor  = {1, 1, 1, 1};
  f32     metallic_factor    = 1;
  f32     roughness_factor   = 1;
  f32     normal_scale       = 1;
  f32     occlusion_strength = 1;
  Vec3    emissive_factor    = {1, 1, 1};
  f32     emissive_strength  = 1;
  bool    unlit              = false;
};

struct PBRVertex
{
  f32 x = 0, y = 0, z = 0;
  f32 u = 0, v = 0;
};

struct PBRMesh
{
  u32            vertex_buffer = 0;
  u32            index_buffer  = 0;
  u32            first_index   = 0;
  u32            num_indices   = 0;
  gfx::IndexType index_type    = gfx::IndexType::Uint16;
};

struct PBRObject
{
  PBRMaterial material   = {};
  PBRMesh     mesh       = {};
  u64         scene_node = 0;
};

struct PBRPass
{
  PBRObject               *objects               = nullptr;
  u64                      num_objects           = 0;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;
  gfx::DescriptorHeapImpl  descriptor_heap       = {};
  gfx::PipelineCache       pipeline_cache        = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::Sampler             sampler               = nullptr;

  u64 add_object(Scene *scene, PBRMesh const &mesh, PBRMaterial const &material)
  {
  }

  void remove_object(Scene *scene, u64 object)
  {
  }

  static void init(Pass self_, ResourceManager *mgr)
  {
    // create resources
    PBRPass *self = (PBRPass *) self_;
  }

  static void deinit(Pass self_, ResourceManager *mgr)
  {
  }

  static void update(Pass self_, ResourceManager *mgr)
  {
    // re-build renderpass and framebuffer if needed
  }

  static void encode(Pass self_, ResourceManager *mgr, Scene *scene,
                     gfx::CommandEncoderImpl command_encoder, i64 z_index,
                     bool is_transparent, u64 first_scene_object,
                     u64 num_scene_objects)
  {
  }

  static PassInterface const interface{
      .init = init, .deinit = deinit, .update = update, .encode = encode};
};

struct RRect
{
  Vec3 center           = {};
  Vec3 half_extent      = {};
  f32  border_thickness = 0;
  Vec4 border_radii     = {};
};

/// @base_color_factors: [TL, TR, BR, BL]
/// @border_colors: [TL, TR, BR, BL]
struct RRectMaterial
{
  Texture base_color_texture    = {};
  Vec4    base_color_factors[4] = {};
  Vec4    border_colors[4]      = {};
};

struct RRectObject
{
  RRect         rrect      = {};
  RRectMaterial material   = {};
  u64           scene_node = 0;
};

// quad + instanced + transformed + antialiased
//
// Quad is an object without a type or specification. a unit square with just a
// transformation matrix.
//
// Quads don't need vertex/index buffers. only shaders + gl_Index into
// shader-stored vertices
//
// offscreen passes - run offscreen passes in the update function? what about
// pass data and context? bool is_offscreen?
//
// how will offscreen rendering work? separate scene? - it must not be a
// separate scene, it is left to the pass to decide allocate own texture for
// rendering, then call passes of the objects to be rendered onto the allocated
// framebuffer void render(...); the objects will be added during render?
// will also need separate coordinates? or transformed?
// HOW TO REUSE PASSES IN OFFSCREEN PASSES
//
//
// - temp-offscreen rendering: request scratch image with maximum
// size of the viewport size and specific format, not released until execution
// completes, the pass doesn't need to release it so other passes can re-use it
// if needed.
//
//
// - offscreen pass will store the objects + their actual rendering pass and
// invoke the actual pass when rendering is needed. these are sorted by sub-pass
// again. we then invoke the actual passes with the frame buffer and location we
// need to render to? WROOONNGGGGG - the subpass will also need to store info
// and track data of the objects
//   recursive offscreen?
//
//
// GUI blur for example needs to capture the whole scene at one point and then
// render to screen (Layer)
//
// Store last capture z-index + area, if non-intersecting and re-usable re-use
//
struct RRectPass
{
  RRectObject             *objects               = nullptr;
  u64                      num_objects           = 0;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;
  gfx::DescriptorHeapImpl  descriptor_heap       = {};
  gfx::PipelineCache       pipeline_cache        = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::Sampler             sampler               = nullptr;

  // todo(lamarrr): multiple framebuffers? should it be
  // stored here? since we are allocating scratch images, we would need to
  // recreate the framebuffers every frame [scene, pass] association cos we need
  // to be able to dispatch for several types of scenes (offscreen and
  // onscreen?)
  //
  // resource_mgr->create_frame_buffer()
  // resource_mgr->allocate_scratch_frame_buffer()
  // resource_mgr->release_scratch_frame_buffer()
  // resource_mgr->destroy_frame_buffer/_image()
  //
  // i.e. blur on offscreen layer
  //
  u64  add_object(Scene *scene, RRect const &rrect,
                  RRectMaterial const &material, i64 z_index);
  void remove_object(Scene *scene, u64 object);
};

struct ChromaticAberrationPass
{
};

struct FXAAPass
{
};

// object-clip space blur
//
// - capture scene at object's screen-space area, dilate by the blur extent
// - reserve scratch stencil image with at least size of the dilated area
// - blur captured area
// - render object to offscreen scratch image stencil only
// - using rendered stencil, directly-write (without blending) onto scene again
struct BlurPass
{
};

struct BloomPass
{
};

};        // namespace ash
