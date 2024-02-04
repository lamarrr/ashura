#include "ashura/engine/renderer.h"
#include "ashura/std/range.h"
#include "ashura/std/bit_span.h"
#include "ashura/std/math.h"

namespace ash
{

void destroy_pass_group(PassGroup &group, AllocatorImpl const &allocator)
{
  allocator.deallocate_typed(group.passes, group.passes_capacity);
  group.id_map.reset(allocator);
}

void destroy_scene(Scene &scene, AllocatorImpl const &allocator)
{
  allocator.deallocate_typed(scene.directional_lights,
                             scene.directional_lights_capacity);
  scene.directional_lights_id_map.reset(allocator);
  allocator.deallocate_typed(scene.point_lights, scene.point_lights_capacity);
  scene.point_lights_id_map.reset(allocator);
  allocator.deallocate_typed(scene.spot_lights, scene.spot_lights_capacity);
  scene.spot_lights_id_map.reset(allocator);
  allocator.deallocate_typed(scene.area_lights, scene.area_lights_capacity);
  scene.area_lights_id_map.reset(allocator);
  allocator.deallocate_typed(scene.sort_indices, scene.sort_indices_capacity);
}

void destroy_scene_group(SceneGroup &group, AllocatorImpl const &allocator)
{
  for (u32 i = 0; i < group.id_map.num_valid(); i++)
  {
    destroy_scene(group.scenes[i], allocator);
  }
  group.id_map.reset(allocator);
}

void destroy_view(View &view, AllocatorImpl const &allocator)
{
  allocator.deallocate_typed(view.is_object_visible,
                             view.is_object_visible_capacity);
}

Option<PassImpl const *> RenderServer::get_pass(uid32 pass)
{
  u32 index;
  if (!pass_group.id_map.try_to_index(pass, index))
  {
    return None;
  }

  return Some{static_cast<PassImpl const *>(&pass_group.passes[index])};
}

Option<uid32> RenderServer::get_pass_id(char const *name)
{
  for (u32 i = 0; i < pass_group.id_map.num_valid(); i++)
  {
    if (strcmp(pass_group.passes[i].name, name) == 0)
    {
      return Some{pass_group.id_map.index_to_id[i]};
    }
  }
  return None;
}

Option<uid32> RenderServer::register_pass(PassImpl pass)
{
  if (pass_group.id_map.num_free == 0)
  {
    if (!tvec::reserve(allocator, pass_group.passes, pass_group.passes_capacity,
                       pass_group.id_map.num_slots + 1) ||
        !pass_group.id_map.reserve_new_ids(allocator, 1))
    {
      return None;
    }
  }

  uid32 id;
  u32   index;
  if (!pass_group.id_map.allocate_id(id, index))
  {
    return None;
  }

  pass_group.passes[index] = pass;
  return Some{id};
}

Option<uid32> RenderServer::create_scene(char const *name)
{
  if (scene_group.id_map.num_free == 0)
  {
    if (!tvec::reserve(allocator, scene_group.scenes,
                       scene_group.scenes_capacity,
                       scene_group.id_map.num_slots + 1) ||
        !scene_group.id_map.reserve_new_ids(allocator, 1))
    {
      return None;
    }
  }

  uid32 id;
  u32   index;
  if (!scene_group.id_map.allocate_id(id, index))
  {
    return None;
  }

  scene_group.scenes[index] = Scene{.name = name};
  return Some{id};
}

Option<Scene *> RenderServer::get_scene(uid32 scene)
{
  u32 index;
  if (!scene_group.id_map.try_to_index(scene, index))
  {
    return None;
  }
  return Some{scene_group.scenes + index};
}

void RenderServer::remove_scene(uid32 scene)
{
  u32 index;
  if (!scene_group.id_map.try_to_index(scene, index))
  {
    return;
  }
  // TODO(lamarrr): notify all views

  destroy_scene(scene_group.scenes[index], allocator);
  scene_group.id_map.release(scene, [](auto...) {
    // TODO(lamarrr): trivial relocate op
  });
}

Option<uid32> RenderServer::add_view(uid32 scene, char const *name,
                                     Camera const &camera)
{
  u32 scene_index;
  if (!scene_group.id_map.try_to_index(scene, scene_index))
  {
    return None;
  }

  if (scene_group.id_map.num_free == 0)
  {
    if (!tvec::reserve(allocator, scene_group.scenes,
                       scene_group.scenes_capacity,
                       scene_group.id_map.num_slots + 1) ||
        !scene_group.id_map.reserve_new_ids(allocator, 1))
    {
      return None;
    }
  }

  uid32 id;
  u32   index;
  if (!view_group.id_map.allocate_id(id, index))
  {
    return None;
  }

  // TODO(lamarrr): resize is_object_visible?
  view_group.views[index] = View{.name                       = name,
                                 .camera                     = camera,
                                 .scene                      = scene,
                                 .is_object_visible          = nullptr,
                                 .is_object_visible_capacity = 0};

  // TODO(lamarrr): notify all views
  return Some{id};
}

Option<View *> RenderServer::get_view(uid32 view)
{
  u32 index;
  if (!view_group.id_map.try_to_index(view, index))
  {
    return None;
  }
  return Some{view_group.views + index};
}

void RenderServer::remove_view(uid32 view)
{
  u32 index;
  if (!view_group.id_map.try_to_index(view, index))
  {
    return;
  }
  destroy_view(view_group.views[index], allocator);
  view_group.id_map.release(view, [](auto...) {
    // TODO(lamarrr): trivial relocate op
  });
}

Option<uid32> RenderServer::add_object(uid32 scene, uid32 parent,
                                       SceneObjectDesc const &desc)
{
  // resize cull masks for all views referring to the scene, on view added, on
  // view removed, on view for (u32 iview = 0; iview < view_group.num_views();
  // iview++)
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
  return None;
}

void RenderServer::remove_object(uid32 scene, uid32 object)
{
  u32 scene_index;
  if (!scene_group.id_map.try_to_index(scene, scene_index))
  {
    return;
  }

  Scene &scene_r = scene_group.scenes[scene_index];

  (void) scene_r.objects.id_map.try_release(object, [](auto...) {});

  // uid32 object_pass = scene_r.objects.node[object_index].pass;
  // uid32 object_pass_id = scene_r.objects.node[object_index].pass_object_id;
  // u32 object_pass_index =  pass_group.id_map[object_pass_id];
  // PassImpl const  pass = pass_group.passes[object_index];
  // pass.interface->
}

Option<uid32> RenderServer::add_directional_light(uid32                   scene,
                                                  DirectionalLight const &light)
{
  return None;
}

Option<uid32> RenderServer::add_point_light(uid32             scene,
                                            PointLight const &light)
{
  return None;
}

Option<uid32> RenderServer::add_spot_light(uid32 scene, SpotLight const &light)
{
  return None;
}

Option<uid32> RenderServer::add_area_light(uid32 scene, AreaLight const &light)
{
  return None;
}

Option<AmbientLight *> RenderServer::get_ambient_light(uid32 scene)
{
  return None;
}

Option<DirectionalLight *> RenderServer::get_directional_light(uid32 scene,
                                                               uid32 id)
{
  return None;
}

Option<PointLight *> RenderServer::get_point_light(uid32 scene, uid32 id)
{
  return None;
}

Option<SpotLight *> RenderServer::get_spot_light(uid32 scene, uid32 id)
{
  return None;
}

Option<AreaLight *> RenderServer::get_area_light(uid32 scene, uid32 id)
{
  return None;
}

void RenderServer::remove_directional_light(uid32 scene, uid32 id)
{
}

void RenderServer::remove_point_light(uid32 scene, uid32 id)
{
}

void RenderServer::remove_spot_light(uid32 scene, uid32 id)
{
}

void RenderServer::remove_area_light(uid32 scene, uid32 id)
{
}

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
    // filter_indirect();?
    // filter_masked();?
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
