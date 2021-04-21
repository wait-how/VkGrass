#version 460

layout (location = 0) in vec3 position;

layout (set = 0, binding = 0, std140) uniform uniformBuffer {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout (location = 0) out vec3 p;

void main() {
    // kill any camera translations by casting to mat3
    vec4 pos = ubo.proj * mat4(mat3(ubo.view)) * ubo.model * vec4(position, 1.0);

    // ensure that depth is 1.0 all the time
    gl_Position = pos.xyww;

    p = position;
}