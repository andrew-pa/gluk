#version 410

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 itexc;

uniform mat4 world;
uniform mat4 view_proj;

out vec3 pos_w;
out vec3 norm_w;
out vec2 texc;

void main()
{
	mat4 wvp = view_proj*world;
	pos_w = (world*vec4(pos,1)).xyz;
	norm_w = (world*vec4(norm,0)).xyz;
	texc = itexc;
	gl_Position = wvp*vec4(pos, 1);
}
