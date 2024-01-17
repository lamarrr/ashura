#pragma once

#include "ashura/renderer.h"

namespace ash
{
namespace gfx
{

typedef struct RRect         RRect;
typedef struct RRectMaterial RRectMaterial;
typedef struct RRectObject   RRectObject;
typedef struct RRectPass     RRectPass;

struct RRect
{
  Vec3 center           = {};
  Vec3 half_extent      = {};
  f32  border_thickness = 0;
  Vec4 border_radii     = {};
};

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
  RRectObject        *objects               = nullptr;
  u64                 num_objects           = 0;
  DescriptorSetLayout descriptor_set_layout = nullptr;
  DescriptorHeapImpl  descriptor_heap       = {};
  PipelineCache       pipeline_cache        = nullptr;
  GraphicsPipeline    pipeline              = nullptr;
  Sampler             sampler               = nullptr;

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
  // store attachments for each scene in the scene group. prepare to render for
  // each.
  //
  //
  // TODO(lamarrr): clipping
  //
  u64  add_object(Scene *scene, RRect const &rrect,
                  RRectMaterial const &material, i64 z_index);
  void remove_object(Scene *scene, u64 object);
};

}        // namespace gfx
}        // namespace ash
