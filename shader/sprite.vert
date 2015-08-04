#version 450

layout(location = 0) in vec4 in_texCoords;
layout(location = 1) in uvec2 in_textureHandle;
layout(location = 2) in vec3 in_position;
layout(location = 3) in float in_rotation;
layout(location = 4) in vec2 in_scale;

layout(location = 0) out vec4 out_texCoords;
layout(location = 1) out uvec2 out_textureHandle;
layout(location = 2) out vec3 out_position;
layout(location = 3) out float out_rotation;
layout(location = 4) out vec2 out_scale;

void main()
{
	out_texCoords = in_texCoords;
	out_textureHandle = in_textureHandle;
	out_position = in_position;
	out_rotation = in_rotation;
	out_scale = in_scale;
}