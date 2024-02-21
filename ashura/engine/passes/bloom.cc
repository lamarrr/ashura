#include "ashura/engine/passes/bloom.h"
#include "ashura/engine/passes/gaussian_weights.h"

namespace ash
{

void BloomPass::init(Pass self, RenderServer *server, uid32 id)
{
  /// E' = Blur(E)
  /// D' = Blur(D) + E'
  /// C' = Blur(C) + D'
  /// B' = Blur(B) + C'
  /// A' = Blur(A) + B'
}

void BloomPass::deinit(Pass self, RenderServer *server)
{
}

void BloomPass::acquire_scene(Pass self, RenderServer *server, uid32 scene)
{
}

void BloomPass::release_scene(Pass self, RenderServer *server, uid32 scene)
{
}

void BloomPass::acquire_view(Pass self, RenderServer *server, uid32 view)
{
}
void BloomPass::release_view(Pass self, RenderServer *server, uid32 view)
{
}

void BloomPass::release_object(Pass self, RenderServer *server, uid32 scene,
                               uid32 object)
{
}

void BloomPass::begin(Pass self, RenderServer *server,
                      PassBeginInfo const *info)
{
}

void BloomPass::encode(Pass self, RenderServer *server,
                       PassEncodeInfo const *info)
{        // downsample to mip chains of 5 total
         // perform gaussian blur of the image
         // addittive composite back unto the first mip
}

void BloomPass::end(Pass self, RenderServer *server, PassEndInfo const *info)
{
}

}        // namespace ash
