#version 450

layout(location = 0) in vec4 in_texCoords;
layout(location = 1) in vec2 in_size;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec3 in_position;
layout(location = 4) in float in_rotation;

layout(location = 0) out vec4 out_texCoords;
layout(location = 1) out vec2 out_size;
layout(location = 2) out vec4 out_color;
layout(location = 3) out vec3 out_position;
layout(location = 4) out float out_rotation;

void main()
{
	out_texCoords = in_texCoords;
	out_size = in_size;
	out_color = in_color;
	out_position = in_position;
	out_rotation = in_rotation;
}