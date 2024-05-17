
inline stx::Span<Vertex2d> rect(Vec2 offset, Vec2 extent, Vec4 color,
                                stx::Span<Vertex2d> polygon)
{
  Vertex2d const vertices[] = {
      {.position = offset, .uv = {}, .color = color},
      {.position = offset + Vec2{extent.x, 0}, .uv = {}, .color = color},
      {.position = offset + extent, .uv = {}, .color = color},
      {.position = offset + Vec2{0, extent.y}, .uv = {}, .color = color}};

  return polygon.copy(vertices);
}

inline stx::Span<Vertex2d> arc(Vec2 offset, f32 radius, f32 begin, f32 end,
                               u32 nsegments, Vec4 color,
                               stx::Span<Vertex2d> polygon)
{
  begin = to_radians(begin);
  end   = to_radians(end);

  if (nsegments < 1 || radius <= 0)
  {
    return {};
  }

  for (u32 i = 0; i < nsegments; i++)
  {
    f32  angle = lerp(begin, end, static_cast<f32>(i / (nsegments - 1)));
    Vec2 p     = radius + radius * Vec2{std::cos(angle), std::sin(angle)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  return polygon;
}

inline stx::Span<Vertex2d> circle(Vec2 offset, f32 radius, u32 nsegments,
                                  Vec4 color, stx::Span<Vertex2d> polygon)
{
  if (nsegments == 0 || radius <= 0)
  {
    return {};
  }

  f32 step = (2 * PI) / nsegments;

  for (u32 i = 0; i < nsegments; i++)
  {
    Vec2 p     = radius + radius * Vec2{std::cos(i * step), std::sin(i * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  return polygon;
}

inline stx::Span<Vertex2d> ellipse(Vec2 offset, Vec2 radii, u32 nsegments,
                                   Vec4 color, stx::Span<Vertex2d> polygon)
{
  if (nsegments == 0 || radii.x <= 0 || radii.y <= 0)
  {
    return {};
  }

  f32 step = (2 * PI) / nsegments;

  for (u32 i = 0; i < nsegments; i++)
  {
    Vec2 p     = radii + radii * Vec2{std::cos(i * step), std::sin(i * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  return polygon;
}

// outputs 8 + nsegments * 4 vertices
inline stx::Span<Vertex2d> round_rect(Vec2 offset, Vec2 extent, Vec4 radii,
                                      u32 nsegments, Vec4 color,
                                      stx::Span<Vertex2d> polygon)
{
  f32 max_radius   = min(extent.x, extent.y);
  radii.x          = min(radii.x, max_radius);
  radii.y          = min(radii.y, max_radius - radii.x);
  f32 max_radius_z = min(max_radius - radii.x, max_radius - radii.y);
  radii.z          = min(radii.z, max_radius_z);
  f32 max_radius_w = min(max_radius_z, max_radius - radii.z);
  radii.w          = min(radii.w, max_radius_w);

  f32 step = nsegments == 0 ? 0.0f : (PI / 2) / nsegments;

  u32 i = 0;

  polygon[i] = Vertex2d{
      .position = offset + extent - Vec2{0, radii.z}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    Vec2 p = (extent - radii.z) +
             radii.z * Vec2{std::cos(segment * step), std::sin(segment * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = Vertex2d{
      .position = offset + extent - Vec2{radii.z, 0}, .uv = {}, .color = color};
  i++;

  polygon[i] = Vertex2d{
      .position = offset + Vec2{radii.w, extent.y}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    Vec2 p = Vec2{radii.w, extent.y - radii.w} +
             radii.w * Vec2{std::cos(PI / 2 + segment * step),
                            std::sin(PI / 2 + segment * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = Vertex2d{.position = offset + Vec2{0, extent.y - radii.w},
                        .uv       = {},
                        .color    = color};
  i++;

  polygon[i] =
      Vertex2d{.position = offset + Vec2{0, radii.x}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    Vec2 p     = radii.x + radii.x * Vec2{std::cos(PI + segment * step),
                                      std::sin(PI + segment * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] =
      Vertex2d{.position = offset + Vec2{radii.x, 0}, .uv = {}, .color = color};
  i++;

  polygon[i] = Vertex2d{.position = offset + Vec2{extent.x - radii.y, 0},
                        .uv       = {},
                        .color    = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    Vec2 p = Vec2{extent.x - radii.y, radii.y} +
             radii.y * Vec2{std::cos(PI * 3.0f / 2.0f + segment * step),
                            std::sin(PI * 3.0f / 2.0f + segment * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = Vertex2d{
      .position = offset + Vec2{extent.x, radii.y}, .uv = {}, .color = color};

  return polygon;
}

inline stx::Span<Vertex2d> bevel_rect(Vec2 offset, Vec2 extent, Vec4 radii,
                                      Vec4 color, stx::Span<Vertex2d> polygon)
{
  f32 max_radius   = min(extent.x, extent.y);
  radii.x          = min(radii.x, max_radius);
  radii.y          = min(radii.y, max_radius - radii.x);
  f32 max_radius_z = min(max_radius - radii.x, max_radius - radii.y);
  radii.z          = min(radii.z, max_radius_z);
  f32 max_radius_w = min(max_radius_z, max_radius - radii.z);
  radii.w          = min(radii.w, max_radius_w);

  Vertex2d const vertices[] = {
      {.position = offset + Vec2{radii.x, 0}, .uv = {}, .color = color},
      {.position = offset + Vec2{extent.x - radii.y, 0},
       .uv       = {},
       .color    = color},
      {.position = offset + Vec2{extent.x, radii.y}, .uv = {}, .color = color},
      {.position = offset + Vec2{extent.x, extent.y - radii.z},
       .uv       = {},
       .color    = color},
      {.position = offset + Vec2{extent.x - radii.z, extent.y},
       .uv       = {},
       .color    = color},
      {.position = offset + Vec2{radii.w, extent.y}, .uv = {}, .color = color},
      {.position = offset + Vec2{0, extent.y - radii.w},
       .uv       = {},
       .color    = color},
      {.position = offset + Vec2{0, radii.x}, .uv = {}, .color = color}};

  return polygon.copy(vertices);
}

inline stx::Span<Vertex2d> lerp_uvs(stx::Span<Vertex2d> path, Vec2 extent,
                                    Vec2 uv0, Vec2 uv1)
{
  for (Vertex2d &v : path)
  {
    Vec2 t =
        v.position / Vec2{epsilon_clamp(extent.x), epsilon_clamp(extent.y)};
    v.uv.x = lerp(uv0.x, uv1.x, t.x);
    v.uv.y = lerp(uv0.y, uv1.y, t.y);
  }

  return path;
}

inline stx::Span<Vertex2d> lerp_color_gradient(stx::Span<Vertex2d> path,
                                               Vec2                extent,
                                               LinearColorGradient gradient)
{
  if (gradient.is_uniform())
  {
    return path;
  }

  f32 const x = std::cos(to_radians(gradient.angle));
  f32 const y = std::sin(to_radians(gradient.angle));

  for (Vertex2d &v : path)
  {
    Vec2 const p = v.position / extent;
    f32 const  t = p.x * x + p.y * y;
    v.color      = lerp(gradient.begin, gradient.end, t);
  }

  return path;
}
