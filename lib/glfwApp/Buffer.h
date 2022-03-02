//
// Created by Jeremy Guo on 2022/3/2.
//

#ifndef TRIANGLE_BUFFER_H
#define TRIANGLE_BUFFER_H

#include <common.h>

namespace glfw {
    class glfwApp;

    class Buffer {
    public:
        Buffer(glfw::glfwApp *app);

        Buffer(const Buffer &buffer) = delete;

        virtual ~Buffer();

        VkResult create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties);

        void destroy();

        void *map(VkDeviceSize size = UINT64_MAX, VkDeviceSize offset = 0) const;

        void unmap() const;

        VkResult uploadData(const void *data, VkDeviceSize size, VkDeviceSize offset = 0) const;

        VkBuffer getBuffer();

        VkDeviceSize size();

        void copyTo(Buffer &dst, VkCommandPool commandPool, VkQueue graphicsQueue, VkDeviceSize size);

    protected:
        glfw::glfwApp *mApp;
        VkBuffer mBuffer;
        VkDeviceMemory mDeviceMemory;
        VkDeviceSize mSize;

        uint32_t getMemoryType(VkMemoryRequirements &memoryRequiriments, VkMemoryPropertyFlags memoryProperties);
    };
}


#endif //TRIANGLE_BUFFER_H
