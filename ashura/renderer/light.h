#pragma once
#include "ashura/std/types.h"

namespace ash
{

typedef Vec4                    AmbientLight;
typedef struct DirectionalLight DirectionalLight;
typedef struct PointLight       PointLight;
typedef struct SpotLight        SpotLight;
typedef struct AreaLight        AreaLight;
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
  f32  attenuation = 0;
  Vec4 color       = {};
  Vec3 position    = {};
};

/// SEE: https://learnopengl.com/Guest-Articles/2022/Area-Lights
/// https://docs.unrealengine.com/5.0/en-US/rectangular-area-lights-in-unreal-engine/
struct AreaLight
{
  Vec3 color     = {};
  Vec3 position  = {};
  Vec3 extent    = {};
  f32  intensity = 0;
  bool two_sided = false;
};

struct SkyLight
{
};

}        // namespace ash