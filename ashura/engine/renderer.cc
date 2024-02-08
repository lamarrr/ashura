#include "ashura/engine/renderer.h"
#include "ashura/std/math.h"
#include "ashura/std/range.h"

namespace ash
{

void destroy_pass_group(PassGroup &group)
{
  group.id_map.reset(group.passes);
}

void destroy_scene(Scene &scene)
{
  scene.directional_lights_id_map.reset(scene.directional_lights);
  scene.point_lights_id_map.reset(scene.point_lights);
  scene.spot_lights_id_map.reset(scene.spot_lights);
  scene.area_lights_id_map.reset(scene.area_lights);
  scene.objects.id_map.reset(scene.objects.node, scene.objects.local_transform,
                             scene.objects.global_transform, scene.objects.aabb,
                             scene.objects.z_index,
                             scene.objects.is_transparent);
  scene.sort_indices.reset();
}

void destroy_scene_group(SceneGroup &group)
{
  for (u32 i = 0; i < group.scenes.size(); i++)
  {
    destroy_scene(group.scenes[i]);
  }
  group.id_map.reset(group.scenes);
}

void destroy_view(View &view)
{
  view.is_object_visible.reset();
}

Option<PassImpl *> RenderServer::get_pass(uid32 pass)
{
  u32 index;
  if (!pass_group.id_map.try_to_index(pass, index))
  {
    return None;
  }

  return Some{&pass_group.passes[index]};
}

Option<uid32> RenderServer::get_pass_id(char const *name)
{
  for (u32 i = 0; i < pass_group.id_map.size(); i++)
  {
    if (strcmp(pass_group.passes[i].name, name) == 0)
    {
      return Some{pass_group.id_map.to_id(i)};
    }
  }
  return None;
}

Option<uid32> RenderServer::register_pass(PassImpl pass)
{
  uid32 id;

  if (!pass_group.id_map.push(
          [&](u32 in_id, u32) {
            id = in_id;
            (void) pass_group.passes.push(pass);
          },
          pass_group.passes))
  {
    return None;
  }

  return Some{id};
}

Option<uid32> RenderServer::add_scene(char const *name)
{
  uid32 id;

  if (!scene_group.id_map.push(
          [&](uid32 in_id, u32) {
            id = in_id;
            (void) scene_group.scenes.push(Scene{.name = name});
          },
          scene_group.scenes))
  {
    return None;
  }

  for (PassImpl &pass : pass_group.passes)
  {
    pass.interface->acquire_scene(pass.self, this, id);
  }

  return Some{id};
}

Option<Scene *> RenderServer::get_scene(uid32 scene)
{
  u32 index;
  if (!scene_group.id_map.try_to_index(scene, index))
  {
    return None;
  }
  return Some{scene_group.scenes.data() + index};
}

void RenderServer::remove_scene(uid32 scene)
{
  u32 index;
  if (!scene_group.id_map.try_to_index(scene, index))
  {
    return;
  }

  for (PassImpl &pass : pass_group.passes)
  {
    pass.interface->release_scene(pass.self, this, scene);
  }

  destroy_scene(scene_group.scenes[index]);
  scene_group.id_map.erase(scene, scene_group.scenes);
}

Option<uid32> RenderServer::add_view(uid32 scene, char const *name,
                                     Camera const &camera)
{
  if (!scene_group.id_map.is_valid_id(scene))
  {
    return None;
  }

  uid32 id;
  if (!view_group.id_map.push(
          [&](uid32 in_id, u32) {
            id = in_id;
            (void) view_group.views.push(
                View{.name = name, .camera = camera, .scene = scene});
          },
          view_group.views))
  {
    return None;
  }

  for (PassImpl &pass : pass_group.passes)
  {
    pass.interface->acquire_view(pass.self, this, id);
  }

  return Some{id};
}

Option<View *> RenderServer::get_view(uid32 view)
{
  u32 index;
  if (!view_group.id_map.try_to_index(view, index))
  {
    return None;
  }
  return Some{view_group.views.data() + index};
}

void RenderServer::remove_view(uid32 view)
{
  u32 index;
  if (!view_group.id_map.try_to_index(view, index))
  {
    return;
  }

  for (PassImpl &pass : pass_group.passes)
  {
    pass.interface->release_view(pass.self, this, view);
  }

  destroy_view(view_group.views[index]);
  view_group.id_map.erase(view, view_group.views);
}

Option<uid32> RenderServer::add_object(uid32 pass, uid32 pass_object_id,
                                       uid32 scene_id, uid32 parent_id,
                                       SceneObjectDesc const &desc)
{
  // resize cull masks for all views referring to the scene, on view added, on
  // view removed, on view
  //
  // TODO(lamarrr): call on view updated, resize bit masks and all
  // what about passes that may want to access the view?
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<uid32> {
    SceneNode *parent = nullptr;

    if (parent_id != INVALID_UID32)
    {
      if (!scene->objects.id_map.try_get(parent_id, parent,
                                         scene->objects.node))
      {
        return None;
      }
    }

    u32   depth = parent == nullptr ? 0 : (parent->depth + 1);
    uid32 next_sibling =
        parent == nullptr ? INVALID_UID32 : parent->first_child;
    uid32 object_id;
    if (!scene->objects.id_map.push(
            [&](uid32 in_object_id, u32) {
              object_id = in_object_id;
              (void) scene->objects.aabb.push(desc.aabb);
              (void) scene->objects.global_transform.push();
              (void) scene->objects.is_transparent.push(desc.is_transparent);
              (void) scene->objects.local_transform.push();
              (void) scene->objects.node.push(
                  SceneNode{.parent         = parent_id,
                            .next_sibling   = next_sibling,
                            .depth          = depth,
                            .pass           = pass,
                            .pass_object_id = pass_object_id});
              (void) scene->objects.z_index.push(desc.z_index);
            },
            scene->objects.aabb, scene->objects.global_transform,
            scene->objects.is_transparent, scene->objects.local_transform,
            scene->objects.node, scene->objects.z_index))
    {
      return None;
    }

    if (parent != nullptr)
    {
      parent->first_child = object_id;
    }

    return Some{object_id};
  });
}

struct ObjectReleaseInfo
{
  uid32 scene_object_id = INVALID_UID32;
  uid32 pass_id         = INVALID_UID32;
  uid32 pass_object_id  = INVALID_UID32;
};

static void collect_nodes(Scene &scene, Vec<ObjectReleaseInfo> &vec, uid32 id)
{
  SceneNode &object = scene.objects.node[scene.objects.id_map[id]];
  // TODO(lamarrr): check
  vec.push(ObjectReleaseInfo{.scene_object_id = id,
                             .pass_id         = object.pass,
                             .pass_object_id  = object.pass_object_id});

  uid32 child_id = object.first_child;

  while (child_id != INVALID_UID32)
  {
    collect_nodes(scene, vec, child_id);
    child_id = scene.objects.node[scene.objects.id_map[child_id]].next_sibling;
  }
}

static void remove_node(RenderServer &server, uid32 scene_id, Scene &scene,
                        uid32 scene_object_id, SceneNode &object)
{
  Vec<ObjectReleaseInfo> infos;
  collect_nodes(scene, infos, scene_object_id);

  for (ObjectReleaseInfo const &info : infos)
  {
    PassImpl const &pass =
        server.pass_group.passes[server.pass_group.id_map[info.pass_id]];
    PassObjectReleaseInfo pass_info{.scene_id        = scene_id,
                                    .scene_object_id = info.scene_object_id,
                                    .pass_object_id  = info.pass_object_id};
    pass.interface->release_object(pass.self, &server, &pass_info);
  }

  uid32 const parent_id = object.parent;
  if (parent_id != INVALID_UID32)
  {
    SceneNode &parent = scene.objects.node[scene.objects.id_map[parent_id]];
    if (parent.first_child == scene_object_id)
    {
      if (object.next_sibling != INVALID_UID32)
      {
        parent.first_child = object.next_sibling;
      }
      else
      {
        parent.first_child = INVALID_UID32;
      }
    }
    else
    {
      uid32 sibling_id = parent.first_child;

      while (sibling_id != INVALID_UID32)
      {
        SceneNode &sibling =
            scene.objects.node[scene.objects.id_map[sibling_id]];
        if (sibling.next_sibling == scene_object_id)
        {
          sibling.next_sibling = object.next_sibling;
          break;
        }
        sibling_id = sibling.next_sibling;
      }
    }
  }

  for (ObjectReleaseInfo const &info : infos)
  {
    scene.objects.id_map.erase(info.scene_object_id, scene.objects.aabb,
                               scene.objects.global_transform,
                               scene.objects.is_transparent,
                               scene.objects.local_transform,
                               scene.objects.node, scene.objects.z_index);
  }

  infos.reset();
}

void RenderServer::remove_object(uid32 scene_id, uid32 object_id)
{
  get_scene(scene_id).match(
      [&](Scene *scene) {
        SceneNode *object;
        if (!scene->objects.id_map.try_get(object_id, object,
                                           scene->objects.node))
        {
          return;
        }
        remove_node(*this, scene_id, *scene, object_id, *object);
      },
      [] {});
}

Option<uid32> RenderServer::add_directional_light(uid32 scene_id,
                                                  DirectionalLight const &light)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<uid32> {
    uid32 light_id;
    if (!scene->directional_lights_id_map.push(
            [&](uid32 in_id, u32) {
              light_id = in_id;
              (void) scene->directional_lights.push(light);
            },
            scene->directional_lights))
    {
      return None;
    }

    return Some{light_id};
  });
}

Option<uid32> RenderServer::add_point_light(uid32             scene_id,
                                            PointLight const &light)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<uid32> {
    uid32 light_id;
    if (!scene->point_lights_id_map.push(
            [&](uid32 in_id, u32) {
              light_id = in_id;
              (void) scene->point_lights.push(light);
            },
            scene->point_lights))
    {
      return None;
    }

    return Some{light_id};
  });
}

Option<uid32> RenderServer::add_spot_light(uid32            scene_id,
                                           SpotLight const &light)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<uid32> {
    uid32 light_id;
    if (!scene->spot_lights_id_map.push(
            [&](uid32 in_id, u32) {
              light_id = in_id;
              (void) scene->spot_lights.push(light);
            },
            scene->spot_lights))
    {
      return None;
    }

    return Some{light_id};
  });
}

Option<uid32> RenderServer::add_area_light(uid32            scene_id,
                                           AreaLight const &light)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<uid32> {
    uid32 light_id;
    if (!scene->area_lights_id_map.push(
            [&](uid32 in_id, u32) {
              light_id = in_id;
              (void) scene->area_lights.push(light);
            },
            scene->area_lights))
    {
      return None;
    }

    return Some{light_id};
  });
}

Option<AmbientLight *> RenderServer::get_ambient_light(uid32 scene_id)
{
  return get_scene(scene_id).map(
      [&](Scene *scene) { return &scene->ambient_light; });
}

Option<DirectionalLight *> RenderServer::get_directional_light(uid32 scene_id,
                                                               uid32 light_id)
{
  return get_scene(scene_id).and_then(
      [&](Scene *scene) -> Option<DirectionalLight *> {
        DirectionalLight *light;
        if (!scene->directional_lights_id_map.try_get(
                light_id, light, scene->directional_lights))
        {
          return None;
        }
        return Some{light};
      });
}

Option<PointLight *> RenderServer::get_point_light(uid32 scene_id,
                                                   uid32 light_id)
{
  return get_scene(scene_id).and_then(
      [&](Scene *scene) -> Option<PointLight *> {
        PointLight *light;
        if (!scene->point_lights_id_map.try_get(light_id, light,
                                                scene->point_lights))
        {
          return None;
        }
        return Some{light};
      });
}

Option<SpotLight *> RenderServer::get_spot_light(uid32 scene_id, uid32 light_id)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<SpotLight *> {
    SpotLight *light;
    if (!scene->spot_lights_id_map.try_get(light_id, light, scene->spot_lights))
    {
      return None;
    }
    return Some{light};
  });
}

Option<AreaLight *> RenderServer::get_area_light(uid32 scene_id, uid32 light_id)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<AreaLight *> {
    AreaLight *light;
    if (!scene->area_lights_id_map.try_get(light_id, light, scene->area_lights))
    {
      return None;
    }
    return Some{light};
  });
}

void RenderServer::remove_directional_light(uid32 scene_id, uid32 light_id)
{
  get_scene(scene_id).match(
      [&](Scene *scene) {
        (void) scene->directional_lights_id_map.try_erase(
            light_id, scene->directional_lights);
      },
      [] {});
}

void RenderServer::remove_point_light(uid32 scene_id, uid32 light_id)
{
  get_scene(scene_id).match(
      [&](Scene *scene) {
        (void) scene->point_lights_id_map.try_erase(light_id,
                                                    scene->point_lights);
      },
      [] {});
}

void RenderServer::remove_spot_light(uid32 scene_id, uid32 light_id)
{
  get_scene(scene_id).match(
      [&](Scene *scene) {
        (void) scene->spot_lights_id_map.try_erase(light_id,
                                                   scene->spot_lights);
      },
      [] {});
}

void RenderServer::remove_area_light(uid32 scene_id, uid32 light_id)
{
  get_scene(scene_id).match(
      [&](Scene *scene) {
        (void) scene->area_lights_id_map.try_erase(light_id,
                                                   scene->area_lights);
      },
      [] {});
}

// transform views from object-space to root-object space
void RenderServer::transform_()
{
  for (Scene &scene : scene_group.scenes)
  {
    for (u32 i = 0; i < scene.objects.id_map.size(); i++)
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
/// -w <= x <= w
/// -w <= y <= w
///  0 <= z <= w
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
  for (View &view : view_group.views)
  {
    Scene    &scene       = scene_group.scenes[scene_group.id_map[view.scene]];
    u32 const num_objects = scene.objects.id_map.size();
    for (u32 i = 0; i < num_objects; i++)
    {
      view.is_object_visible[i] =
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
  u32 num_scenes = scene_group.scenes.size();
  for (u32 iscene = 0; iscene < num_scenes; iscene++)
  {
    Scene &scene = scene_group.scenes[iscene];
    // TODO(lamarrr): don't perform any sorting on frustum-culled objects
    // filter_indirect();?
    // filter_masked();?
    indirect_sort(scene.objects.z_index, to_span(scene.sort_indices));
    for_each_partition_indirect(
        scene.objects.z_index, to_span(scene.sort_indices),
        [&](Span<u32> partition_indices) {
          // TODO(lamarrr): partition, not sort
          indirect_sort(static_cast<BitSpan<u64>>(scene.objects.is_transparent),
                        partition_indices);
          for_each_partition_indirect(
              static_cast<BitSpan<u64>>(scene.objects.is_transparent),
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
                      pass.interface->sort(pass.self, this,
                                           scene_group.id_map.to_id(iscene),
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
