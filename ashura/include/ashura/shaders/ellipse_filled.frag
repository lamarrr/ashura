
// TODO(): account for thickness

float circle(in vec2 coord, in vec2 center, in float radius){
vec2 displacement = coord - center;
float dist = length(displacement);
return 1.0f - step(radius, dist);
}

void mainImage(out vec4 frag_color, in vec2 frag_coord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = frag_coord/iResolution.xy;

    // Time varying pixel color
    vec4 col = vec4 (1.0f, 0.0f, 0.0f, 1.0f);

    vec2 center = vec2(0.5f, 0.5f);
    float radius = 0.5f;

    // Output to screen
    frag_color = vec4(col.xyzw * circle(uv, center, radius));
}