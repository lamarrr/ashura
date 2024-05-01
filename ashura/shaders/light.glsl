#ifndef LIGHT_GLSL
#define LIGHT_GLSL

#version 450
#extension GL_GOOGLE_include_directive : require

float getSquareFalloffAttenuation(vec3 posToLight, float lightInvRadius)
{
  float distanceSquare = dot(posToLight, posToLight);
  float factor         = distanceSquare * lightInvRadius * lightInvRadius;
  float smoothFactor   = max(1.0 - factor * factor, 0.0);
  return (smoothFactor * smoothFactor) / max(distanceSquare, 1e-4);
}

float getSpotAngleAttenuation(vec3 l, vec3 lightDir, float innerAngle,
                              float outerAngle)
{
  // the scale and offset computations can be done CPU-side
  float cosOuter    = cos(outerAngle);
  float spotScale   = 1.0 / max(cos(innerAngle) - cosOuter, 1e-4);
  float spotOffset  = -cosOuter * spotScale;
  float cd          = dot(normalize(-lightDir), l);
  float attenuation = clamp(cd * spotScale + spotOffset, 0.0, 1.0);
  return attenuation * attenuation;
}

vec3 evaluatePunctualLight()
{
  vec3  l           = normalize(posToLight);
  float NoL         = clamp(dot(n, l), 0.0, 1.0);
  vec3  posToLight  = lightPosition - worldPosition;
  float attenuation = getSquareFalloffAttenuation(posToLight, lightInvRadius);
  attenuation *= getSpotAngleAttenuation(l, lightDir, innerAngle, outerAngle);
  vec3 luminance =
      (BSDF(v, l) * lightIntensity * attenuation * NoL) * lightColor;
  return luminance;
}

struct Light
{
  vec4 direction;        // xyz - direction, w - cutoff
  vec4 color;
  vec4 position;        // xyz - position, w - attenuation
};

#endif