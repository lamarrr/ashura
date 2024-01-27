#pragma once

#include "ashura/renderer.h"

namespace ash
{
namespace gfx
{

typedef struct PBRMaterial PBRMaterial;
typedef struct PBRVertex   PBRVertex;
typedef struct PBRMesh     PBRMesh;
typedef struct PBRObject   PBRObject;
typedef struct PBRPass     PBRPass;

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
  u32       vertex_buffer = 0;
  u32       index_buffer  = 0;
  u32       first_index   = 0;
  u32       num_indices   = 0;
  IndexType index_type    = IndexType::Uint16;
};

struct PBRObject
{
  PBRMaterial material   = {};
  PBRMesh     mesh       = {};
  u64         scene_node = 0;
};

struct PBRPass
{
  PBRObject          *objects               = nullptr;
  u64                 num_objects           = 0;
  DescriptorSetLayout descriptor_set_layout = nullptr;
  DescriptorHeapImpl  descriptor_heap       = {};
  PipelineCache       pipeline_cache        = nullptr;
  GraphicsPipeline    pipeline              = nullptr;
  Sampler             sampler               = nullptr;

  u64 add_object(Scene *scene, PBRMesh const &mesh, PBRMaterial const &material,
                 Box aabb)
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
                     CommandEncoderImpl command_encoder, i64 z_index,
                     bool is_transparent, u64 first_scene_object,
                     u64 num_scene_objects)
  {
  }

  static PassInterface const interface{
      .init = init, .deinit = deinit, .update = update, .encode = encode};
};

}        // namespace gfx
}        // namespace ash
