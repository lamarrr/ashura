#include "ashura/engine/renderer.h"
#include "ashura/std/algorithms.h"
#include "ashura/std/bit_span.h"
#include "ashura/std/math.h"

namespace ash
{

// for (u32 iview = 0; iview < view_group.num_views(); iview++)
//   {
//     View  &view = view_group.views[iview];
//     Scene &scene =
//         scene_group.scenes[scene_group.id_map.unsafe_to_index(view.scene)];
//     usize const required_size = bitvec::size_u64(scene.num_objects());
//     if (!tvec::reserve(allocator, view.object_cull_mask,
//                        view.object_cull_mask_capacity, required_size))
//     {
//       return Err{RenderError::OutOfMemory};
//     }
//   }

// transform views from object-space to root-object space
void RenderServer::transform_()
{
  for (u32 iscene = 0; iscene < scene_group.num_scenes(); iscene++)
  {
    Scene &scene = scene_group.scenes[iscene];
    for (u32 i = 0; i < scene.num_objects(); i++)
    {
      scene.objects.global_transform[i] =
          scene.objects.global_transform
              [scene.objects.id_map[scene.objects.node[i].parent]] *
          scene.objects.local_transform[i];
    }
  }
}

/// https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron/blob/b92d559bd083f44df9f8f42a6ad149c1584ae94c/src/common/Misc/Misc.cpp#L265
/// https://bruop.github.io/frustum_culling/
///
/// exploits the fact that in clip-space all vertices in the view frustum will
/// obey:
///
/// -w <= x << w
/// -w <= y << w
///  0 <= z << w
///
constexpr bool is_outside_frustum(Mat4 const &mvp, Box const &box)
{
  constexpr u8   NUM_CORNERS = 8;
  constexpr auto to_vec4     = [](Vec3 a) { return Vec4{a.x, a.y, a.z, 1}; };
  Vec4 const     corners[NUM_CORNERS] = {
      mvp * to_vec4(box.offset),
      mvp * to_vec4(box.offset + Vec3{box.extent.x, 0, 0}),
      mvp * to_vec4(box.offset + Vec3{box.extent.x, box.extent.y, 0}),
      mvp * to_vec4(box.offset + Vec3{0, box.extent.y, 0}),
      mvp * to_vec4(box.offset + Vec3{0, 0, box.extent.z}),
      mvp * to_vec4(box.offset + Vec3{box.extent.x, 0, box.extent.z}),
      mvp * to_vec4(box.offset + box.extent),
      mvp * to_vec4(box.offset + Vec3{0, box.extent.y, box.extent.z})};
  u8 left   = 0;
  u8 right  = 0;
  u8 top    = 0;
  u8 bottom = 0;
  u8 back   = 0;

  for (u8 i = 0; i < NUM_CORNERS; i++)
  {
    Vec4 const corner = corners[i];

    if (corner.x < -corner.w)
    {
      left++;
    }

    if (corner.x > corner.w)
    {
      right++;
    }

    if (corner.y < -corner.w)
    {
      bottom++;
    }

    if (corner.y > corner.w)
    {
      top++;
    }

    if (corner.z < 0)
    {
      back++;
    }
  }

  return left == NUM_CORNERS || right == NUM_CORNERS || top == NUM_CORNERS ||
         bottom == NUM_CORNERS || back == NUM_CORNERS;
}

// transform objects from root object space to clip space using view's camera
Result<Void, RenderError> RenderServer::frustum_cull_()
{
  for (u32 iview = 0; iview < view_group.num_views(); iview++)
  {
    View     &view        = view_group.views[iview];
    Scene    &scene       = scene_group.scenes[scene_group.id_map[view.scene]];
    u32 const num_objects = scene.num_objects();
    BitSpan   is_object_visible{view.is_object_visible, num_objects};
    for (u32 i = 0; i < num_objects; i++)
    {
      is_object_visible[i] =
          !is_outside_frustum(view.camera.projection * view.camera.view *
                                  scene.objects.global_transform[i],
                              scene.objects.aabb[i]);
    }
  }

  return Ok<Void>{};
}

// sort by z-index
// sort by transparency, transparent objects last
// sort by pass sorter
Result<Void, RenderError> RenderServer::sort_()
{
  for (u32 iscene = 0; iscene < scene_group.num_scenes(); iscene++)
  {
    // TODO(lamarrr): don't perform any sorting on frustum-culled objects
      // select();
    Scene    &scene       = scene_group.scenes[iscene];
    u32 const num_objects = scene.num_objects();
    indirect_sort(scene.objects.z_index, Span{scene.sort_indices, num_objects});
    for_each_partition_indirect(
        scene.objects.z_index, Span{scene.sort_indices, num_objects},
        [&](Span<u32> partition_indices) {
          indirect_sort(BitSpan{scene.objects.is_transparent, num_objects},
                        partition_indices);
          for_each_partition_indirect(
              BitSpan{scene.objects.is_transparent, num_objects},
              partition_indices, [&](Span<u32> partition_indices) {
                indirect_sort(scene.objects.node, partition_indices,
                              [](SceneNode const &a, SceneNode const &b) {
                                return a.pass < b.pass;
                              });
                for_each_partition_indirect(
                    scene.objects.node, partition_indices,
                    [&](Span<u32> partition_indices) {
                      PassImpl const &pass =
                          pass_group.passes
                              [pass_group.id_map[scene.objects
                                                     .node[partition_indices[0]]
                                                     .pass]];
                      pass.interface->sort(
                          pass.self, this,
                          scene_group.id_map.index_to_id[iscene],
                          partition_indices);
                    },
                    [](SceneNode const &a, SceneNode const &b) {
                      return a.pass == b.pass;
                    });
              });
        });
  }

  return Ok<Void>{};
}

/// transform->frustum_cull->sort->render
///
// Invocation Procedure
//
// - sort scene objects by z-index
// - for objects in the same z-index, sort by transparency (transparent
// objects drawn last)
// - sort transparent objects by AABB from camera frustum, this will help with
// layering/blending one object atop of the other
// - for objects in the same z-index, sort by passes so objects in the same
// pass can be rendered together.
// - sort objects in the same pass by key from render pass (materials and
// textures and resources) to minimize pipeline state changes
// - for the z-index group sorted objects with the same passes, sort using the
// PassCmp key
// - for each partition, invoke the pass with the objects
//
//
//
//
// we need the mesh and object render-data is mostly pre-configured or
// modified outside the renderer we just need to implement the post-effects
// and render-orders and add other passes on top of the objects
//
// each scene is rendered and composited onto one another? can this possibly
// work for portals?
Result<Void, RenderError> RenderServer::render_()
{
  return Ok<Void>{};
}

}        // namespace ash
