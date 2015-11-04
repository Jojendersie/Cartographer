#version 450

layout(location = 0) in vec2 in_texCoord;
layout(location = 1) in flat vec4 in_color;

// A distance field or mask
layout(binding = 0) uniform sampler2D tx_character;

layout(location = 0, index = 0) out vec4 out_color;

void main()
{
	// Use the bitmap-font as if it is a distance field.
	// Compute the antialiasing-alpha based on the distance to 0.5.
	// https://www.reddit.com/r/gamedev/comments/2879jd/just_found_out_about_signed_distance_field_text/
	float dist = texture(tx_character, in_texCoord, -2.0).x - 0.5;
	float aa = length( vec2(dFdx(dist), dFdy(dist)) );
	float alpha = smoothstep( -aa, aa, dist );
	// Alpha-test (this is correct for distance fields and masks)
	if(alpha < 0.005) discard;
	out_color = in_color * alpha;
}