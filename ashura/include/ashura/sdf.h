#pragma once

#include <algorithm>
#include <numeric>

#include "ashura/primitives.h"

namespace ash
{

/// Generates 8-bit Signed Distance Field from a 1-bit Alpha Image.
///
/// output_width = width + sdf_spread * 2
/// output_height = height + sdf_spread * 2
///
///
inline void generate_sdf_from_mono(u8 const *const src, u32 const src_pitch, u32 const width, u32 const height, u32 const sdf_spread, u8 *const output, u32 const output_pitch)
{
  for (i64 i = 0; i < height + sdf_spread * 2; i++)
  {
    for (i64 j = 0; j < width + sdf_spread * 2; j++)
    {
      i64 const isrc = i - sdf_spread;
      i64 const jsrc = j - sdf_spread;

      u8 is_inside = 0;

      if (isrc >= 0 && isrc < height && jsrc >= 0 && jsrc < width) [[likely]]
      {
        is_inside = (src[isrc * src_pitch + (jsrc / 8)] >> (7 - (jsrc % 8))) & 1;
      }

      // the squared distance to the nearest neigbor that has a different position along the shape
      i64 square_distance = sdf_spread * sdf_spread;

      for (i64 ifield = std::max(isrc - sdf_spread, (i64) 0); ifield < std::min(isrc + sdf_spread + 1, (i64) height); ifield++)
      {
        for (i64 jfield = std::max(jsrc - sdf_spread, (i64) 0); jfield < std::min(jsrc + sdf_spread + 1, (i64) width); jfield++)
        {
          u8 const neighbor_is_inside = (src[ifield * src_pitch + (jfield / 8)] >> (7 - (jfield % 8))) & 1;
          if (neighbor_is_inside != is_inside) [[likely]]
          {
            i64 const neighbor_square_distance = (ifield - isrc) * (ifield - isrc) + (jfield - jsrc) * (jfield - jsrc);
            square_distance                    = std::min(square_distance, neighbor_square_distance);
          }
        }
      }

      i64 const distance        = (u8) (127 * std::sqrt((f32) square_distance) / sdf_spread);
      i64 const signed_distance = 127 + (is_inside ? distance : -distance);

      output[i * output_pitch + j] = (u8) signed_distance;
    }
  }
}

}        // namespace ash
