#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texCoord;
layout(location = 3) in vec4 in_instanceOrientation;
layout(location = 4) in vec3 in_instancePosition;

layout(location = 0) out vec3 out_position; // World position
layout(location = 1) out vec3 out_normal;	// World normal
layout(location = 2) out vec2 out_texCoord;

uniform mat4 c_viewProjection;

void main()
{
	// World transformation
	out_position = in_position;//TODO: rot
	out_position += in_instancePosition;
	// Transform into view space
	gl_Position = vec4(out_position, 1) * c_viewProjection;
	
	// Pass through
	out_normal = in_normal;
	out_texCoord = in_texCoord;
}