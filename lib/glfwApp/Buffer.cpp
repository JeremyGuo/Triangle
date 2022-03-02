//
// Created by Jeremy Guo on 2022/3/2.
//

#include "Buffer.h"

Buffer::Buffer(VkDevice device, VkCommandPool commandPool, VkQueue commandQueue) {
    mDevice = device;
    mCommandPool = commandPool;
    mCommandQueue = commandQueue;

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

    result = vkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, &mBuffer);
    if (VK_SUCCESS == result) {
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(mDevice, mBuffer, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo;
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.pNext = nullptr;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = GetMemoryType(memoryRequirements, memoryProperties);

        VkMemoryAllocateFlagsInfo allocationFlags = {};
        allocationFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        allocationFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            memoryAllocateInfo.pNext = &allocationFlags;
        }

        result = vkAllocateMemory(mDevice, &memoryAllocateInfo, nullptr, &mDeviceMemory);
        if (VK_SUCCESS != result) {
            vkDestroyBuffer(mDevice, mBuffer, nullptr);
            mBuffer = VK_NULL_HANDLE;
            mDeviceMemory = VK_NULL_HANDLE;
        } else {
            result = vkBindBufferMemory(mDevice, mBuffer, mDeviceMemory, 0);
            if (VK_SUCCESS != result) {
                vkDestroyBuffer(mDevice, mBuffer, nullptr);
                vkFreeMemory(mDevice, mDeviceMemory, nullptr);
                mBuffer = VK_NULL_HANDLE;
                mDeviceMemory = VK_NULL_HANDLE;
            }
        }
    }

    return result;
}
