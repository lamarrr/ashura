#pragma once

#include "ashura/image.h"
#include "ashura/primitives.h"

namespace rgk {

struct vertex {
  ash::vec3 position;
  ash::vec2 st;
};

struct material {
  ash::gfx::image albedo = 0;
  ash::gfx::image normal = 0;
  ash::gfx::image metalic = 0;
  ash::gfx::image roughness = 0;
  ash::gfx::image ambient_occlusion = 0;
};

struct blur_effect {
  ash::extent offset;
};

}  // namespace rgk
