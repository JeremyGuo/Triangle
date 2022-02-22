//
// Created by JeremyGuo on 2022/2/21.
//

#ifndef TRIANGLE_GLFWAPP_H
#define TRIANGLE_GLFWAPP_H

#endif //TRIANGLE_GLFWAPP_H

#include "common.h"

namespace glfw {
    static
    const std::vector<const char*> vkDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const;
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class glfwApp {
    public:
        bool framebufferResized = false;

        glfwApp();
        virtual ~glfwApp();
        glfwApp(const glfwApp&) = delete;

        virtual void initialize();
        virtual void cleanup();
        void run();
    protected:
        void initWindow();

        void initVulkan();
        void initVulkanInst();
        void initVulkanDevice();
        void initSurface();
        void initSwapChain();

        virtual void recreateSwapChain();
        virtual void cleanupSwapChain();

        virtual void onDraw() = 0;
        virtual void onUpdate() = 0;

        static int rateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface);
        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

        int width = 800;
        int height = 600;

        GLFWwindow* window;

        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;

        VkQueue graphicsQueue{};
        VkQueue presentQueue{};

        VkSurfaceKHR surface{};
        VkSwapchainKHR swapChain{};
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent{};
        std::vector<VkImageView> swapChainImageViews;
    };
}
