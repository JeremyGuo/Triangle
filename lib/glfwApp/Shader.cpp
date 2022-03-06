//
// Created by Jeremy Guo on 2022/3/6.
//

#include "Shader.h"
#include "glfwApp.h"

namespace glfw {
    Shader::Shader(glfwApp* app) {
        mApp = app;

        mVertShaderModule = VK_NULL_HANDLE;
        mFragShaderModule = VK_NULL_HANDLE;
    }

    VkShaderModule Shader::getVertShader() {
        return mVertShaderModule;
    }

    VkShaderModule Shader::getFragShader() {
        return mFragShaderModule;
    }

    void Shader::loadShaderModule(const char *vert_shader, const char* frag_shader) {
        if (vert_shader) {
            auto vertShaderCode = readFile(std::string(vert_shader));
            this->mVertShaderModule = createShaderModule(mApp->device, vertShaderCode);
        }
        if (frag_shader) {
            auto fragShaderCode = readFile(std::string(frag_shader));
            this->mFragShaderModule = createShaderModule(mApp->device, fragShaderCode);
        }
    }

    void Shader::destroy() {
        if (mVertShaderModule) {
            vkDestroyShaderModule(mApp->device, mVertShaderModule, nullptr);
            mVertShaderModule = VK_NULL_HANDLE;
        }
        if (mFragShaderModule) {
            vkDestroyShaderModule(mApp->device, mFragShaderModule, nullptr);
            mFragShaderModule = VK_NULL_HANDLE;
        }
    }

    Shader::~Shader() {
        this->destroy();
    }

    VkPipelineShaderStageCreateInfo Shader::getVertStageInfo(const char* fname) {
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = getVertShader();
        vertShaderStageInfo.pName = fname;
        return vertShaderStageInfo;
    }

    VkPipelineShaderStageCreateInfo Shader::getFragStageInfo(const char* fname) {
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = getFragShader();
        fragShaderStageInfo.pName = fname;
        return fragShaderStageInfo;
    }
}
