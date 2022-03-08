//
// Created by JeremyGuo on 2022/3/2.
//

#include <Texture.h>
#include <Buffer.h>
#include <common.h>
#include <stb_image.h>
#include <glfwApp.h>

namespace glfw {
    Texture::Texture(glfw::glfwApp *app) {
        mApp = app;

        mSampler = VK_NULL_HANDLE;
        mImage = VK_NULL_HANDLE;
        mMemory = VK_NULL_HANDLE;
        mImageView = VK_NULL_HANDLE;
    }

    VkResult Texture::create(VkImageType imageType,
                             VkFormat format,
                             VkExtent3D extent,
                             VkImageTiling tiling,
                             VkImageUsageFlags usage,
                             VkMemoryPropertyFlags memoryProperties) {
        VkResult result = VK_SUCCESS;
        mFormat = format;
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = imageType;
        imageCreateInfo.format = format;
        imageCreateInfo.extent = extent;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.usage = usage;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        result = vkCreateImage(mApp->device, &imageCreateInfo, nullptr, &mImage);
        if (VK_SUCCESS == result) {
            VkMemoryRequirements memoryRequirements = {};
            vkGetImageMemoryRequirements(mApp->device, mImage, &memoryRequirements);

            VkMemoryAllocateInfo memoryAllocateInfo = {};
            memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memoryAllocateInfo.allocationSize = memoryRequirements.size;
            memoryAllocateInfo.memoryTypeIndex = getMemoryType(memoryRequirements, memoryProperties);

            result = vkAllocateMemory(mApp->device, &memoryAllocateInfo, nullptr, &mMemory);
            if (VK_SUCCESS != result) {
                vkDestroyImage(mApp->device, mImage, nullptr);
                mImage = VK_NULL_HANDLE;
                mMemory = VK_NULL_HANDLE;
            } else {
                result = vkBindImageMemory(mApp->device, mImage, mMemory, 0);
                if (VK_SUCCESS != result) {
                    vkDestroyImage(mApp->device, mImage, nullptr);
                    vkFreeMemory(mApp->device, mMemory, nullptr);
                    mImage = VK_NULL_HANDLE;
                    mMemory = VK_NULL_HANDLE;
                }
            }
        }
        return result;
    }

    uint32_t Texture::getMemoryType(VkMemoryRequirements &memoryRequirements, VkMemoryPropertyFlags memoryProperties) {
        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(mApp->physicalDevice, &properties);

        uint32_t result = -1;
        for (uint32_t memoryTypeIndex = 0; memoryTypeIndex < VK_MAX_MEMORY_TYPES; ++memoryTypeIndex) {
            if (memoryRequirements.memoryTypeBits & (1 << memoryTypeIndex)) {
                if ((properties.memoryTypes[memoryTypeIndex].propertyFlags & memoryProperties) == memoryProperties) {
                    result = memoryTypeIndex;
                    break;
                }
            }
        }

        if (result == -1)
            throw std::runtime_error("Buffer: Failed to find correct memory type");

        return result;
    }

    void Texture::destroy() {
        if (mSampler) {
            vkDestroySampler(mApp->device, mSampler, nullptr);
            mSampler = VK_NULL_HANDLE;
        }
        if (mImageView) {
            vkDestroyImageView(mApp->device, mImageView, nullptr);
            mImageView = VK_NULL_HANDLE;
        }
        if (mMemory) {
            vkFreeMemory(mApp->device, mMemory, nullptr);
            mMemory = VK_NULL_HANDLE;
        }
        if (mImage) {
            vkDestroyImage(mApp->device, mImage, nullptr);
            mImage = VK_NULL_HANDLE;
        }
    }

    bool Texture::load(const char *fileName, VkCommandPool commandPool, VkQueue graphicsQueue) {
        int width, height, channels;
        bool textureHDR = false;
        stbi_uc *imageData = nullptr;

        std::string fileNameString(fileName);
        const std::string extension = fileNameString.substr(fileNameString.length() - 3);

        if (extension == "hdr") {
            textureHDR = true;
            imageData = reinterpret_cast<stbi_uc *>(stbi_loadf(fileName, &width, &height, &channels, STBI_rgb_alpha));
        } else {
            imageData = stbi_load(fileName, &width, &height, &channels, STBI_rgb_alpha);
        }

        VkResult result = VK_SUCCESS;

        if (!imageData)
            return false;

        const int bpp = textureHDR ? sizeof(float[4]) : sizeof(uint8_t[4]);
        VkDeviceSize imageSize = static_cast<VkDeviceSize>(width * height * bpp);

        glfw::Buffer stagingBuffer(mApp);
        result = stagingBuffer.create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (result != VK_SUCCESS) {
            stbi_image_free(imageData);
            return false;
        }
        result = stagingBuffer.uploadData(imageData, imageSize);
        stbi_image_free(imageData);
        if (result != VK_SUCCESS)
            return false;

        VkExtent3D imageExtent{
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                1
        };
        const VkFormat fmt = textureHDR ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R8G8B8A8_SRGB;
        result = this->create(VK_IMAGE_TYPE_2D, fmt, imageExtent, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (result != VK_SUCCESS)
            return false;
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(mApp->device, commandPool);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = mImage;
        barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);

        VkBufferImageCopy region = {};
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.imageExtent = imageExtent;

        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.getBuffer(), mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1, &region);

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);

        endSingleTimeCommands(mApp->device, commandPool, graphicsQueue, commandBuffer);
        return true;
    }

    VkResult
    Texture::createImageView(VkImageViewType viewType, VkFormat format, VkImageSubresourceRange subresourceRange) {
        VkImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = nullptr;
        imageViewCreateInfo.viewType = viewType;
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.subresourceRange = subresourceRange;
        imageViewCreateInfo.image = mImage;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                                          VK_COMPONENT_SWIZZLE_A};
        return vkCreateImageView(mApp->device, &imageViewCreateInfo, nullptr, &mImageView);
    }

    VkResult Texture::createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode,
                                    VkSamplerAddressMode addressMode) {
        VkSamplerCreateInfo samplerCreateInfo;
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.pNext = nullptr;
        samplerCreateInfo.flags = 0;
        samplerCreateInfo.magFilter = magFilter;
        samplerCreateInfo.minFilter = minFilter;
        samplerCreateInfo.mipmapMode = mipmapMode;
        samplerCreateInfo.addressModeU = addressMode;
        samplerCreateInfo.addressModeV = addressMode;
        samplerCreateInfo.addressModeW = addressMode;
        samplerCreateInfo.mipLodBias = 0;
        samplerCreateInfo.anisotropyEnable = VK_FALSE;
        samplerCreateInfo.maxAnisotropy = 1;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.minLod = 0;
        samplerCreateInfo.maxLod = 0;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        return vkCreateSampler(mApp->device, &samplerCreateInfo, nullptr, &mSampler);
    }

    VkFormat Texture::getFormat() const {
        return mFormat;
    }

    VkImage Texture::getImage() const {
        return mImage;
    }

    VkImageView Texture::getImageView() const {
        return mImageView;
    }

    VkSampler Texture::getSampler() const {
        return mSampler;
    }

    static
    bool hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void Texture::transition(VkImageLayout oldLayout, VkImageLayout newLayout, VkQueue graphicsQueue,
                             VkCommandPool commandPool) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(mApp->device, commandPool);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = mImage;
        //    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            fprintf(stdout, "Transition to depth\n");
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (hasStencilComponent(mFormat)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );

        endSingleTimeCommands(mApp->device, commandPool, graphicsQueue, commandBuffer);
    }
}
