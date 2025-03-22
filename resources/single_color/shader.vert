#version 450

layout (location = 0) in vec2 a_in_position;

void main()
{
    gl_Position = vec4(a_in_position, 0.0, 1.0);
}
