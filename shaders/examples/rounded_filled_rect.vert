
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









/**
 * @author jonobr1 / http://jonobr1.com/
 */

vec4 circle(vec2 fragCoord, vec2 center, float radius, vec3 color) {
	float distance = length(fragCoord - center) - radius;
	float t = clamp(distance, 0.0, 1.0);
	return vec4(color, 1.0 - t);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {

	vec2 fragCoordXY = fragCoord.xy;
	
    vec2 center = iResolution.xy * 0.5;
    
	float radius = 0.25 * iResolution.y;

	vec4 background = vec4(0.0, 0.0, 0.0, 1.0);
	 
	vec3 color = vec3(1.0, 0.0, 0.0);
    
	vec4 x = circle(fragCoordXY, center, radius, color);
	
	// Blend the two
	fragColor = mix(background, x, x.a);

}