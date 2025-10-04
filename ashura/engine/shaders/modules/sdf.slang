/// SPDX-License-Identifier: MIT

#pragma once

#include "core.slang"
#include "modules/types.slang"

namespace sdf
{

f32 xor_op(f32 a, f32 b)
{
  return max(min(a, b), -max(a, b));
}

f32 round(f32 sd, f32 r)
{
  return sd - r;
}

f32 onion(f32 sd, f32 r)
{
  return abs(sd) - r;
}

f32 union(f32 d1, f32 d2)
{
  return min(d1, d2);
}

f32 sub(f32 d1, f32 d2)
{
  return max(-d1, d2);
}

f32 intersection(f32 d1, f32 d2)
{
  return max(d1, d2);
}

f32 smooth_union(f32 d1, f32 d2, f32 k)
{
  f32 h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
  return lerp(d2, d1, h) - k * h * (1.0 - h);
}

f32 smooth_sub(f32 d1, f32 d2, f32 k)
{
  f32 h = clamp(0.5 - 0.5 * (d2 + d1) / k, 0.0, 1.0);
  return lerp(d2, -d1, h) + k * h * (1.0 - h);
}

f32 smooth_intersection(f32 d1, f32 d2, f32 k)
{
  f32 h = clamp(0.5 - 0.5 * (d2 - d1) / k, 0.0, 1.0);
  return lerp(d2, d1, h) + k * h * (1.0 - h);
}

/// see: https://www.shadertoy.com/view/DlVcW1
f32 exp_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 1.0;
  f32 r = exp2(-a / k) + exp2(-b / k);
  return -k * log2(r);
}

f32 root_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 2.0;
  f32 x = b - a;
  return 0.5 * (a + b - sqrt(x * x + k * k));
}

f32 sigmoid_smoothmin(f32 a, f32 b, f32 k)
{
  k *= log(2.0);
  f32 x = b - a;
  return a + x / (1.0 - exp2(x / k));
}

f32 quad_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 4.0;
  f32 h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * k * (1.0 / 4.0);
}

f32 cubic_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 6.0;
  f32 h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * h * k * (1.0 / 6.0);
}

f32 quart_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 16.0 / 3.0;
  f32 h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * h * (4.0 - h) * k * (1.0 / 16.0);
}

f32 circ_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 1.0 / (1.0 - sqrt(0.5));
  f32 h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - k * 0.5 * (1.0 + h - sqrt(1.0 - h * (h - 2.0)));
}

f32 circ_geom_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 1.0 / (1.0 - sqrt(0.5));
  return max(k, min(a, b)) - length(max(k - f32x2(a, b), 0.0));
}

// see: https://www.shadertoy.com/view/MXfXzM
f32x2 quad_smoothmin_lerp(f32 a, f32 b, f32 k)
{
  f32 h = 1.0 - min(abs(a - b) / (4.0 * k), 1.0);
  f32 w = h * h;
  f32 m = w * 0.5;
  f32 s = w * k;
  return (a < b) ? f32x2(a - s, m) : f32x2(b - s, 1.0 - m);
}

f32x2 cubic_smoothmin_lerp(f32 a, f32 b, f32 k)
{
  f32 h = 1.0 - min(abs(a - b) / (6.0 * k), 1.0);
  f32 w = h * h * h;
  f32 m = w * 0.5;
  f32 s = w * k;
  return (a < b) ? f32x2(a - s, m) : f32x2(b - s, 1.0 - m);
}

f32 circle(f32x2 pos, f32 radius)
{
  return length(pos) - radius;
}

f32 box(f32x2 pos, f32x2 extent)
{
  f32x2 dist = abs(pos) - extent;
  return length(max(dist, 0.0)) + min(max(dist.x, dist.y), 0.0);
}

f32 arc(f32x2 pos, f32x2 sc, f32 ra, f32 rb)
{
  // sc is the sin/cos of the arc's aperture
  pos.x = abs(pos.x);
  return ((sc.y * pos.x > sc.x * pos.y) ? length(pos - sc * ra) :
                                          abs(length(pos) - ra)) -
         rb;
}

// https://www.shadertoy.com/view/fsdyzB
// https://iquilezles.org/articles/distfunctions/
// https://iquilezles.org/articles/distfunctions2d/
// length(...+ corner_radii) - corner_radii -> gives the rounding of the
// corners
f32 rrect(f32x2 pos, f32x2 half_extent, f32x4 radii)
{
  radii.xy = (pos.x > 0.0) ? radii.xy : radii.zw;
  radii.x  = (pos.y > 0.0) ? radii.x : radii.y;
  f32x2 q  = abs(pos) - half_extent + radii.x;
  return min(max(q.x, q.y), 0) + length(max(q, 0)) - radii.x;
}

/// @brief Gets the approximate SDF of a squircle
/// @returns the signed distance (x-component) and the gradient (yz-components).
/// @see https://www.shadertoy.com/view/fsccz4
/// @param pos normalized position
/// @param radius normalized radius
f32 squircle(f32x2 pos, f32x2 half_extent, f32 degree, f32 radius)
{
  f32x2 norm_pos = pos / half_extent;
  f32   q        = pow(abs(norm_pos.x), degree) + pow(abs(norm_pos.y), degree);
  return pow(q, 1.0 / degree) * radius - radius;
}

/// @brief Triangle texture coords: [0, 0], [0.5, 0.25], [1, 1]
///
/// @param texture_coord texture coordinates; sensitive to screen-space distortions
///
f32 parabola(f32x2 texture_coord)
{
  f32x2 pos = texture_coord;
  return (pos.x * pos.x - pos.y) / length(f32x2(2.0 * pos.x, -1.0));
}

f32 triangle(f32x2 p, f32x2 p0, f32x2 p1, f32x2 p2)
{
  f32x2 e0 = p1 - p0, e1 = p2 - p1, e2 = p0 - p2;
  f32x2 v0 = p - p0, v1 = p - p1, v2 = p - p2;
  f32x2 pq0 = v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0.0, 1.0);
  f32x2 pq1 = v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0.0, 1.0);
  f32x2 pq2 = v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0.0, 1.0);
  f32   s   = sign(e0.x * e2.y - e0.y * e2.x);
  f32x2 d   = min(min(f32x2(dot(pq0, pq0), s * (v0.x * e0.y - v0.y * e0.x)),
                      f32x2(dot(pq1, pq1), s * (v1.x * e1.y - v1.y * e1.x))),
                  f32x2(dot(pq2, pq2), s * (v2.x * e2.y - v2.y * e2.x)));
  return -sqrt(d.x) * sign(d.y);
}

f32 segment(f32x2 p, f32x2 a, f32x2 b)
{
  f32x2 pa = p - a, ba = b - a;
  f32   h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
  return length(pa - ba * h);
}

f32 sphere(f32x3 p, f32 s)
{
  return length(p) - s;
}

f32 box(f32x3 p, f32x3 b)
{
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

f32 round_box(f32x3 p, f32x3 b, f32 r)
{
  f32x3 q = abs(p) - b + r;
  return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}

f32 box_frame(f32x3 p, f32x3 b, f32 e)
{
  p       = abs(p) - b;
  f32x3 q = abs(p + e) - e;
  return min(min(length(max(f32x3(p.x, q.y, q.z), 0.0)) +
                   min(max(p.x, max(q.y, q.z)), 0.0),
                 length(max(f32x3(q.x, p.y, q.z), 0.0)) +
                   min(max(q.x, max(p.y, q.z)), 0.0)),
             length(max(f32x3(q.x, q.y, p.z), 0.0)) +
               min(max(q.x, max(q.y, p.z)), 0.0));
}

f32 torus(f32x3 p, f32x2 t)
{
  f32x2 q = f32x2(length(p.xz) - t.x, p.y);
  return length(q) - t.y;
}

f32 octahedron(f32x3 p, f32 s)
{
  p = abs(p);
  return (p.x + p.y + p.z - s) * 0.57735027;
}

f32 pyramid(f32x3 p, f32 h)
{
  f32 m2 = h * h + 0.25;

  p.xz = abs(p.xz);
  p.xz = (p.z > p.x) ? p.zx : p.xz;
  p.xz -= 0.5;

  f32x3 q = f32x3(p.z, h * p.y - 0.5 * p.x, h * p.x + 0.5 * p.y);

  f32 s = max(-q.x, 0.0);
  f32 t = clamp((q.y - 0.5 * p.z) / (m2 + 0.25), 0.0, 1.0);

  f32 a = m2 * (q.x + s) * (q.x + s) + q.y * q.y;
  f32 b =
    m2 * (q.x + 0.5 * t) * (q.x + 0.5 * t) + (q.y - m2 * t) * (q.y - m2 * t);

  f32 d2 = min(q.y, -q.x * m2 - q.y * 0.5) > 0.0 ? 0.0 : min(a, b);

  return sqrt((d2 + q.z * q.z) / m2) * sign(max(q.z, -p.y));
}

f32 quad(f32x3 p, f32x3 a, f32x3 b, f32x3 c, f32x3 d)
{
  f32x3 ba  = b - a;
  f32x3 pa  = p - a;
  f32x3 cb  = c - b;
  f32x3 pb  = p - b;
  f32x3 dc  = d - c;
  f32x3 pc  = p - c;
  f32x3 ad  = a - d;
  f32x3 pd  = p - d;
  f32x3 nor = cross(ba, ad);

  return sqrt(
    (sign(dot(cross(ba, nor), pa)) + sign(dot(cross(cb, nor), pb)) +
       sign(dot(cross(dc, nor), pc)) + sign(dot(cross(ad, nor), pd)) <
     3.0) ?
      min(min(min(dot2(ba * clamp(dot(ba, pa) / dot2(ba), 0.0, 1.0) - pa),
                  dot2(cb * clamp(dot(cb, pb) / dot2(cb), 0.0, 1.0) - pb)),
              dot2(dc * clamp(dot(dc, pc) / dot2(dc), 0.0, 1.0) - pc)),
          dot2(ad * clamp(dot(ad, pd) / dot2(ad), 0.0, 1.0) - pd)) :
      dot(nor, pa) * dot(nor, pa) / dot2(nor));
}

f32 antialiased_mask(f32 edge_signed_dist)
{
  f32 edge_mask = 1 - antialias_mask(edge_signed_dist, 1);
  return edge_mask;
}

f32 ring_signed_distance(f32 edge_signed_dist, f32 feather)
{
  return onion(edge_signed_dist, feather);
}

}    // namespace sdf

namespace msdf
{

f32 screen_px_range(f32 msd_px_range, f32x2 texture_size, f32x2 tex_coord)
{
  f32x2 unit_range      = f32x2(msd_px_range) / texture_size;
  f32x2 screen_tex_size = f32x2(1.0) / fwidth(tex_coord);
  return max(0.5 * dot(unit_range, screen_tex_size), 1.0);
}

f32 median(f32 r, f32 g, f32 b)
{
  return max(min(r, g), min(max(r, g), b));
}

// https://github.com/Chlumsky/msdfgen/tree/master?tab=readme-ov-file#using-a-multi-channel-distance-field
f32 msdf(f32x3 msd, f32 msd_px_range, f32x2 texture_size, f32x2 tex_coord)
{
  f32 sd = median(msd.r, msd.g, msd.b);
  f32 screen_px_distance =
    screen_px_range(msd_px_range, texture_size, tex_coord) * (sd - 0.5);
  return clamp(screen_px_distance + 0.5, 0.0, 1.0);
}

f32x4 blend(f32x3 msd, f32 msd_px_range, f32x2 texture_size, f32x2 tex_coord,
            f32x4 bg_color, f32x4 fg_color)
{
  f32 alpha = msdf(msd, msd_px_range, texture_size, tex_coord);
  return lerp(bg_color, fg_color, alpha);
}

}    // namespace msdf
