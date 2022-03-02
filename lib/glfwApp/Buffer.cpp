//
// Created by Jeremy Guo on 2022/3/2.
//

#include <Buffer.h>
#include <common.h>
#include <glfwApp.h>

namespace glfw {
    Buffer::Buffer(glfw::glfwApp *app) {
        mApp = app;
        mSize = 0;
        mBuffer = VK_NULL_HANDLE;
    }

    VkDeviceSize Buffer::size() {
        return mSize;
    }

    VkBuffer Buffer::getBuffer() {
        return mBuffer;
    }

    VkResult Buffer::create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties) {
        if (mBuffer != VK_NULL_HANDLE) {
            throw std::runtime_error("Buffer can only be created once.");
        }
        VkResult result = VK_SUCCESS;

        VkBufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.pNext = nullptr;
        bufferCreateInfo.flags = 0;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCreateInfo.queueFamilyIndexCount = 0;
        bufferCreateInfo.pQueueFamilyIndices = nullptr;

        mSize = size;

        result = vkCreateBuffer(mApp->device, &bufferCreateInfo, nullptr, &mBuffer);
        if (VK_SUCCESS == result) {
            VkMemoryRequirements memoryRequirements;
            vkGetBufferMemoryRequirements(mApp->device, mBuffer, &memoryRequirements);

            VkMemoryAllocateInfo memoryAllocateInfo;
            memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memoryAllocateInfo.pNext = nullptr;
            memoryAllocateInfo.allocationSize = memoryRequirements.size;
            memoryAllocateInfo.memoryTypeIndex = getMemoryType(memoryRequirements, memoryProperties);

            VkMemoryAllocateFlagsInfo allocationFlags = {};
            allocationFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
            allocationFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
            if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                memoryAllocateInfo.pNext = &allocationFlags;
            }

            result = vkAllocateMemory(mApp->device, &memoryAllocateInfo, nullptr, &mDeviceMemory);
            if (VK_SUCCESS != result) {
                vkDestroyBuffer(mApp->device, mBuffer, nullptr);
                mBuffer = VK_NULL_HANDLE;
                mDeviceMemory = VK_NULL_HANDLE;
            } else {
                result = vkBindBufferMemory(mApp->device, mBuffer, mDeviceMemory, 0);
                if (VK_SUCCESS != result) {
                    vkDestroyBuffer(mApp->device, mBuffer, nullptr);
                    vkFreeMemory(mApp->device, mDeviceMemory, nullptr);
                    mBuffer = VK_NULL_HANDLE;
                    mDeviceMemory = VK_NULL_HANDLE;
                }
            }
        }

        return result;
    }

    uint32_t Buffer::getMemoryType(VkMemoryRequirements &memoryRequiriments, VkMemoryPropertyFlags memoryProperties) {
        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(mApp->physicalDevice, &properties);

        uint32_t result = -1;
        for (uint32_t memoryTypeIndex = 0; memoryTypeIndex < VK_MAX_MEMORY_TYPES; ++memoryTypeIndex) {
            if (memoryRequiriments.memoryTypeBits & (1 << memoryTypeIndex)) {
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

    void Buffer::destroy() {
        if (mBuffer) {
            vkDestroyBuffer(mApp->device, mBuffer, nullptr);
            mBuffer = VK_NULL_HANDLE;
        }
        if (mDeviceMemory) {
            vkFreeMemory(mApp->device, mDeviceMemory, nullptr);
            mDeviceMemory = VK_NULL_HANDLE;
        }
    }

    void *Buffer::map(VkDeviceSize size, VkDeviceSize offset) const {
        if (!mDeviceMemory)
            throw std::runtime_error("Buffer: Device Memory is not created.");

        void *mem = nullptr;
        if (offset >= mSize)
            return nullptr;
        if (offset + size > mSize)
            size = mSize - offset;
        VkResult result = vkMapMemory(mApp->device, mDeviceMemory, offset, size, 0, &mem);
        if (result != VK_SUCCESS)
            mem = nullptr;
        return mem;
    }

    void Buffer::unmap() const {
        vkUnmapMemory(mApp->device, mDeviceMemory);
    }

    VkResult Buffer::uploadData(const void *data, VkDeviceSize size, VkDeviceSize offset) const {
        void *mem = this->map(size, offset);
        if (mem) {
            std::memcpy(mem, data, size);
            this->unmap();
        } else
            return VK_ERROR_UNKNOWN;
        return VK_SUCCESS;
    }

    void Buffer::copyTo(Buffer &dst, VkCommandPool commandPool, VkQueue graphicsQueue, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(mApp->device, commandPool);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin command buffer");
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, mBuffer, dst.getBuffer(), 1, &copyRegion);

        endSingleTimeCommands(mApp->device, commandPool, graphicsQueue, commandBuffer);
    }

    Buffer::~Buffer() {
        this->destroy();
    }
}
