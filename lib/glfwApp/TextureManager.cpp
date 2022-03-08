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
        for (auto& p : mTextures)
            p->destroy();
        mTextures.clear();
        mTextureIds.clear();
    }

    int TextureManager::getTexutreNum() {
        return mTextures.size();
    }

    VkDescriptorSet TextureManager::getDescriptorSet(int frame) {
        return descriptorSets[frame];
    }

    int TextureManager::getTexture(const char *name, VkCommandPool commandPool, VkQueue graphicsQueue) {
        if (this->mTextureIds.count(std::string(name)))
            return this->mTextureIds[std::string(name)];
        Texture* nTexture = new Texture(mApp);
        nTexture->load(name, commandPool, graphicsQueue);
        int ret = this->mTextures.size();
        this->mTextureIds[std::string(name)] = ret;
        this->mTextures.push_back(nTexture);

        // TODO: init sampler & Image View

        return ret;
    }
}