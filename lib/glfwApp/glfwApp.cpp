//
// Created by JeremyGuo on 2022/2/21.
//

#include "glfwApp.h"
#include <chrono>
using namespace glfw;

const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

const bool enableValidationLayers = true;

bool QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
}

glfwApp::glfwApp() {
    window = nullptr;
}

glfwApp::~glfwApp() {
}

void glfwApp::cleanup() {
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void glfwApp::initialize() {
    this->initWindow();
    this->initVulkan();
}

void glfwApp::run() {
    std::cout << "Started to run" << std::endl;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        std::chrono::high_resolution_clock::time_point thisCallUpdate = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> dur = (thisCallUpdate - lastCallUpdate);
        deltaTime = dur.count();
        lastCallUpdate = thisCallUpdate;

        this->onUpdate();
        this->onDraw();
    }
    vkDeviceWaitIdle(device);
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<glfwApp*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void glfwApp::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    glfwSetKeyCallback(window, [](GLFWwindow* wnd, int key, int scancode, int action, int mods){
        glfwApp* app = reinterpret_cast<glfwApp*>(glfwGetWindowUserPointer(wnd));
        app->onKeyDown(key, scancode, action, mods);
    });
    glfwSetMouseButtonCallback(window, [](GLFWwindow* wnd, int button, int action, int mods) {
        glfwApp* app = reinterpret_cast<glfwApp*>(glfwGetWindowUserPointer(wnd));
        app->onMouseButton(button, action, mods);
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* wnd, double x, double y) {
        glfwApp* app = reinterpret_cast<glfwApp*>(glfwGetWindowUserPointer(wnd));
        app->onMouseMove(static_cast<float>(x), static_cast<float>(y));
    });
}

void glfwApp::initVulkan() {
    try {
        this->initVulkanInst();
        this->initSurface();
        this->initVulkanDevice();
        this->initSwapChain();
    } catch (...) {
        std::throw_with_nested(std::runtime_error("Failed to initialize Vulkan"));
    }
}

void glfwApp::initVulkanInst() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pApplicationName = "Vulkan";
    appInfo.pEngineName = "No Engine";

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensionNames;
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        for (int i = 0; i < glfwExtensionCount; i ++)
            extensionNames.push_back(glfwExtensions[i]);
    }

    createInfo.enabledExtensionCount = extensionNames.size();
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    if (!enableValidationLayers) {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    } else {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
        std::throw_with_nested(std::runtime_error("failed to create instance"));
}

void glfwApp::initVulkanDevice() {
    {
        /**
         * Init Vulkan Physical device
         */
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            std::throw_with_nested(std::runtime_error("Failed to find GPU device"));
        std::cout << "Device Count: " << deviceCount << std::endl;
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        int maxScore = 0;
        for (auto &_device : devices) {
            int curScore = rateDeviceSuitability(_device, surface);
            if (curScore > maxScore) {
                std::cout << "Updated physical device" << std::endl;
                physicalDevice = _device;
                msaaSamples = getMaxUsableSampleCount();
                maxScore = curScore;
            }
        }
        if (!physicalDevice)
            std::throw_with_nested(std::runtime_error("No suitable GPU found."));
    }

    {
        /**
         * Init Vulkan Logical device
         */
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::vector<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        std::sort(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());
        uniqueQueueFamilies.erase(std::unique(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end()), uniqueQueueFamilies.end());

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(vkDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = vkDeviceExtensions.data();
        createInfo.enabledLayerCount = 0;
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            std::throw_with_nested(std::runtime_error("failed to create logical device!"));
        }
    }

    {
        /**
         * Get Queue from device
         */
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }
}

QueueFamilyIndices glfwApp::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport) == VK_SUCCESS) {
            if (presentSupport)
                indices.presentFamily = i;
        }
        if (indices.isComplete())
            break;
        i++;
    }
    return indices;
}

int glfwApp::rateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    int score = 0;
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }
    score += static_cast<int>(deviceProperties.limits.maxImageDimension2D);
    std::cout << deviceProperties.deviceName << " Supported max image dimension 2D: " << deviceProperties.limits.maxImageDimension2D << std::endl;
#ifndef __APPLE__
    if (!deviceFeatures.geometryShader) {
        std::cout << deviceProperties.deviceName << " Geometry Shader not supported" << std::endl;
        return 0;
    }
#endif
    if (!checkDeviceExtensionSupport(device)) {
        std::cout << deviceProperties.deviceName << " Extension not supported" << std::endl;
        return 0;
    }
    /** Swap chain Extension check **/
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
    if (swapChainSupport.presentModes.empty() || swapChainSupport.formats.empty()) {
        std::cout << deviceProperties.deviceName << " No valid swap chain feature" << std::endl;
        return 0;
    }
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    if (!indices.isComplete()) {
        std::cout << deviceProperties.deviceName << " No valid queue" << std::endl;
        return 0;
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    if (!supportedFeatures.samplerAnisotropy) {
        std::cout << deviceProperties.deviceName << " Device not support anisotropy sampler" << std::endl;
        return 0;
    }

    return score;
}

void glfwApp::initSurface() {
//    VkWin32SurfaceCreateInfoKHR createInfo{};
//    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
//    createInfo.hwnd = glfwGetWin32Window(window);
//    createInfo.hinstance = GetModuleHandle(nullptr);
//
//    if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
//        std::throw_with_nested(std::runtime_error("failed to create surface"));
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        std::throw_with_nested(std::runtime_error("failed to create surface"));
}

bool glfwApp::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string> requiredExtensions(vkDeviceExtensions.begin(), vkDeviceExtensions.end());
    for (const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);
    return requiredExtensions.empty();
}

SwapChainSupportDetails glfwApp::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities) != VK_SUCCESS)
        std::throw_with_nested(std::runtime_error("failed to get swap chain capability"));
    uint32_t formatCount;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr) != VK_SUCCESS)
        std::throw_with_nested(std::runtime_error("failed to get surface format"));
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }
    uint32_t presentModeCount;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr) != VK_SUCCESS)
        std::throw_with_nested(std::runtime_error("failed to get device present mode"));
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}

VkSurfaceFormatKHR glfwApp::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR glfwApp::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D glfwApp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
        };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

void glfwApp::initSwapChain() {
    try {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
            std::cout << "SwapChain: Sharing mode" << std::endl;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
            std::cout << "SwapChain: Exclusive mode" << std::endl;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // 不透明模式
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            std::throw_with_nested(std::runtime_error("failed to call vkCreateSwapChainKHR()"));
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;

        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                std::throw_with_nested(std::runtime_error("failed to create image views!"));
            }
        }
    } catch(...) {
        std::throw_with_nested(std::runtime_error("failed to create swap chain"));
    }
}

void glfwApp::cleanupSwapChain() {
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void glfwApp::recreateSwapChain() {
    vkDeviceWaitIdle(device);
    this->cleanupSwapChain();
    this->initSwapChain();
}

VkSampleCountFlagBits glfwApp::getMaxUsableSampleCount() {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

void glfwApp::onMouseButton(int button, int action, int mods) {
}

void glfwApp::onKeyDown(int key, int scancode, int action, int mods) {
}

void glfwApp::onMouseMove(float x, float y) {
}
