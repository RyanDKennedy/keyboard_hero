#version 450

struct InstanceInfo
{
    vec2 pos_offset;
    uvec2 tex_bottom_left;
    uvec2 tex_top_right;
};

layout (location = 0) in vec2 a_position;

layout (location = 1) out vec2 v_tex_coord;

layout (set = 2, binding = 0) readonly buffer Buffer
{
    InstanceInfo infos[];
};

void main()
{
    vec2 tex_coord;

    if (a_position.x < 0)
    {
	tex_coord.x = infos[gl_InstanceIndex].tex_bottom_left.x;
    }    
    else
    {
	tex_coord.x = infos[gl_InstanceIndex].tex_top_right.x;
    }

    if (a_position.y < 0)
    {
	tex_coord.y = infos[gl_InstanceIndex].tex_bottom_left.y;
    }
    else
    {
	tex_coord.y = infos[gl_InstanceIndex].tex_top_right.y;
    }

    v_tex_coord = tex_coord;
    vec2 pos = a_position + infos[gl_InstanceIndex].pos_offset;

    gl_Position = vec4(pos.xy, 0.0f, 1.0f);
}
