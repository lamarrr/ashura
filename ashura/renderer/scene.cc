#include "ashura/renderer/scene.h"

namespace ash
{

// transform views from object-space to root-object space
void ::ash::transform_nodes(SparseVec<u32> const &id_map, uid32 root_object,
                            Span<SceneNode const>  node,
                            Span<Mat4Affine const> local_transform,
                            Span<Mat4Affine>       global_transform)
{
  // a dirty list might be better anyway
  // TODO(lamarrr): this would require that the hierarchy is maintained, i.e.
  // sorted by depth after each insert? and sorted by that
  for (u32 i = 0; i < id_map.size(); i++)
  {
    global_transform[i] =
        global_transform[id_map[node[i].parent]] * local_transform[i];
  }
}
}        // namespace ash
