#version 450

layout (location = 0) in vec3 a_in_position;

layout (location = 0) out vec3 v_pos;

void main()
{
    v_pos = a_in_position;
    gl_Position = vec4(a_in_position.xyz, 1.0);
}
