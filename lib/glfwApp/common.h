//
// Created by JeremyGuo on 2022/2/21.
//

#ifndef TRIANGLE_COMMON_H
#define TRIANGLE_COMMON_H

#include <exception>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <cstdint>
#include <array>
#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "stb_image.h"

static
void printException(const std::exception& e, int level =  0) {
    std::cout << std::string(level, '\t') << e.what() << '\n';
    try {
        std::rethrow_if_nested(e);
    } catch(const std::exception& nestedException) {
        printException(nestedException, level+1);
    } catch(...) {}
}

static
std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

static
VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        std::throw_with_nested(std::runtime_error("failed to create shader module!"));
    }
    return shaderModule;
}

#endif //TRIANGLE_COMMON_H
