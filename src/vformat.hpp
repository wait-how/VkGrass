#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace vformat {
// defining a very specific vertex buffer format here to allow for more than one source for geometry data
// (assimp and procedurally generated stuff)
struct __attribute__((packed)) vertex {
    // have to align everything to vec4 boundaries for glsl
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 normal;
    alignas(16) glm::vec2 uv;
    // alignas(16) glm::vec3 tangent;
};
}