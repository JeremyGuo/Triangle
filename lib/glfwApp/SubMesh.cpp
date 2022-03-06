//
// Created by JeremyGuo on 2022/3/5.
//

#include "SubMesh.h"

#include <Buffer.h>

namespace glfw {
    SubMesh::SubMesh(glfwApp *app): mApp(app) {
        numIndices = 0;
        needDestroyVertex = false;

        vertex = NULL;
        indice = NULL;

        mat_name = NULL;
        material = NULL;
    }

    SubMesh::~SubMesh() {
        this->destroy();
    }

    void SubMesh::loadSubMesh(tinyobj::shape_t &shape, tinyobj::attrib_t &attrib, VkCommandPool commandPool, VkQueue& graphicsQueue) {
        throw std::runtime_error("SubMesh: Dont call this function for standalone creation");
        std::unordered_map<Vertex, uint32_t> tmp_vert_pool;
        std::vector<Vertex> tmp_vert;
        this->vertex = new glfw::Buffer(mApp);
        this->needDestroyVertex = true;
        this->loadSubMesh(shape, attrib, tmp_vert_pool, tmp_vert, this->vertex, commandPool, graphicsQueue);
        {
            /**
             * Create Vertex Buffer
             */
            glfw::Buffer stagingBuffer(mApp);
            VkDeviceSize bufferSize = sizeof(tmp_vert[0]) * tmp_vert.size();
            stagingBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            stagingBuffer.uploadData(tmp_vert.data(), bufferSize, 0);
            this->vertex->create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            stagingBuffer.copyTo(*this->vertex, commandPool, graphicsQueue, bufferSize);
            stagingBuffer.destroy();
        }
    }

    void SubMesh::loadSubMesh(tinyobj::shape_t &shape, tinyobj::attrib_t &attrib, std::unordered_map<Vertex, uint32_t> &vert_pool, std::vector<Vertex> &vertices, glfw::Buffer* vert_buffer, VkCommandPool commandPool, VkQueue& graphicsQueue) {
        this->vertex = vert_buffer;
        std::unordered_map<Vertex, uint32_t> uniqueVertices;
        std::vector<uint32_t> indices;
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            vertex.color = {1.0f, 1.0f, 1.0f};
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
        this->indice = new glfw::Buffer(mApp);
        {
            /**
             * Create Indices Buffer
             */
            VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
            glfw::Buffer stagingBuffer(mApp);
            stagingBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            stagingBuffer.uploadData(indices.data(), bufferSize);
            this->indice->create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            stagingBuffer.copyTo(*this->indice, commandPool, graphicsQueue, bufferSize);
            stagingBuffer.destroy();
        }
        this->numIndices = static_cast<uint32_t>(indices.size());
    }

    void SubMesh::destroy() {
        if (this->indice) {
            this->indice->destroy();
            delete this->indice;
            this->indice = NULL;
        }
    }
}