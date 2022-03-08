//
// Created by Jeremy Guo on 2022/3/6.
//

#include "Material.h"
#include "Texture.h"

namespace glfw {
//    Material::Material(glfwApp *app) {
//        mApp = app;
//    }
//
//    Material::~Material() {
//        this->destroy();
//    }
//
//    void Material::destroy() {
//        if (this->diffuse) {
//            this->diffuse->destroy();
//            this->diffuse = nullptr;
//        }
//        if (this->illumination) {
//            this->illumination->destroy();
//            this->illumination = nullptr;
//        }
//        if (this->normal) {
//            this->normal->destroy();
//            this->normal = nullptr;
//        }
//        if (this->opacity) {
//            this->opacity->destroy();
//            this->opacity = nullptr;
//        }
//    }
//
//    void Material::loadDiffuse(const char *filename, VkCommandPool commandPool, VkQueue graphicsQueue) {
//        this->diffuse = new Texture(mApp);
//        this->diffuse->load(filename, commandPool, graphicsQueue);
//    }
//
//    void Material::loadIllumination(const char *filename, VkCommandPool commandPool, VkQueue graphicsQueue) {
//        this->illumination = new Texture(mApp);
//        this->illumination->load(filename, commandPool, graphicsQueue);
//    }
//
//    void Material::loadOpacity(const char *filename, VkCommandPool commandPool, VkQueue graphicsQueue) {
//        this->opacity = new Texture(mApp);
//        this->opacity->load(filename, commandPool, graphicsQueue);
//    }
//
//    void Material::loadNormal(const char *filename, VkCommandPool commandPool, VkQueue graphicsQueue) {
//        this->normal = new Texture(mApp);
//        this->normal->load(filename, commandPool, graphicsQueue);
//    }
}