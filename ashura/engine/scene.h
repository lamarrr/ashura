#pragma once
#include "ashura/engine/light.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/sparse_vec.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{
/// linearly-tilted tree node
/// @param depth depth of the tree this node belongs to. there's ever only one
/// root node at depth 0
struct SceneNode
{
  uid parent       = UID_MAX;
  uid next_sibling = UID_MAX;
  uid first_child  = UID_MAX;
  u32 depth        = 0;
};

/// @param global_transform accumulation of transform from root parent down to
/// this object's transform
template <typename T>
struct SceneObjects
{
  Vec<SceneNode>  node             = {};
  Vec<Mat4Affine> local_transform  = {};
  Vec<Mat4Affine> global_transform = {};
  Vec<Box>        aabb             = {};
  Vec<i64>        z_index          = {};
  Vec<T>          objects          = {};
  SparseVec       id_map           = {};
};

struct SceneEnvironment
{
  Vec<PunctualLight> lights        = {};
  SparseVec          lights_id_map = {};
};

/// @remove_scene: remove all pass resources associated with a scene object.
///
/// @add_object: once an object is added to the scene, if it is not at the end
/// of the tree, then the tree should be re-sorted based on depth, sort indices,
/// resize object cull masks for all views
/// @remove_object: remove object and all its children
template <typename T>
struct Scene
{
  Span<char const> name        = {};
  SceneEnvironment environment = {};
  SceneObjects<T>  objects     = {};

  // after add, sort last and present ids by depth? or use hierarchical sort
  // before frame begin or when it is dirty?
  template <typename... Args>
  Option<uid>             add_object(uid parent, Mat4Affine transform, Box aabb,
                                     i64 z_index, bool is_transparent, Args &&...args);
  void                    remove_object(uid object);
  Option<PunctualLight *> get_light(uid id);
  Option<uid>             add_spot_light(PunctualLight const &light);
  void                    remove_light(uid id);
};

// TODO(lamarrr): hierarchy should be maintained upon insertion and removal
void           hierarchical_sort(Span<SceneNode const> node, Span<u32> indices);
constexpr void transform_nodes(SparseVec const       &id_map,
                               Span<SceneNode const>  node,
                               Span<Mat4Affine const> local_transform,
                               Span<Mat4Affine>       global_transform)
{
  // TODO(lamarrr): this would require that the hierarchy is maintained, i.e.
  // sorted by depth after each insert? and sorted by that
  for (u32 i = 0; i < id_map.size(); i++)
  {
    global_transform[i] =
        global_transform[id_map[node[i].parent]] * local_transform[i];
  }
}

}        // namespace ash
