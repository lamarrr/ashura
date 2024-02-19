#include "ashura/engine/passes/blur.h"

namespace ash
{

void BlurPass::init(Pass self, RenderServer *server, uid32 id)
{
}

void BlurPass::deinit(Pass self, RenderServer *server)
{
}

void BlurPass::acquire_scene(Pass self, RenderServer *server, uid32 scene)
{
}

void BlurPass::release_scene(Pass self, RenderServer *server, uid32 scene)
{
}

void BlurPass::acquire_view(Pass self, RenderServer *server, uid32 view)
{
}

void BlurPass::release_view(Pass self, RenderServer *server, uid32 view)
{
}

void BlurPass::release_object(Pass self, RenderServer *server, uid32 scene,
                              uid32 object)
{
}

void BlurPass::begin(Pass self, RenderServer *server, uid32 view,
                     gfx::CommandEncoderImpl const *encoder)
{
}

void BlurPass::encode(Pass self, RenderServer *server, uid32 view,
                      PassEncodeInfo const *info)
{
    gfx::CommandEncoderImpl enc= info->command_encoder;
    // enc->copy_image(enc.self, );
}

void BlurPass::end(Pass self, RenderServer *server, uid32 view,
                   gfx::CommandEncoderImpl const *encoder)
{
}

}        // namespace ash