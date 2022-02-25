cmake_minimum_required(VERSION 3.20.1)

project(Triangle)
set(CMAKE_CXX_STANDARD 17)
set(SHADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
add_subdirectory(lib/glfwApp)
add_subdirectory(shaders)

include_directories(include)

add_executable(triangle src/triangle.cpp)
target_link_libraries(triangle PUBLIC glfwApp)
target_shader(triangle triangle vert)
target_shader(triangle triangle frag)

add_executable(vertbuf src/vertbuf.cpp)
target_link_libraries(vertbuf PUBLIC glfwApp)
target_shader(vertbuf vertbuf vert)
target_shader(vertbuf vertbuf frag)

add_executable(unibuf src/unibuf.cpp)
target_link_libraries(unibuf PUBLIC glfwApp)
target_shader(unibuf unibuf vert)
target_shader(unibuf unibuf frag)

add_executable(texbuf src/texbuf.cpp)
target_link_libraries(texbuf PUBLIC glfwApp)
target_shader(texbuf texbuf vert)
target_shader(texbuf texbuf frag)

add_executable(depth src/depth.cpp)
target_link_libraries(depth PUBLIC glfwApp)
target_shader(depth depth vert)
target_shader(depth depth frag)

add_executable(object src/object.cpp)
target_link_libraries(object PUBLIC glfwApp)
target_shader(object object vert)
target_shader(object object frag)
