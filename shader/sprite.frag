#version 450
#extension GL_ARB_bindless_texture : enable

layout(location = 0) in vec2 in_texCoord;
layout(location = 1) in flat uvec2 in_textureHandle;

layout(location = 0, index = 0) out vec4 out_color;

void main()
{
	sampler2D tex = sampler2D(in_textureHandle);
	vec4 color = texture(tex, in_texCoord);
	if(color.a < 0.05) discard;
	out_color = color;
}