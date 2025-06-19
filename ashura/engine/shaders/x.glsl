// Rounded rectangle SDF
float sdSquircle(vec2 pos, vec2 extent, float radius, float degree)
{
  vec2 p = abs(pos / (extent - radius));
  vec2 s = sign(p);
  float   m = pow(p.x, degree) + pow(p.y, degree);
  return pow(m, 1.0 / degree) - 1.0;
}

float squircle(vec2 pos,  vec2 half_extent,  float radius,float degree)
{
  vec2 norm_pos = pos / half_extent;
  float q = pow(abs(norm_pos.x),degree) + pow(abs(norm_pos.y),degree);
  return pow(q,1.0/degree) * radius - radius;
}


float sdRRect(vec2 p, vec2 b, float r) {
    vec2 d = abs(p) - b + vec2(r);
    return length(max(d, 0.0)) - r;
}

// AA helper using screen-space derivatives
float aaSmooth(float d) {
    float w = fwidth(d); // pixel width in SDF space
    return smoothstep( - w,   w, d);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord.xy / iResolution.xy) * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;

    // Position two rectangles side by side
    vec2 posLeft  = uv + vec2(0.55, 0.0);
    vec2 posRight = uv - vec2(0.55, 0.0);

    // RRect params
    vec2 halfSize = vec2(0.3, 0.2); // half-width/height
    float radius  = 0.1 + 0.1;

    // Compute distances
    float dLeft  = sdRRect(posLeft, halfSize, radius);
    float dRight = sdRRect(posRight, halfSize, radius);

    // Raw thresholding (no AA)
    float maskLeft  = 1.0- aaSmooth(dRight*2.0);
    
    
    vec2 halfSize2 = vec2(0.2, 0.1); // half-width/height
    float radius2  = 4.04;

    // Compute distances
    float dLeft2  = sdRRect(posLeft, halfSize2, radius2);
    float dRight2 = sdRRect(posRight, halfSize2, radius2);
     dRight2 =   squircle(posRight, halfSize2,  radius2, 18.0);

    // Raw thresholding (no AA)
    float maskLeft2  =   aaSmooth(dRight2*2.0);
    
    maskLeft *= maskLeft2;
 

    // Colors 
    vec3 colorLeft = vec3(0.4, 1.0, 0.6);  // AA

    vec3 bg = vec3(0.08);
    vec3 col = bg;

    col = vec3(maskLeft ); 

    fragColor = vec4(col, 1.0);
}