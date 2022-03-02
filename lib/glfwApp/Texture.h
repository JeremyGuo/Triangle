//
// Created by JeremyGuo on 2022/3/2.
//

#ifndef TRIANGLE_TEXTURE_H
#define TRIANGLE_TEXTURE_H

#include <common.h>

namespace glfw {
    class glfwApp;

    class Texture {
    public:
        Texture(glfw::glfwApp *app);

        Texture(const Texture &) = delete;

        VkResult create(VkImageType imageType,
                        VkFormat format,
                        VkExtent3D extent,
                        VkImageTiling tiling,
                        VkImageUsageFlags usage,
                        VkMemoryPropertyFlags memoryProperties);

        void destroy();

        bool load(const char *fileName, VkCommandPool commandPool, VkQueue graphicsQueue);

        VkResult createImageView(VkImageViewType viewType, VkFormat format, VkImageSubresourceRange subresourceRange);

        VkResult createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode,
                               VkSamplerAddressMode addressMode);

        void
        transition(VkImageLayout oldLayout, VkImageLayout newLayout, VkQueue graphicsQueue, VkCommandPool commandPool);

        // getters
        VkFormat getFormat() const;

        VkImage getImage() const;

        VkImageView getImageView() const;

        VkSampler getSampler() const;

    private:
        VkFormat mFormat;
        VkImage mImage;
        VkDeviceMemory mMemory;
        VkImageView mImageView;
        VkSampler mSampler;
        glfw::glfwApp *mApp;

        uint32_t getMemoryType(VkMemoryRequirements &memoryRequirements, VkMemoryPropertyFlags memoryProperties);
    };
}

#endif //TRIANGLE_TEXTURE_H
