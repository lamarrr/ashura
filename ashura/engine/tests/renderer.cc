#include "ashura/engine/renderer.h"
#include "ashura/engine/system.h"
#include "ashura/engine/widget.h"
#include "gtest/gtest.h"

ash::Logger panic_logger;

TEST(RendererTest, Scene)
{
  ash::RenderServer server{};
  server.frustum_cull_().unwrap();
  server.transform_();
  server.sort_();
  ash::uid32 scene_id = server.add_scene("ROOT SCENE").unwrap();
  ash::uid32 light_id = server.add_point_light(scene_id, {}).unwrap();
  server.frustum_cull_().unwrap();
  server.transform_();
  server.sort_();
}
