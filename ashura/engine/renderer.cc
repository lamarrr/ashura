#include "ashura/engine/renderer.h"
#include "ashura/std/math.h"

namespace ash
{

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
    usize const required_size = bitvec::size_u64(scene.num_objects());
    if (!tvec::reserve(allocator, view.object_cull_mask,
                       view.object_cull_mask_capacity, required_size))
    {
      return Err{RenderError::OutOfMemory};
    }
  }

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

}        // namespace ash
