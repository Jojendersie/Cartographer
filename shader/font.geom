#version 450

layout(location = 0) in vec4 in_texCoords[1];
layout(location = 1) in vec2 in_size[1];
layout(location = 2) in vec4 in_color[1];
layout(location = 3) in vec3 in_position[1];
layout(location = 4) in float in_rotation[1];

layout(location = 0) uniform mat4 c_viewProjection;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
layout(location = 0) out vec2 out_texCoord;
layout(location = 1) out flat vec4 out_color;

void main()
{
	out_color = in_color[0];
	
	mat2 rot;
	rot[0][0] = rot[1][1] = cos(in_rotation[0]);
	rot[1][0] = sin(in_rotation[0]);
	rot[0][1] = - rot[1][0];
	
	vec2 toRight = rot * vec2(in_size[0].x, 0);
	vec2 toTop   = rot * vec2(0, in_size[0].y);
	
	// Bottom-Left
	out_texCoord = in_texCoords[0].xy;
	vec3 worldPos = in_position[0];
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	// Bottom-Right
	out_texCoord = in_texCoords[0].zy;
	worldPos = in_position[0];
	worldPos.xy += toRight;
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	// Top-Left
	out_texCoord = in_texCoords[0].xw;
	worldPos = in_position[0];
	worldPos.xy += toTop;
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	// Top-Right
	out_texCoord = in_texCoords[0].zw;
	worldPos = in_position[0];
	worldPos.xy += toRight + toTop;
	gl_Position = vec4(worldPos, 1) * c_viewProjection;
	EmitVertex();

	EndPrimitive();
}