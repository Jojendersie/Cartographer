#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texCoord;
layout(binding = 0) uniform sampler2D diffuseTexture;

layout(location = 0) out vec4 out_color;

void main()
{
	float shade = abs(dot(in_normal, normalize(vec3(1.0, 2.0, 0.5))));
	vec4 color = texture(diffuseTexture, in_texCoord);
	out_color = vec4(color.rgb * shade, color.a);
}