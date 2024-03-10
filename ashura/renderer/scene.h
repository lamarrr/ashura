#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/light.h"
#include "ashura/std/box.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/sparse_vec.h"
#include "ashura/std/types.h"

namespace ash
{
template <typename T>
struct Scene;
typedef struct SceneNode SceneNode;
template <typename T>
struct SceneObjects;
typedef struct SceneEnvironment SceneEnvironment;

/// linearly-tilted tree node
/// @depth: depth of the tree this node belongs to. there's ever only one root
/// node at depth 0
struct SceneNode
{
  uid32 parent       = UID32_INVALID;
  uid32 next_sibling = UID32_INVALID;
  uid32 first_child  = UID32_INVALID;
  u32   depth        = 0;
};

template <typename T>
struct SceneObjects
{
  Vec<SceneNode>  node             = {};
  Vec<Mat4Affine> local_transform  = {};
  Vec<Mat4Affine> global_transform = {};
  Vec<Box>        aabb             = {};
  Vec<i64>        z_index          = {};
  Vec<T>          objects          = {};
  SparseVec<u32>  id_map           = {};
};

struct SceneEnvironment
{
  SkyLight              sky_light                 = {};
  AmbientLight          ambient_light             = {};
  Vec<DirectionalLight> directional_lights        = {};
  SparseVec<u32>        directional_lights_id_map = {};
  Vec<PointLight>       point_lights              = {};
  SparseVec<u32>        point_lights_id_map       = {};
  Vec<SpotLight>        spot_lights               = {};
  SparseVec<u32>        spot_lights_id_map        = {};
  Vec<AreaLight>        area_lights               = {};
  SparseVec<u32>        area_lights_id_map        = {};
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

  template <typename... Args>
  Option<uid32> add_object(uid32 parent, Mat4Affine transform, Box aabb,
                           i64 z_index, bool is_transparent, Args &&...args);
  void          remove_object(uid32 object);
  Option<AmbientLight *>     get_ambient_light();
  Option<DirectionalLight *> get_directional_light(uid32 id);
  Option<PointLight *>       get_point_light(uid32 id);
  Option<SpotLight *>        get_spot_light(uid32 id);
  Option<AreaLight *>        get_area_light(uid32 id);
  Option<uid32> add_directional_light(DirectionalLight const &light);
  Option<uid32> add_point_light(PointLight const &light);
  Option<uid32> add_spot_light(SpotLight const &light);
  Option<uid32> add_area_light(AreaLight const &light);
  void          remove_directional_light(uid32 id);
  void          remove_point_light(uid32 id);
  void          remove_spot_light(uid32 id);
  void          remove_area_light(uid32 id);
};

void transform_nodes(SparseVec<u32> const &id_map, uid32 root_object,
                     Span<SceneNode const>  node,
                     Span<Mat4Affine const> local_transform,
                     Span<Mat4Affine>       global_transform);

}        // namespace ash