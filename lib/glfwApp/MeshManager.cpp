//
// Created by JeremyGuo on 2022/3/9.
//

#include "MeshManager.h"
#include <Mesh.h>
#include <glfwApp.h>

namespace glfw {
    MeshManager::MeshManager(glfwApp *app) {
        mApp = app;
    }

    Mesh *MeshManager::getMesh(const char *name, VkCommandPool commandPool, VkQueue graphicsQueue) {
        if (this->mMeshes.count(std::string(name)))
            return this->mMeshes[std::string(name)];
        Mesh* ret = new Mesh(mApp);
        ret->loadObject(name, commandPool, graphicsQueue);
        this->mMeshes[std::string(name)] = ret;
        return ret;
    }

    MeshManager::~MeshManager() {
        for (auto& mesh : mMeshes) {
            delete mesh.second;
        }
        mMeshes.clear();
    }
}
