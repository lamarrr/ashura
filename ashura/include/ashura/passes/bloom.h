#pragma once
#include <string_view>

#include "ashura/lgfx.h"
#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "stx/fn.h"
#include "stx/vec.h"

namespace ash
{
namespace lgfx
{

struct BloomCapturePassExecutor
{
  // we need to sync each generated frame?
  void init(BlurCapturePass);
  void execute()
  {
    // take draw lists
  }
};

inline void bloom3d_pass()
{
  // SETUP
  // RENDER
}

}        // namespace lgfx
}        // namespace ash
