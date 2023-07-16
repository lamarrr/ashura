#version 450

layout(location = 0) in vec2 in_uv;

layout(push_constant) uniform GlyphProps {
    mat3 transform;
    float thickness; // thickness will range from 0 to 1
    float outline_thickness;
    vec4 color;
    vec4 outline_color;
} props;

layout(set = 0, binding = 0) uniform sampler2D sdf_texture; /// VK_FORMAT_R8_UNORM

layout(location = 0) out vec4 out_color;

void main() {
    float signed_distance = texture(sdf_texture, in_uv).r;
    float alpha = smoothstep(1 - props.thickness, 1 - props.thickness, signed_distance);
    float outline = smoothstep(props.outline_thickness, props.outline_thickness, signed_distance);
    out_color = vec4(mix(props.outline_color.rgb, props.color.rgb, outline), alpha);
}

// ...
// const float outlineDistance; // Between 0 and 0.5, 0 = thick outline, 0.5 = no outline
// const vec4 outlineColor;
// ...
// void main() {
//     float distance = texture2D(u_texture, v_texCoord).a;
//     float outlineFactor = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
//     vec4 color = mix(outlineColor, v_color, outlineFactor);
//     float alpha = smoothstep(outlineDistance - smoothing, outlineDistance + smoothing, distance);
//     gl_FragColor = vec4(color.rgb, color.a * alpha);
// }
// Adding a drop shadowPermalink
// Here, we sample the texture a second time, slightly offset from the first. The second application gets a lot more smoothing applied to it, and is rendered “behind” the actual text.

// ...
// const vec2 shadowOffset; // Between 0 and spread / textureSize
// const float shadowSmoothing; // Between 0 and 0.5
// const vec4 shadowColor;
// ...
// void main() {
//     float distance = texture2D(u_texture, v_texCoord).a;
//     float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
//     vec4 text = vec4(v_color.rgb, v_color.a * alpha);

//     float shadowDistance = texture2D(u_texture, v_texCoord - shadowOffset).a;
//     float shadowAlpha = smoothstep(0.5 - shadowSmoothing, 0.5 + shadowSmoothing, shadowDistance);
//     vec4 shadow = vec4(shadowColor.rgb, shadowColor.a * shadowAlpha);

//     gl_FragColor = mix(shadow, text, text.a);
// }