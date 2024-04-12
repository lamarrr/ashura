#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"

layout(location = 0) in vec2 i_rel_position;
layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 0) uniform Params
{
  ViewTransform transform;
  vec4          tint[4];
  vec4          border_color[4];
  vec4          border_radii;
  float         border_thickness;
  vec2          uv0;
  vec2          uv1;
}
u_params;

layout(set = 1, binding = 0) uniform sampler2D u_base_color;

void main()
{
  o_color = u_params.tint[0];
}

/*
// from https://iquilezles.org/articles/distfunctions
// additional thanks to iq for optimizing conditional block for individual
// corner radii!
float roundedBoxSDF(vec2 CenterPosition, vec2 Size, vec4 Radius)
{
  Radius.xy = (CenterPosition.x > 0.0) ? Radius.xy : Radius.zw;
  Radius.x  = (CenterPosition.y > 0.0) ? Radius.x : Radius.y;

  vec2 q = abs(CenterPosition) - Size + Radius.x;
  return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - Radius.x;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
  // =========================================================================
  // Inputs (uniforms)

  // The pixel-space scale of the rectangle.
  vec2 u_rectSize = vec2(250.0, 250.0);
  // The pixel-space rectangle center location
  vec2 u_rectCenter = (iResolution.xy / 2.0);

  // How soft the edges should be (in pixels). Higher values
  // could be used to simulate a drop shadow.
  float u_edgeSoftness = 2.0;
  // The radiuses of the corners(in pixels): [topRight,
  // bottomRight, topLeft, bottomLeft]
  vec4 u_cornerRadiuses = vec4(10.0, 20.0, 40.0, 60.0);

  // Border
  // The border size (in pixels)
  // How soft the (internal) border should be (in pixels)
  float u_borderThickness = 5.0;
  float u_borderSoftness  = 2.0;

  // Rectangle extents (half of the size)
  vec2 halfSize = (u_rectSize / 2.0);

  // Animated corners radiuses
  vec4 radius = vec4((sin(iTime) + 1.0)) * u_cornerRadiuses;

  // -------------------------------------------------------------------------

  // Calculate distance to edge.
  float distance = roundedBoxSDF(fragCoord.xy - u_rectCenter, halfSize, radius);

  // Smooth the result (free antialiasing).
  float smoothedAlpha = 1.0 - smoothstep(0.0, 1, distance);

  // -------------------------------------------------------------------------
  // Border.

  float borderAlpha = 1.0 - smoothstep(u_borderThickness - u_borderSoftness,
                                       u_borderThickness, abs(distance));

  // -------------------------------------------------------------------------
  // Apply colors layer-by-layer: background <- shadow <- rect <- border.

  // Blend (background+shadow) with rect
  //   Note:
  //     - Used 'min(u_colorRect.a, smoothedAlpha)' instead of 'smoothedAlpha'
  //       to enable rectangle color transparency
  vec4 res_shadow_with_rect_color =
      mix(res_shadow_color, u_colorRect, min(u_colorRect.a, smoothedAlpha));

  // Blend (background+shadow+rect) with border
  //   Note:
  //     - Used 'min(borderAlpha, smoothedAlpha)' instead of 'borderAlpha'
  //       to make border 'internal'
  //     - Used 'min(u_colorBorder.a, alpha)' instead of 'alpha' to enable
  //       border color transparency
  vec4 res_shadow_with_rect_with_border =
      mix(res_shadow_with_rect_color, u_colorBorder,
          min(u_colorBorder.a, min(borderAlpha, smoothedAlpha)));

  // -------------------------------------------------------------------------

  fragColor = res_shadow_with_rect_with_border;
}
*/