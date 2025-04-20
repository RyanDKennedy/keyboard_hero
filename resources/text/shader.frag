#version 450

layout (location = 0) out vec4 out_color;

layout (location = 0) in vec2 v_pos;
layout (location = 1) in vec2 v_tex_coord;

layout (set = 0, binding = 0) uniform sampler2D u_character_map;
layout (set = 1, binding = 0) uniform CharacterInformation
{
    vec3 color;
} u_character_information;

void main()
{
    vec4 texture_color = textureLod(u_character_map, v_tex_coord, 0);

    if (texture_color.r < 0.5f)
	discard;

    out_color = vec4(u_character_information.color.rgb, 1.0f);
}
