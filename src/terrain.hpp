#pragma once

#include "vformat.hpp"
#include <glm/gtc/noise.hpp>

#include <vector>
#include <cassert>

class terrain {
public:
    enum features {
        normal = 0x1,
        uv = 0x2,
        tangent = 0x4,
    };

    terrain() = delete;
    terrain(unsigned int nwidth, unsigned int nheight, float xlim, float zlim, const features f);

    std::vector<vformat::vertex> verts;
    std::vector<uint32_t> indices;
};