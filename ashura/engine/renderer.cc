#include "ashura/engine/renderer.h"
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

// transform views from object-space to world space then to clip space using
// view's camera
void RenderServer::transform_()
{
  for (u32 iscene = 0; iscene < scene_group.num_scenes(); iscene++)
  {
    Scene    &scene       = scene_group.scenes[iscene];
    u64 const num_objects = scene.num_objects();
    for (u64 i = 0; i < num_objects; i++)
    {
      scene.objects.global_transform[i] =
          scene.objects.global_transform[scene.objects.id_map.unsafe_to_index(
              scene.objects.node[i].parent)] *
          scene.objects.local_transform[i];
    }
  }
}

Result<Void, RenderError> RenderServer::frustum_cull_()
{
  for (u32 iview = 0; iview < view_group.num_views(); iview++)
  {
    View  &view = view_group.views[iview];
    Scene &scene =
        scene_group.scenes[scene_group.id_map.unsafe_to_index(view.scene)];
    for (u64 i = 0; i < scene.num_objects(); i++)
    {
      bitvec::set(
          view.object_cull_mask, i,
          is_outside_frustum(
              static_cast<Mat4>(view.camera.projection * view.camera.view *
                                scene.objects.global_transform[i]),
              scene.objects.aabb[i]));
    }
  }

  return Ok<Void>{};
}

// sort objects by z-index, get min and max z-index
// for all objects in the z-index range, invoke the passes
// pass->encode(z_index, begin_objects, num_objects)
//
// also calls PassObjectCmp to sort all objects belonging to a pass invocation
// sort by z-index
// sort by transparency, transparent objects last
// sort by pass sort key
// sort by depth?
Result<Void, RenderError> RenderServer::sort_()
{
  for (u32 iscene = 0; iscene < scene_group.num_scenes(); iscene++)
  {
    Scene    &scene       = scene_group.scenes[iscene];
    u64 const num_objects = scene.num_objects();
    alg::indirect_sort(scene.objects.z_index,
                       Span{scene.objects.sort_index, num_objects});
    // what is it called when you iterate to a partition
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
}

}        // namespace ash
