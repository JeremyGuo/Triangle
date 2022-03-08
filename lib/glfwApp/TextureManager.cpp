//
// Created by Jeremy Guo on 2022/3/8.
//

#include "TextureManager.h"
#include <glfwApp.h>
#include <Texture.h>

namespace glfw {
    TextureManager::TextureManager(glfwApp *app) {
        mApp = app;
    }

    TextureManager::~TextureManager() {
        this->destroy();
    }

    void TextureManager::destroy() {
        for (auto& p : mTextures) {
            p.second->destroy();
        }
        mTextures.clear();
    }

    Texture* TextureManager::getTexture(const char *name, VkCommandPool commandPool, VkQueue graphicsQueue) {
        if (this->mTextures.count(std::string(name)))
            return this->mTextures[std::string(name)];
        Texture* nTexture = new Texture(mApp);
        nTexture->load(name, commandPool, graphicsQueue);
        this->mTextures[std::string(name)] = nTexture;
        return nTexture;
    }
}