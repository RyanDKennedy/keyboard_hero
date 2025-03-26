#version 450

layout (location = 0) out vec4 out_color;

layout (set = 1, binding = 0) uniform Material {
	vec3 diffuse;
	vec3 specular;
	vec3 ambient;
} u_material;

layout (location = 0) in vec3 v_pos;

void main()
{
    vec3 color = u_material.diffuse.rgb * (sqrt(v_pos.x * v_pos.x + v_pos.y * v_pos.y + v_pos.z * v_pos.z)) / (0.5 * sqrt(3));

    out_color = vec4(color.rgb, 1.0);
}
