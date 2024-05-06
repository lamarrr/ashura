#include "ashura/renderer/renderer.h"
#include "ashura/std/math.h"
#include "ashura/std/range.h"
#include "ashura/std/source_location.h"
/*
#define ENSURE(description, ...)                                              \
  do                                                                          \
  {                                                                           \
    if (!(__VA_ARGS__))                                                       \
    {                                                                         \
      default_logger->panic(description, " (expression: " #__VA_ARGS__,          \
                         ") [function: ", SourceLocation::current().function, \
                         ", file: ", SourceLocation::current().file, ":",     \
                         SourceLocation::current().line, ":",                 \
                         SourceLocation::current().column, "]");              \
    }                                                                         \
  } while (false)

namespace ash
{

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
}

void destroy_scene_group(SceneGroup &group)
{
  for (u32 i = 0; i < group.scenes.size(); i++)
  {
    destroy_scene(group.scenes[i]);
  }
  group.id_map.reset(group.scenes);
}

Option<uid> RenderServer::add_scene(Span<char const> name)
{
  uid id;

  if (!scene_group.id_map.push(
          [&](uid in_id, u32) {
            id = in_id;
            ENSURE("", scene_group.scenes.push(Scene{.name = name}));
          },
          scene_group.scenes))
  {
    return None;
  }

  for (PassImpl const &pass : pass_group.passes)
  {
    pass.interface->acquire_scene(pass.self, this, id);
  }

  return Some{id};
}

Option<Scene *> RenderServer::get_scene(uid scene)
{
  u32 index;
  if (!scene_group.id_map.try_to_index(scene, index))
  {
    return None;
  }
  return Some{scene_group.scenes.data() + index};
}

void RenderServer::remove_scene(uid scene)
{
  u32 index;
  if (!scene_group.id_map.try_to_index(scene, index))
  {
    return;
  }

  for (PassImpl const &pass : pass_group.passes)
  {
    pass.interface->release_scene(pass.self, this, scene);
  }

  destroy_scene(scene_group.scenes[index]);
  scene_group.id_map.erase(scene, scene_group.scenes);
}

Option<uid> RenderServer::add_object(uid pass, uid pass_object_id,
                                       uid scene_id, uid parent_id,
                                       SceneObjectDesc const &desc)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<uid> {
    SceneNode *parent = nullptr;

    if (parent_id != UID_INVALID)
    {
      if (!scene->objects.id_map.try_get(parent_id, parent,
                                         scene->objects.node))
      {
        return None;
      }
    }

    u32 const   depth = parent == nullptr ? 0 : (parent->depth + 1);
    uid const next_sibling =
        parent == nullptr ? UID_INVALID : parent->first_child;
    uid object_id;
    if (!scene->objects.id_map.push(
            [&](uid in_object_id, u32) {
              object_id = in_object_id;
              ENSURE("", scene->objects.aabb.push(desc.aabb));
              ENSURE("", scene->objects.global_transform.push());
              ENSURE("",
                     scene->objects.is_transparent.push(desc.is_transparent));
              ENSURE("", scene->objects.local_transform.push());
              ENSURE("", scene->objects.node.push(
                             SceneNode{.parent       = parent_id,
                                       .next_sibling = next_sibling,
                                       .depth        = depth,
                                       .pass         = pass,
                                       .pass_object  = pass_object_id}));
              ENSURE("", scene->objects.z_index.push(desc.z_index));
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

static void collect_nodes(Scene const &scene, Vec<uid> &ids, uid id)
{
  SceneNode const &object = scene.objects.node[scene.objects.id_map[id]];
  ENSURE("", ids.push(object.pass_object));

  uid child_id = object.first_child;

  while (child_id != UID_INVALID)
  {
    collect_nodes(scene, ids, child_id);
    child_id = scene.objects.node[scene.objects.id_map[child_id]].next_sibling;
  }
}

static void remove_node(RenderServer &server, uid scene_id, Scene &scene,
                        uid scene_object_id, SceneNode &object)
{
  Vec<uid> ids;
  collect_nodes(scene, ids, scene_object_id);

  for (uid object_id : ids)
  {
    uid pass_id = scene.objects.node[scene.objects.id_map[object_id]].pass;
    PassImpl const &pass =
        server.pass_group.passes[server.pass_group.id_map[pass_id]];
    pass.interface->release_object(pass.self, &server, scene_id, object_id);
  }

  uid const parent_id = object.parent;
  if (parent_id != UID_INVALID)
  {
    SceneNode &parent = scene.objects.node[scene.objects.id_map[parent_id]];
    if (parent.first_child == scene_object_id)
    {
      if (object.next_sibling != UID_INVALID)
      {
        parent.first_child = object.next_sibling;
      }
      else
      {
        parent.first_child = UID_INVALID;
      }
    }
    else
    {
      uid sibling_id = parent.first_child;

      while (sibling_id != UID_INVALID)
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

  for (uid id : ids)
  {
    scene.objects.id_map.erase(
        id, scene.objects.aabb, scene.objects.global_transform,
        scene.objects.is_transparent, scene.objects.local_transform,
        scene.objects.node, scene.objects.z_index);
  }

  ids.reset();
}

void RenderServer::remove_object(uid scene_id, uid object_id)
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

Option<uid> RenderServer::add_directional_light(uid scene_id,
                                                  DirectionalLight const &light)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<uid> {
    uid light_id;
    if (!scene->directional_lights_id_map.push(
            [&](uid in_id, u32) {
              light_id = in_id;
              ENSURE("", scene->directional_lights.push(light));
            },
            scene->directional_lights))
    {
      return None;
    }

    return Some{light_id};
  });
}

Option<uid> RenderServer::add_point_light(uid             scene_id,
                                            PointLight const &light)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<uid> {
    uid light_id;
    if (!scene->point_lights_id_map.push(
            [&](uid in_id, u32) {
              light_id = in_id;
              ENSURE("", scene->point_lights.push(light));
            },
            scene->point_lights))
    {
      return None;
    }

    return Some{light_id};
  });
}

Option<uid> RenderServer::add_spot_light(uid            scene_id,
                                           SpotLight const &light)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<uid> {
    uid light_id;
    if (!scene->spot_lights_id_map.push(
            [&](uid in_id, u32) {
              light_id = in_id;
              ENSURE("", scene->spot_lights.push(light));
            },
            scene->spot_lights))
    {
      return None;
    }

    return Some{light_id};
  });
}

Option<uid> RenderServer::add_area_light(uid            scene_id,
                                           AreaLight const &light)
{
  return get_scene(scene_id).and_then([&](Scene *scene) -> Option<uid> {
    uid light_id;
    if (!scene->area_lights_id_map.push(
            [&](uid in_id, u32) {
              light_id = in_id;
              ENSURE("", scene->area_lights.push(light));
            },
            scene->area_lights))
    {
      return None;
    }

    return Some{light_id};
  });
}

Option<AmbientLight *> RenderServer::get_ambient_light(uid scene_id)
{
  return get_scene(scene_id).map(
      [&](Scene *scene) { return &scene->ambient_light; });
}

Option<DirectionalLight *> RenderServer::get_directional_light(uid scene_id,
                                                               uid light_id)
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

Option<PointLight *> RenderServer::get_point_light(uid scene_id,
                                                   uid light_id)
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

Option<SpotLight *> RenderServer::get_spot_light(uid scene_id, uid light_id)
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

Option<AreaLight *> RenderServer::get_area_light(uid scene_id, uid light_id)
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

void RenderServer::remove_directional_light(uid scene_id, uid light_id)
{
  get_scene(scene_id).match(
      [&](Scene *scene) {
        (void) scene->directional_lights_id_map.try_erase(
            light_id, scene->directional_lights);
      },
      [] {});
}

void RenderServer::remove_point_light(uid scene_id, uid light_id)
{
  get_scene(scene_id).match(
      [&](Scene *scene) {
        (void) scene->point_lights_id_map.try_erase(light_id,
                                                    scene->point_lights);
      },
      [] {});
}

void RenderServer::remove_spot_light(uid scene_id, uid light_id)
{
  get_scene(scene_id).match(
      [&](Scene *scene) {
        (void) scene->spot_lights_id_map.try_erase(light_id,
                                                   scene->spot_lights);
      },
      [] {});
}

void RenderServer::remove_area_light(uid scene_id, uid light_id)
{
  get_scene(scene_id).match(
      [&](Scene *scene) {
        (void) scene->area_lights_id_map.try_erase(light_id,
                                                   scene->area_lights);
      },
      [] {});
}

void RenderServer::transform()
{
  for (Scene &scene : scene_group.scenes)
  {

  }
}


// transform objects from root object space to clip space using view's camera
Result<Void, Error> RenderServer::frustum_cull()
{
  for (View &view : view_group.views)
  {
    Scene    &scene       = scene_group.scenes[scene_group.id_map[view.scene]];
    u32 const num_objects = scene.objects.id_map.size();
    if (!view.is_object_visible.resize_uninitialized(num_objects))
    {
      return Err{Error::OutOfMemory};
    }
    
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
Result<Void, Error>
    RenderServer::encode_view(uid                          view_id,
                              gfx::CommandEncoderImpl const &encoder,
                              ViewAttachments const         &attachments)
{
  u32 const view_index  = view_group.id_map[view_id];
  View     &view        = view_group.views[view_index];
  u32 const scene_index = scene_group.id_map[view.scene];
  Scene    &scene       = scene_group.scenes[scene_index];
  u32 const num_objects = scene.objects.id_map.size();
  if (!view.sort_indices.resize_uninitialized(num_objects))
  {
    return Err{Error::OutOfMemory};
  }

  Vec<PassBinding> bindings;
  if (!bindings.resize_defaulted(pass_group.id_map.size()))
  {
    return Err{Error::OutOfMemory};
  }

  for (u32 i = 0; i < pass_group.id_map.size(); i++)
  {
    PassImpl const &pass = pass_group.passes[i];
    PassBeginInfo   info{.view        = view_id,
                         .encoder     = encoder,
                         .attachments = &attachments,
                         .binding     = &bindings[i]};
    pass.interface->begin(pass.self, this, &info);
  }

  for (u32 i = 0; i < num_objects; i++)
  {
    view.sort_indices[i] = i;
  }

  u32 const num_visible =
      binary_partition(
          view.sort_indices,
          [&](u32 index) { return view.is_object_visible[index]; }) -
      view.sort_indices.begin();
  Span<u32> indices = to_span(view.sort_indices).slice(0, num_visible);
  indirect_sort(scene.objects.z_index, indices);
  for_each_partition_indirect(
      scene.objects.z_index, indices, [&](Span<u32> indices) {
        binary_partition(indices, [&](u32 index) {
          return !scene.objects.is_transparent[index];
        });
        for_each_partition_indirect(
            scene.objects.is_transparent, indices, [&](Span<u32> indices) {
              indirect_sort(scene.objects.node, indices,
                            [](SceneNode const &a, SceneNode const &b) {
                              return a.pass < b.pass;
                            });
              for_each_partition_indirect(
                  scene.objects.node, indices,
                  [&](Span<u32> indices) {
                    uid const pass_id = scene.objects.node[indices[0]].pass;
                    u32 const   pass_index    = pass_group.id_map[pass_id];
                    PassImpl const      &pass = pass_group.passes[pass_index];
                    PassEncodeInfo const info{
                        .view        = view_id,
                        .encoder     = encoder,
                        .attachments = &attachments,
                        .binding     = bindings[pass_index],
                        .is_transparent =
                            scene.objects.is_transparent[indices[0]],
                        .z_index = scene.objects.z_index[indices[0]],
                        .indices = indices};
                    pass.interface->encode(pass.self, this, &info);
                  },
                  [](SceneNode const &a, SceneNode const &b) {
                    return a.pass == b.pass;
                  });
            });
      });

  for (u32 i = 0; i < pass_group.id_map.size(); i++)
  {
    PassImpl const &pass = pass_group.passes[i];
    PassEndInfo     info{.view        = view_id,
                         .encoder     = encoder,
                         .attachments = &attachments,
                         .binding     = bindings[i]};
    pass.interface->end(pass.self, this, &info);
  }

  bindings.reset();

  return Ok<Void>{};
}

Result<ViewAttachments, int> create_view_attachments(RenderServer &server,
                                                     View const   &view)
{
  ViewAttachments attachments;

  if (view.config.color_format != gfx::Format::Undefined &&
      view.config.extent.x > 0 && view.config.extent.y > 0)
  {
    gfx::SampleCount sample_count = gfx::SampleCount::Count1;
    switch (view.config.aa.technique)
    {
      case AATechnique::FXAA:
        break;
      case AATechnique::MSAA:
        sample_count = view.config.aa.msaa.sample_count;
        break;
      default:
        break;
    }

    attachments.color_attachment =
        Some{server
                 .request_scratch_attachment(
                     gfx::ImageDesc{.label   = nullptr,
                                    .type    = gfx::ImageType::Type2D,
                                    .format  = view.config.color_format,
                                    .usage   = gfx::ImageUsage::ColorAttachment,
                                    .aspects = gfx::ImageAspects::Color,
                                    .extent =
                                        gfx::Extent3D{
                                            view.config.extent.x,
                                            view.config.extent.y,
                                            1,
                                        },
                                    .mip_levels   = 1,
                                    .array_layers = 1,
                                    .sample_count = sample_count})
                 .unwrap()};

    if (sample_count != gfx::SampleCount::Count1)
    {
      attachments.resolve_color_attachment =
          Some{server
                   .request_scratch_attachment(
                       gfx::ImageDesc{.label  = nullptr,
                                      .type   = gfx::ImageType::Type2D,
                                      .format = view.config.color_format,
                                      .usage = gfx::ImageUsage::ColorAttachment,
                                      .aspects = gfx::ImageAspects::Color,
                                      .extent =
                                          gfx::Extent3D{
                                              view.config.extent.x,
                                              view.config.extent.y,
                                              1,
                                          },
                                      .mip_levels   = 1,
                                      .array_layers = 1,
                                      .sample_count = gfx::SampleCount::Count1})
                   .unwrap()};
    }
  }

  if (view.config.depth_stencil_format != gfx::Format::Undefined &&
      view.config.extent.x > 0 && view.config.extent.y > 0)
  {
    attachments.depth_stencil_attachment =
        Some{server
                 .request_scratch_attachment(gfx::ImageDesc{
                     .label  = nullptr,
                     .type   = gfx::ImageType::Type2D,
                     .format = view.config.depth_stencil_format,
                     .usage  = gfx::ImageUsage::DepthStencilAttachment,
                     .aspects =
                         gfx::ImageAspects::Depth | gfx::ImageAspects::Stencil,
                     .extent =
                         gfx::Extent3D{
                             view.config.extent.x,
                             view.config.extent.y,
                             1,
                         },
                     .mip_levels   = 1,
                     .array_layers = 1,
                     .sample_count = gfx::SampleCount::Count1})
                 .unwrap()};
  }

  return Ok{attachments};
}

void RenderServer::tick()
{
  transform();
  frustum_cull().unwrap();
  gfx::FrameInfo frame_info =
      device->get_frame_info(device.self, frame_context);
  gfx::CommandEncoderImpl const encoder =
      frame_info.command_encoders[frame_info.current_command_encoder];

  for (PassImpl const &pass : pass_group.passes)
  {
    pass.interface->begin_frame(pass.self, this, &encoder);
  }

  if (view_group.root_view != UID_INVALID)
  {
    // render scene to offscreen image, and then perform
    // post-fx, and then composite back to parent view
    View &view = view_group.views[view_group.id_map[view_group.root_view]];
    ViewAttachments attachments = create_view_attachments(*this, view).unwrap();
    encode_view(view_group.root_view, encoder, attachments);
    if (attachments.color_attachment)
    {
      release_scratch_attachment(attachments.color_attachment.value());
    }
    if (attachments.depth_stencil_attachment)
    {
      release_scratch_attachment(attachments.color_attachment.value());
    }

    if (attachments.resolve_color_attachment)
    {
      release_scratch_attachment(attachments.color_attachment.value());
    }
  }

  for (PassImpl const &pass : pass_group.passes)
  {
    pass.interface->end_frame(pass.self, this, &encoder);
  }

  // TODO(lamarrr): copy resolved attachment to screen
  // move attachment release here
}

}        // namespace ash
*/