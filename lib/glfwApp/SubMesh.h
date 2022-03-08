//
// Created by JeremyGuo on 2022/3/5.
//

#ifndef TRIANGLE_SUBMESH_H
#define TRIANGLE_SUBMESH_H

#include <cstdint>
#include <Buffer.h>
#include <Texture.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <tiny_obj_loader.h>
#include <unordered_map>
#include <Vertex.h>

namespace glfw {
    class glfwApp;
    class Buffer;
    class Material;
    struct SubMesh {
        uint32_t numIndices;

        Buffer *vertex;
        Buffer *indice;

        Material* material;
        char* mat_name;

        glfwApp* mApp;
        std::vector<uint32_t> matIDs;

        SubMesh(glfwApp* app);
        virtual ~SubMesh();
        void loadSubMesh(tinyobj::shape_t &shape, tinyobj::attrib_t &attrib, VkCommandPool commandPool, VkQueue& graphicsQueue);
        void loadSubMesh(tinyobj::shape_t &shape, tinyobj::attrib_t &attrib, std::unordered_map<Vertex, uint32_t> &vert_pool, std::vector<Vertex> &vertices, glfw::Buffer* vert_buffer, VkCommandPool commandPool, VkQueue& graphicsQueue);

        void destroy();
    private:
        bool needDestroyVertex;
    };
}


#endif //TRIANGLE_SUBMESH_H
