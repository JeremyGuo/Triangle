//
// Created by JeremyGuo on 2022/3/2.
//

#ifndef TRIANGLE_MESH_H
#define TRIANGLE_MESH_H

#include <cstdint>
#include <Buffer.h>
#include <Texture.h>

namespace glfw {
    class Buffer;
    struct Mesh {
        uint32_t numVertices;
        uint32_t numFaces;

        Buffer positions;
        Buffer attribs;
        Buffer indices;
        Buffer faces;
        Buffer matIDs;
    };

    struct Material {
        Texture texture;
    };

    struct Scene {
        std::vector<Mesh*>  meshes;
    };
}

#endif //TRIANGLE_MESH_H
