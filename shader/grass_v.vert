#version 460

layout (binding = 0, location = 0) in vec3 position;
layout (binding = 0, location = 1) in vec3 normal;
layout (binding = 0, location = 2) in vec2 texcoord;

layout (binding = 1, location = 3) in mat4 instance_model;

layout (set = 0, binding = 0) uniform uniformBuffer {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout (location = 0) out vec3 p;
layout (location = 1) out vec3 n;
layout (location = 2) out vec2 uv;

void main() {

	vec4 p4 = ubo.model * instance_model * vec4(position, 1.0);

	gl_Position = ubo.proj * ubo.view * p4;
	
	p = p4.xyz;
	n = mat3(ubo.model) * normal;
	uv = texcoord;
}
