#include "terrain.hpp"

terrain::terrain(unsigned int nwidth, unsigned int nheight, float xlim, float zlim, const features f) {

    verts.reserve(nwidth * nheight);
    indices.reserve(6 * nwidth * nheight);

    float xstep = (xlim * 2) / nwidth;
    float zstep = (zlim * 2) / nwidth;

    // write vertices
    for (size_t xn = 0; xn < nwidth; xn++) {
        for (size_t zn = 0; zn < nheight; zn++) {
            const float xfscale = 0.1;
            const float zfscale = 0.1;

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
        }
    }

    assert(verts.size() == nwidth * nheight);

    // write indices
    for (size_t i = 0; i < 6 * nwidth * nheight; i += 6) {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + nwidth);

        indices.push_back(i + 1);
        indices.push_back(i + nwidth);
        indices.push_back(i + nwidth + 1);
    }

    assert(indices.size() == 6 * nwidth * nheight);

    if (f & features::normal) {
        for (size_t i = 0; i < nwidth * nheight; i++) {
            verts[i].normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }

    if (f & features::uv) {
        for (size_t i = 0; i < nwidth * nheight; i++) {
            verts[i].uv = glm::vec2(0.0f, 0.0f);
        }
    }

    if (f & features::tangent) {

    }
}