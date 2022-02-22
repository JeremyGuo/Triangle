cmake_minimum_required(VERSION 3.20.1)

project(Triangle)
set(CMAKE_CXX_STANDARD 17)
set(SHADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
add_subdirectory(lib/glfwApp)
add_subdirectory(shaders)

include_directories(include)

add_executable(main src/main.cpp)
target_link_libraries(main PUBLIC glfwApp)
target_shader(main triangle vert)
target_shader(main triangle frag)

add_executable(vertbuf src/vertbuf.cpp)
target_link_libraries(vertbuf PUBLIC glfwApp)
target_shader(vertbuf vertbuf vert)
target_shader(vertbuf vertbuf frag)
