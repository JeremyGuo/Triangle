cmake_minimum_required(VERSION 3.20.1)

project(Triangle)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(lib/glfwApp)

include_directories(include)
add_executable(main src/main.cpp)
target_link_libraries(main PUBLIC glfwApp)
