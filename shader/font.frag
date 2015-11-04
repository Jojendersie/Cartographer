#version 450

layout(location = 0) in vec2 in_texCoord;
layout(location = 1) in flat vec4 in_color;

// A distance field or mask
layout(binding = 0) uniform sampler2D tx_character;

layout(location = 0, index = 0) out vec4 out_color;

void main()
{
	// TODO: test manual lodding
	float alpha = texture(tx_character, in_texCoord, -1.5).x;
	// TODO: calculate alpha
	// Alpha-test (this is correct for distance fields and masks)
	if(alpha < 0.005) discard;
	alpha *= alpha;
	out_color = in_color * alpha;
}