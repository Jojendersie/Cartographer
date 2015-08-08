#version 450

layout(location = 0) in vec4 in_texCoords;
layout(location = 1) in uvec2 in_textureHandle;
layout(location = 2) in uvec2 in_numTiles;
layout(location = 3) in vec3 in_position;
layout(location = 4) in float in_rotation;
layout(location = 5) in vec4 in_scale;
layout(location = 6) in vec2 in_anim;

layout(location = 0) out vec4 out_texCoords;
layout(location = 1) out uvec2 out_textureHandle;
layout(location = 2) out vec2 out_numTiles;
layout(location = 3) out vec3 out_position;
layout(location = 4) out float out_rotation;
layout(location = 5) out vec4 out_scale;
layout(location = 6) out vec2 out_anim;

void main()
{
	out_texCoords = in_texCoords;
	out_textureHandle = in_textureHandle;
	out_numTiles = vec2(in_numTiles);
	out_position = in_position;
	out_rotation = in_rotation;
	out_scale = in_scale;
	out_anim = in_anim;
}