#version 450

layout(location = 0) in vec4 in_texCoords[1];
layout(location = 1) in uvec2 in_textureHandle[1];
layout(location = 2) in vec3 in_position[1];
layout(location = 3) in float in_rotation[1];
layout(location = 4) in vec2 in_scale[1];

layout(location = 0) uniform mat4 c_viewProjection;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
layout(location = 0) out vec2 out_texCoord;
layout(location = 1) out flat uvec2 out_textureHandle;

void main()
{
	out_textureHandle = in_textureHandle[0];
	
	// Bottom-Left
	out_texCoord = in_texCoords[0].xy;
	vec3 worldPos = in_position[0];
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	// Bottom-Right
	out_texCoord = in_texCoords[0].zy;
	worldPos = in_position[0];
	worldPos.x += in_scale[0].x;
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	// Top-Left
	out_texCoord = in_texCoords[0].xw;
	worldPos = in_position[0];
	worldPos.y += in_scale[0].y;
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	// Top-Right
	out_texCoord = in_texCoords[0].zw;
	worldPos = in_position[0];
	worldPos.xy += in_scale[0].xy;
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	EndPrimitive();
}