#version 450
#extension GL_ARB_bindless_texture : enable

layout(location = 0) in vec2 in_texCoord;
layout(location = 1) in flat uvec2 in_textureHandle;
layout(location = 2) in flat vec4 in_anim;
layout(location = 3) in flat vec2 in_numTiles;

layout(location = 0, index = 0) out vec4 out_color;

vec2 tileOffset(vec2 _first, float _x, float _y)
{
	return mod(_first + vec2(_x, _y), in_numTiles) * in_anim.zw;
}

void main()
{
	sampler2D tex = sampler2D(in_textureHandle);
	vec4 color;
	if(in_anim.x == 0.0 && in_anim.y == 0.0)
	{
		color = texture(tex, in_texCoord);
	} else {
		vec2 firstTile = floor(in_anim.xy);
		vec2 tileInterpolation = in_anim.xy - firstTile;
		color  = texture(tex, in_texCoord + firstTile * in_anim.zw) * (1 - tileInterpolation.x) * (1 - tileInterpolation.y);
		color += texture(tex, in_texCoord + tileOffset(firstTile, 1.0, 0.0)) * (    tileInterpolation.x) * (1 - tileInterpolation.y);
		color += texture(tex, in_texCoord + tileOffset(firstTile, 0.0, 1.0)) * (1 - tileInterpolation.x) * (    tileInterpolation.y);
		color += texture(tex, in_texCoord + tileOffset(firstTile, 1.0, 1.0)) * (    tileInterpolation.x) * (    tileInterpolation.y);
	}
	
	if(color.a < 0.05) discard;
	out_color = color;
}