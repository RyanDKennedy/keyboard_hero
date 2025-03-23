#version 450

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform FrameDataUniform {
    vec3 color;
} u_frame_data;

layout (set = 1, binding = 1) uniform Material {
	vec3 diffuse;
	vec3 specular;
	vec3 ambient;
} u_material;

void main()
{
    out_color = vec4(u_material.diffuse.rgb, 1.0);
}
