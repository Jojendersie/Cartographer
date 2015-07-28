#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texCoord;
layout(location = 3) in vec4 in_instanceOrientation;
layout(location = 4) in vec3 in_instancePosition;

layout(location = 0) out vec3 out_position; // World position
layout(location = 1) out vec3 out_normal;	// World normal
layout(location = 2) out vec2 out_texCoord;

layout(location = 0) uniform mat4 c_viewProjection;

vec3 quaternionRotation(vec4 _q, vec3 _v)
{
	float x1 = _q.y*_v.z - _q.z*_v.y;
    float y1 = _q.z*_v.x - _q.x*_v.z;
    float z1 = _q.x*_v.y - _q.y*_v.x;

    return vec3(
        _v.x + 2.0 * (_q.w*x1 + _q.y*z1 - _q.z*y1),
        _v.y + 2.0 * (_q.w*y1 + _q.z*x1 - _q.x*z1),
        _v.z + 2.0 * (_q.w*z1 + _q.x*y1 - _q.y*x1)
    );
}

void main()
{
	// World transformation
	out_position = quaternionRotation(in_instanceOrientation, in_position);
	out_position += in_instancePosition;
	// Transform into view space
	gl_Position = vec4(out_position, 1) * c_viewProjection;
	
	// Pass through
	out_normal = in_normal;
	out_texCoord = in_texCoord;
}