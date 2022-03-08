//
// Created by JeremyGuo on 2022/3/2.
//

#include "Mesh.h"
#include "SubMesh.h"
#include "glfwApp.h"
#include "TextureManager.h"

namespace glfw {
    Mesh::Mesh(glfwApp *app) {
        mApp = app;
    }

    Mesh::~Mesh() {
        this->destroy();
    }

    void Mesh::destroy() {
        if (this->vertexBuffer) {
            delete this->vertexBuffer;
            this->vertexBuffer = NULL;
        }
        for (SubMesh* &smesh : submesh) {
            delete smesh;
        }
        submesh.resize(0);
    }

    void Mesh::loadObject(const char *filename, VkCommandPool commandPool, VkQueue graphicsQueue) {
        this->vertexBuffer = new glfw::Buffer(mApp);
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename))
            throw std::runtime_error(err);

        std::unordered_map<Vertex, uint32_t> tmp_vert_pool;
        std::vector<Vertex> tmp_vert;

        fprintf(stdout, "Decoding Mesh\n");

        mMats.resize(materials.size());
        for (auto& mat : materials) {
            std::cout << "TEX:" << mat.diffuse_texname << std::endl;
            mMats.push_back(mApp->textureManager->getTexture(mat.diffuse_texname.c_str(), commandPool, graphicsQueue));
        }

        for (auto& shape : shapes) {
            SubMesh* smesh = new SubMesh(mApp);
            smesh->loadSubMesh(shape, attrib, tmp_vert_pool, tmp_vert, this->vertexBuffer, commandPool, graphicsQueue);
            this->submesh.push_back(smesh);
        }

        {
            /**
             * Create Vertex Buffer
             */
            glfw::Buffer stagingBuffer(mApp);
            VkDeviceSize bufferSize = sizeof(tmp_vert[0]) * tmp_vert.size();
            stagingBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            stagingBuffer.uploadData(tmp_vert.data(), bufferSize, 0);
            this->vertexBuffer->create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            stagingBuffer.copyTo(*this->vertexBuffer, commandPool, graphicsQueue, bufferSize);
            stagingBuffer.destroy();
        }
    }
}
