//
// Created by JeremyGuo on 2022/3/2.
//

#ifndef TRIANGLE_MESH_H
#define TRIANGLE_MESH_H

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
    class SubMesh;
    class Texture;
    class Mesh {
    public:
        Mesh(glfwApp* app);
        virtual ~Mesh();

        void destroy();

        void loadObject(const char* filename, VkCommandPool commandPool, VkQueue graphicsQueue);

        Buffer* vertexBuffer;
        std::vector<SubMesh*> submesh;
        std::vector<Texture*> mMats;
    private:
        glfwApp* mApp;
    };
}

#endif //TRIANGLE_MESH_H
