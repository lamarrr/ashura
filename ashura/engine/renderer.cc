#include "ashura/engine/renderer.h"

namespace ash
{

void Renderer::transform(RenderServer *server)
{
  for (u32 i = 0; i < server->scene_group->id_map.num_valid(); i++)
  {
    Scene    &scene       = server->scene_group->scenes[i];
    u64 const num_objects = scene.objects_id_map.num_valid();
    // transform all objects from local to
  }
}

}        // namespace ash
