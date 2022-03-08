//
// Created by JeremyGuo on 2022/3/6.
//

#ifndef TRIANGLE_INSTANCE_H
#define TRIANGLE_INSTANCE_H

#include <common.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace glfw {
    class Mesh;
    class glfwApp;
    class Buffer;
    struct Instance {
        glm::mat4 mModel;
        std::vector<VkDescriptorSet> mModelDesc;
        std::vector<Buffer*> mModelBuffer;

        Mesh* mMesh;
        glfwApp* mApp;
        VkDescriptorPool mDescPool;

        Instance(glfwApp* app, Mesh* mesh);
        virtual ~Instance();
        void initGPUMemory(VkDescriptorPool descPool, VkDescriptorSetLayout defaultLayout, VkCommandPool commandPool, VkQueue graphicsQueue, int num_frame);
        void destroy(int destroy_mesh = 0);
        VkDescriptorSet getModelDescriptorSet(int frame_index);
    };
}


#endif //TRIANGLE_INSTANCE_H
