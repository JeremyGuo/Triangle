//
// Created by Jeremy Guo on 2022/3/8.
//

#ifndef TRIANGLE_TEXTUREMANAGER_H
#define TRIANGLE_TEXTUREMANAGER_H

#include "common.h"
#include <unordered_map>

namespace glfw {
    class glfwApp;
    class Texture;
    class TextureManager {
    public:
        TextureManager(glfwApp* app);
        virtual ~TextureManager();

        int getTexture(const char* name, VkCommandPool commandPool, VkQueue graphicsQueue);

        int getTexutreNum();
        void initDescriptorSet(); // TODO
        VkDescriptorSet getDescriptorSet(int frame);

        void destroy();
    private:
        std::unordered_map<std::string, int> mTextureIds;
        std::vector<Texture*> mTextures;

        std::vector<VkDescriptorSet> descriptorSets;

        glfwApp* mApp;
    };
}


#endif //TRIANGLE_TEXTUREMANAGER_H
