#pragma once
#include "ashura/std/types.h"

namespace ash
{

typedef Vec4                    AmbientLight;
typedef struct DirectionalLight DirectionalLight;
typedef struct PointLight       PointLight;
typedef struct SpotLight        SpotLight;
typedef struct SkyLight         SkyLight;

struct DirectionalLight
{
  Vec3 direction = {};
  Vec4 color     = {};
};

struct PointLight
{
  Vec4 color       = {};
  Vec3 position    = {};
  f32  attenuation = 0;
};

struct SpotLight
{
  Vec3 direction   = {};
  f32  cutoff      = 0;
  Vec4 color       = {};
  Vec3 position    = {};
  f32  attenuation = 0;
};

}        // namespace ash
