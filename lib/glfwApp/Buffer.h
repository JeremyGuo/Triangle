//
// Created by Jeremy Guo on 2022/3/2.
//

#ifndef TRIANGLE_BUFFER_H
#define TRIANGLE_BUFFER_H

#include <common.h>

class Buffer {
public:
    Buffer(VkDevice device, VkCommandPool commandPool, VkQueue commandQueue);
    Buffer(const VkDevice& device) = delete;

    VkResult        create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties);
    void            Destroy();

    void*           Map(VkDeviceSize size = UINT64_MAX, VkDeviceSize offset = 0) const;
    void            Unmap() const;

    bool            UploadData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0) const;

    VkBuffer getBuffer();
    VkDeviceSize size();
protected:
    VkDevice mDevice;
    VkBuffer mBuffer;
    VkDeviceMemory mDeviceMemory;
    VkDeviceSize mSize;

    VkCommandPool mCommandPool;
    VkQueue mCommandQueue;
};


#endif //TRIANGLE_BUFFER_H
