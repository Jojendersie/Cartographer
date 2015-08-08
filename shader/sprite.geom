#version 450

layout(location = 0) in vec4 in_texCoords[1];
layout(location = 1) in uvec2 in_textureHandle[1];
layout(location = 2) in vec2 in_numTiles[1];
layout(location = 3) in vec3 in_position[1];
layout(location = 4) in float in_rotation[1];
layout(location = 5) in vec4 in_scale[1];
layout(location = 6) in vec2 in_anim[1];

layout(location = 0) uniform mat4 c_viewProjection;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
layout(location = 0) out vec2 out_texCoord;
layout(location = 1) out flat uvec2 out_textureHandle;
layout(location = 2) out flat vec4 out_anim;
layout(location = 3) out flat vec2 out_numTiles;

void main()
{
	out_textureHandle = in_textureHandle[0];
	out_anim.xy = in_anim[0];
	// Also transmit texture tile size to allow offsetting due to animation
	out_anim.zw = in_texCoords[0].zw - in_texCoords[0].xy;
	out_anim.w = -out_anim.w;
	out_numTiles = in_numTiles[0];
	
	mat2 rot;
	rot[0][0] = rot[1][1] = cos(in_rotation[0]);
	rot[1][0] = sin(in_rotation[0]);
	rot[0][1] = - rot[1][0];
	
	// Bottom-Left
	out_texCoord = in_texCoords[0].xy;
	vec3 worldPos = in_position[0];
	worldPos.xy += rot * in_scale[0].xy;
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	// Bottom-Right
	out_texCoord = in_texCoords[0].zy;
	worldPos = in_position[0];
	worldPos.xy += rot * in_scale[0].zy;
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	// Top-Left
	out_texCoord = in_texCoords[0].xw;
	worldPos = in_position[0];
	worldPos.xy += rot * in_scale[0].xw;
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	// Top-Right
	out_texCoord = in_texCoords[0].zw;
	worldPos = in_position[0];
	worldPos.xy += rot * in_scale[0].zw;
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	EndPrimitive();
}