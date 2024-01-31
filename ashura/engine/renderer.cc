#include "ashura/engine/renderer.h"
#include "ashura/std/math.h"

namespace ash
{

void Renderer::transform(RenderServer *server)
{
  for (u32 iscene = 0; iscene < server->scene_group->num_scenes(); iscene++)
  {
    Scene    &scene       = server->scene_group->scenes[iscene];
    u64 const num_objects = scene.num_objects();
    for (u64 i = 0; i < num_objects; i++)
    {
      scene.object_global_transforms[i] =
          scene.object_global_transforms[scene.objects_id_map.unsafe_to_index(
              scene.object_nodes[i].parent)] *
          scene.object_local_transforms[i];
    }
  }
}

void Renderer::frustum_cull(RenderServer *server)
{
  for (u32 iview = 0; iview < server->view_group->num_views(); iview++)
  {
    View &view = server->view_group->views[iview];
    // view.object_cull_mask; reallocate cull mask bitvec::size_u64(num_objects)
    //
    Scene *scene = server->get_scene(view.scene);
    for (u64 i = 0; i < scene->num_objects(); i++)
    {
      bitvec::set(
          view.object_cull_mask, i,
          !is_outside_frustum(
              static_cast<Mat4>(view.camera.projection * view.camera.view *
                                scene->object_global_transforms[i]),
              scene->object_aabb[i]));
    }
  }
}

}        // namespace ash
