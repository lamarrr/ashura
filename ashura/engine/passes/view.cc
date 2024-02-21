#include "ashura/engine/passes/view.h"

namespace ash
{

void ViewPass::init(Pass self, RenderServer *server, uid32 id)
{
}

void ViewPass::deinit(Pass self, RenderServer *server)
{
}

void ViewPass::acquire_scene(Pass self, RenderServer *server, uid32 scene)
{
}

void ViewPass::release_scene(Pass self, RenderServer *server, uid32 scene)
{
}

void ViewPass::acquire_view(Pass self, RenderServer *server, uid32 view)
{
}
void ViewPass::release_view(Pass self, RenderServer *server, uid32 view)
{
}

void ViewPass::release_object(Pass self, RenderServer *server, uid32 scene,
                              uid32 object)
{
}

void ViewPass::begin(Pass self, RenderServer *server, PassBeginInfo const *info)
{
}

void ViewPass::encode(Pass self, RenderServer *server,
                      PassEncodeInfo const *info)
{
}

void ViewPass::end(Pass self, RenderServer *server, PassEndInfo const *info)
{
}

}        // namespace ash
