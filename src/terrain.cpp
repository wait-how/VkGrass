#include "terrain.hpp"

terrain::terrain(unsigned int nwidth, unsigned int nheight, float xlim, float zlim, const uint8_t f) {
    regen(nwidth, nheight, xlim, zlim, f);
}

void terrain::regen(unsigned int nwidth, unsigned int nheight, float xlim, float zlim, const uint8_t f) {
    if (nwidth < 2 && nheight < 2) {
        throw std::invalid_argument("terrain width and height must be >2!");
    }
    
    verts.reserve(nwidth * nheight);
    indices.reserve(6 * nwidth * nheight);

    float xstep = (xlim * 2) / nwidth;
    float zstep = (zlim * 2) / nwidth;

    // write vertices
    for (size_t zn = 0; zn < nheight; zn++) {
        for (size_t xn = 0; xn < nwidth; xn++) {
            /*
            scaling by small constants works because f = v / wavelength (from physics),
            and our "noise velocity" is xstep or zstep.

            if we increase the wavelength, we can effectively decrease the frequency.
            */

            float x = -xlim + xstep * xn;
            float z = -zlim + zstep * zn;
            float height = glm::simplex(glm::vec2(x * xfscale, z * zfscale));

            verts.emplace_back();
            verts[verts.size() - 1].pos = glm::vec3(x, height, z);

            if (f & features::uv) {
                verts[verts.size() - 1].uv = glm::vec2(xn, zn);
            }
        }
    }

    assert(verts.size() == nwidth * nheight);

    // write indices
    // all calculations for indices are only valid from the top left index, which is 
    // why weird index arithmetic is going on
    size_t vi = 0;
    for (size_t i = 0; i < (nwidth - 1) * (nheight - 1); i++) {
        if (vi % nwidth == nwidth - 1) {
            vi++;
        }

        uint32_t tl = vi;
        uint32_t tr = vi + 1;
        uint32_t bl = vi + nwidth;
        uint32_t br = vi + nwidth + 1;

        indices.push_back(bl);
        indices.push_back(tr);
        indices.push_back(tl);

        indices.push_back(bl);
        indices.push_back(br);
        indices.push_back(tr);

        vi++;
    }

    assert(indices.size() == 6 * (nwidth - 1) * (nheight - 1));
    assert(indices.size() % 3 == 0);

    for (size_t i = 0; i < indices.size(); i++) {
        assert(indices[i] < verts.size());
    }

    // TODO: write this
    if (f & features::normal) {
        for (size_t i = 0; i < verts.size(); i++) {
            verts[i].normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }

    // TODO: handle this eventually
    if (f & features::tangent) {

    }
}

float terrain::getHeight(const float x, const float z) {
    return glm::simplex(glm::vec2(x * xfscale, z * zfscale));
}