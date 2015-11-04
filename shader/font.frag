#version 450

layout(location = 0) in vec2 in_texCoord;

// A distance field or mask
layout(binding = 0) uniform sampler2D tx_character;

layout(location = 0, index = 0) out vec4 out_color;

void main()
{
	// TODO: test manual lodding
	float color = texture(tx_character, in_texCoord, -1.5).x;
	// TODO: calculate alpha
	// Alpha-test (this is correct for distance fields and masks)
	if(color < 0.005) discard;
	color *= color;
	out_color = vec4(color, color, color, color);
}