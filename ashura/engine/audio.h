#pragma once
#include "ashura/std/types.h"
#include <math.h>

namespace ash
{

f32 db_to_volume(f32 db)
{
  return std::powf(10, 0.05f * db);
}

f32 volume_to_db(f32 volume)
{
  return 20 * std::logf(volume);
}

}        // namespace ash
