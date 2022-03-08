//
// Created by JeremyGuo on 2022/3/6.
//

#include "Instance.h"
#include "Mesh.h"
#include "glfwApp.h"
#include "Buffer.h"

namespace glfw {
    Instance::Instance(glfwApp* app, Mesh *mesh) {
        mMesh = mesh;
        mModel = glm::mat4(1.0f);

        mApp = app;
    }

    void Instance::initGPUMemory(VkDescriptorPool descPool, VkDescriptorSetLayout defaultLayout, VkCommandPool commandPool, VkQueue graphicsQueue, int num_frame) {
        mDescPool = descPool;

        std::vector<VkDescriptorSetLayout> layouts(num_frame, defaultLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(num_frame);
        allocInfo.pSetLayouts = layouts.data();
        mModelDesc.resize(num_frame);
        if (vkAllocateDescriptorSets(mApp->device, &allocInfo, mModelDesc.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate descriptor sets!");

        for (int i = 0; i < num_frame; i ++) {
            mModelBuffer.push_back(new Buffer(mApp));
            mModelBuffer[i]->create(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            mModelBuffer[i]->uploadData(&this->mModel, sizeof(glm::mat4));

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = mModelBuffer[i]->getBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(glm::mat4);

            VkWriteDescriptorSet descriptorWrite;
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.pNext = nullptr;
            descriptorWrite.dstSet = mModelDesc[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(mApp->device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    Instance::~Instance() {
        this->destroy(0);
    }

    void Instance::destroy(int destroy_mesh) {
        if (destroy_mesh && mMesh) {
            mMesh->destroy();
            mMesh = nullptr;
        }
        if (mDescPool) {
            vkFreeDescriptorSets(mApp->device, mDescPool, mModelDesc.size(), mModelDesc.data());
            mModelDesc.resize(0);
            mDescPool = VK_NULL_HANDLE;
            for (auto &buffer : mModelBuffer)
                delete buffer;
            mModelBuffer.resize(0);
        }
    }

    VkDescriptorSet Instance::getModelDescriptorSet(int frame_index) {
        return mModelDesc[frame_index];
    }
}