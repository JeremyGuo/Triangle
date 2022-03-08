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
//        Texture* bump;

        glfwApp* mApp;
    };
}


#endif //TRIANGLE_MATERIAL_H
