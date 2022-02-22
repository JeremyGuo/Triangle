//
// Created by JeremyGuo on 2022/2/21.
//

#ifndef TRIANGLE_COMMON_H
#define TRIANGLE_COMMON_H

#include <exception>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static inline
void printException(const std::exception& e, int level =  0) {
    std::cerr << std::string(level, '\t') << e.what() << '\n';
    try {
        std::rethrow_if_nested(e);
    } catch(const std::exception& nestedException) {
        printException(nestedException, level+1);
    } catch(...) {}
}

#endif //TRIANGLE_COMMON_H
