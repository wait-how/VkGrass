#version 460

layout (location = 0) in vec3 p;

layout (location = 0) out vec4 fragcolor;

layout (set = 0, binding = 1) uniform samplerCube cubemap;

void main() {
    fragcolor = texture(cubemap, p);
}