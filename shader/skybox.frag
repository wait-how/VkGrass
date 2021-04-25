#version 460

layout (location = 0) in vec3 p;

layout (location = 0) out vec4 fragcolor;

void main() {
    const vec4 skyc = vec4(1.0, 1.0, 1.0, 1.0);
    const vec4 groundc = vec4(0.0);

    fragcolor = mix(skyc, groundc, p.y + 0.3);
}