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

vec3 ibl(vec3 n, vec3 v, vec3 diffuseColor, vec3 f0, vec3 f90,
         float perceptualRoughness)
{
  vec3  r    = reflect(n);
  vec3  Ld   = textureCube(irradianceEnvMap, r) * diffuseColor;
  float lod  = computeLODFromRoughness(perceptualRoughness);
  vec3  Lld  = textureCube(prefilteredEnvMap, r, lod);
  vec2  Ldfg = textureLod(dfgLut, vec2(dot(n, v), perceptualRoughness), 0.0).xy;
  vec3  Lr   = (f0 * Ldfg.x + f90 * Ldfg.y) * Lld;
  return Ld + Lr;
}

#endif