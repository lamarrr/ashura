#include "ashura/engine/renderer.h"
#include "ashura/std/math.h"

namespace ash
{

void Renderer::transform(RenderServer *server)
{
  for (u32 i = 0; i < server->scene_group->id_map.num_valid(); i++)
  {
    Scene    &scene       = server->scene_group->scenes[i];
    u64 const num_objects = scene.objects_id_map.num_valid();
    for (u64 iobject = 0; iobject < num_objects; iobject++)
    {
      scene.object_global_transforms[iobject] =
          scene.object_global_transforms[scene.objects_id_map.unsafe_to_index(
              scene.object_nodes[iobject].parent)] *
          scene.object_local_transforms[iobject];
    }
  }
}

constexpr bool frustum_cull_aabb(Mat4 const       &camera_view_projection,
                                 Mat4Affine const &global_transform,
                                 Vec3 offset, Vec3 extent)
{
  constexpr auto to_vec4  = [](Vec3 a) { return Vec4{a.x, a.y, a.z, 1}; };
  Mat4 const     mvp      = camera_view_projection * global_transform;
  Vec4 const     edges[8] = {mvp * to_vec4(offset),
                             mvp * to_vec4(offset + Vec3{extent.x, 0, 0}),
                             mvp * to_vec4(offset + Vec3{extent.x, extent.y, 0}),
                             mvp * to_vec4(offset + Vec3{0, extent.y, 0}),
                             mvp * to_vec4(offset + Vec3{0, 0, extent.z}),
                             mvp * to_vec4(offset + Vec3{extent.x, 0, extent.z}),
                             mvp * to_vec4(offset + extent),
                             mvp * to_vec4(offset + Vec3{0, extent.y, extent.z})};
  u8             left     = 0;
  u8             right    = 0;
  u8             top      = 0;
  u8             bottom   = 0;
  u8             back     = 0;

  for (u8 i = 0; i < 8; i++)
  {
    Vec4 const edge = edges[i];
    if (edge.x < -edge.w)
    {
      left++;
    }
    if (edge.x > edge.w)
    {
      right++;
    }
    if (edge.y < -edge.w)
    {
      bottom++;
    }
    if (edge.y > edge.w)
    {
      top++;
    }
    if (edge.z < 0)
    {
      back++;
    }
  }

  return left == 8 || right == 8 || top == 8 || bottom == 8 || back == 8;
}

void Renderer::frustum_cull(RenderServer *server)
{
}

}        // namespace ash
