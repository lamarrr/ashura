#version 450

layout(location = 0) in vec2 in_uv;

#define ADD_SOFT_EDGES_BIT 1
#define ADD_OUTLINE_BIT 2

layout(push_constant) uniform GlyphProps {
    mat3 transform;
    vec4 stroke_color;
    vec4 outline_color;
    float outline_min_0;
    float outline_min_1;
    float outline_max_0;
    float outline_max_1;
    float soft_edge_min;
    float soft_edge_max;
    uint flags;
} props;

/// texture should be in R8 format
layout(set = 0, binding = 0) uniform sampler2D sdf_texture;

layout(location = 0) out vec4 out_color;

void main() {
    vec4 color = props.stroke_color;
    const float signed_distance = texture(sdf_texture, in_uv).r;

    // if we are within outline region
    if((props.flags & ADD_OUTLINE_BIT) == ADD_OUTLINE_BIT && (signed_distance >= props.outline_max_0) && (signed_distance <= props.outline_max_1)) {
        float outline_factor = 1.0;

        if(signed_distance <= props.outline_min_1) {
            outline_factor = smoothstep(props.outline_min_0, props.outline_min_1, signed_distance);
        } else {
            outline_factor = smoothstep(props.outline_max_1, props.outline_max_0, signed_distance);
        }

        color = mix(color, props.outline_color, outline_factor);
    }

    if((props.flags & ADD_SOFT_EDGES_BIT) == ADD_SOFT_EDGES_BIT) {
        color.a *= smoothstep(props.soft_edge_min, props.soft_edge_max, signed_distance);
    } else {
        color.a = signed_distance >= 0.5 ? 1 : 0;
    }

    out_color = color;
}