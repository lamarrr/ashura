/// SPDX-License-Identifier: MIT

#pragma once

/// see: https://www.shadertoy.com/view/DlVcW1
float exponential_smoothmin(float a, float b, float k)
{
  k *= 1.0;
  float r = exp2(-a / k) + exp2(-b / k);
  return -k * log2(r);
}

float root_smoothmin(float a, float b, float k)
{
  k *= 2.0;
  float x = b - a;
  return 0.5 * (a + b - sqrt(x * x + k * k));
}

float sigmoid_smoothmin(float a, float b, float k)
{
  k *= log(2.0);
  float x = b - a;
  return a + x / (1.0 - exp2(x / k));
}

float quadratic_smoothmin(float a, float b, float k)
{
  k *= 4.0;
  float h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * k * (1.0 / 4.0);
}

float cubic_smoothmin(float a, float b, float k)
{
  k *= 6.0;
  float h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * h * k * (1.0 / 6.0);
}

float quartic_smoothmin(float a, float b, float k)
{
  k *= 16.0 / 3.0;
  float h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * h * (4.0 - h) * k * (1.0 / 16.0);
}

float circular_smoothmin(float a, float b, float k)
{
  k *= 1.0 / (1.0 - sqrt(0.5));
  float h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - k * 0.5 * (1.0 + h - sqrt(1.0 - h * (h - 2.0)));
}

float circular_geometrical_smoothmin(float a, float b, float k)
{
  k *= 1.0 / (1.0 - sqrt(0.5));
  return max(k, min(a, b)) - length(max(k - float2(a, b), 0.0));
}

// https://www.shadertoy.com/view/fsdyzB
// https://iquilezles.org/articles/distfunctions/
// https://iquilezles.org/articles/distfunctions2d/
// length(...+ corner_radii) - corner_radii -> gives the rounding of the
// corners
float rrect_sdf(float2 pos, float2 half_extent, float corner_radii)
{
  float2 q = abs(pos) - half_extent + corner_radii;
  return min(max(q.x, q.y), 0) + length(max(q, 0)) - corner_radii;
}
