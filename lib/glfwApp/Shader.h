//
// Created by Jeremy Guo on 2022/3/6.
//

#ifndef TRIANGLE_SHADER_H
#define TRIANGLE_SHADER_H

#include "common.h"

namespace glfw {
    class glfwApp;
    struct Shader {
        Shader(glfwApp* app);
        virtual ~Shader();

        VkShaderModule getVertShader();
        VkShaderModule getFragShader();
        void loadShaderModule(const char* vert_shader, const char* frag_shader);

        VkPipelineShaderStageCreateInfo getVertStageInfo(const char* fname);
        VkPipelineShaderStageCreateInfo getFragStageInfo(const char* fname);

        void destroy();
    private:
        VkShaderModule mVertShaderModule;
        VkShaderModule mFragShaderModule;

        glfwApp* mApp;
    };
}


#endif //TRIANGLE_SHADER_H
