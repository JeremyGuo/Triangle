//
// Created by JeremyGuo on 2022/3/9.
//

#ifndef TRIANGLE_MESHMANAGER_H
#define TRIANGLE_MESHMANAGER_H

#include "common.h"
#include <unordered_map>

namespace glfw {
    class Mesh;
    class glfwApp;
    class MeshManager {
    public:
        MeshManager(glfwApp* app);
        virtual ~MeshManager();
        MeshManager(const MeshManager&) = delete;

        Mesh* getMesh(const char* name, VkCommandPool commandPool, VkQueue graphicsQueue);
    private:
        glfwApp* mApp;
        std::unordered_map<std::string, Mesh*> mMeshes;
    };
}


#endif //TRIANGLE_MESHMANAGER_H
