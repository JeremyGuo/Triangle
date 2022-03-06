//
// Created by Jeremy Guo on 2022/3/6.
//

#ifndef TRIANGLE_MATERIAL_H
#define TRIANGLE_MATERIAL_H

#include "common.h"
#include <unordered_map>

namespace glfw {
    class Texture;
    class glfwApp;
    struct Material {
        Material(glfwApp* app);
        virtual ~Material();

        Texture* diffuse;
        Texture* illumination;
        Texture* opacity;
        Texture* normal;

        glfwApp* mApp;

        void destroy();
        void loadDiffuse(const char* filename, VkCommandPool commandPool, VkQueue graphicsQueue);
        void loadIllumination(const char* filename, VkCommandPool commandPool, VkQueue graphicsQueue);
        void loadOpacity(const char* filename, VkCommandPool commandPool, VkQueue graphicsQueue);
        void loadNormal(const char* filename, VkCommandPool commandPool, VkQueue graphicsQueue);
    };
}


#endif //TRIANGLE_MATERIAL_H
