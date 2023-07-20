#version 450

layout(location = 0) in vec2 in_uv;

layout(push_constant) uniform GlyphProps {
    mat3 transform;
    float thickness; // thickness will range from 0 to 1
    float outline_thickness;
    vec4 color;
    vec4 outline_color;
    int add_soft_edges;
    int add_outline;
    float outline_min_0;
    float outline_min_1;
    float outline_max_0;
    float outline_max_1;
    float soft_edge_min;
    float soft_edge_max;
} props;

layout(set = 0, binding = 0) uniform sampler2D sdf_texture; /// VK_FORMAT_R8_UNORM

layout(location = 0) out vec4 out_color;

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

void main() {
    vec4 base_color = props.color;
    const float signed_distance = texture(sdf_texture, in_uv).r;

    if(props.add_outline != 0 && (signed_distance >= props.outline_max_0) && (signed_distance <= props.outline_max_1)) {
        float o_factor = 1.0;

        if(signed_distance <= props.outline_min_1) {
            o_factor = smoothstep(props.outline_min_0, props.outline_min_1, signed_distance);
        } else {
            o_factor = smoothstep(props.outline_max_1, props.outline_max_0, signed_distance);
        }

        base_color = mix(base_color, props.outline_color, o_factor);
    }

    if(props.add_soft_edges != 0) {
        base_color.a *= smoothstep(props.soft_edge_min, props.soft_edge_max, signed_distance);
    } else {
        base_color.a = signed_distance >= 0.5 ? 1 : 0;
    }

    out_color = base_color;
}