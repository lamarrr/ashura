#include "ashura/renderer/renderer.h"
#include "ashura/renderer/system.h"
#include "ashura/renderer/widget.h"
#include "gtest/gtest.h"

ash::Logger panic_logger;

TEST(RendererTest, Scene)
{
  using ash::operator""_span;
  ash::RenderServer server{};
  ash::uid32 scene_id = server.add_scene("ROOT SCENE"_span).unwrap();
  ash::uid32 light_id = server.add_point_light(scene_id, {}).unwrap();
  // server.render_();
}
