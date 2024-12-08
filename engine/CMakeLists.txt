cmake_minimum_required(VERSION 3.22)
project(spear)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Vulkan REQUIRED)

add_compile_options(-Wall -Wextra -pedantic -std=c++20)

set(SPEAR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
add_compile_definitions(SPEAR_ROOT="${SPEAR_ROOT}")

set(SOURCES
    #src/material.cc

    src/window.cc
    src/transform.cc
    src/event_handler.cc
    src/mesh.cc
    src/camera.cc
    src/sprite3d.cc

    # Shapes
    #src/shapes/cube.cc
    src/shapes/shape.cc
    src/shapes/quad.cc

    # Rendering
    src/rendering/api.cc
    src/rendering/base_shader.cc
    src/rendering/base_texture.cc
    src/rendering/shader_type.cc
    src/rendering/opengl/renderer.cc
    src/rendering/opengl/shader.cc
    src/rendering/opengl/texture.cc
    src/rendering/vulkan/renderer.cc
        include/spear/rendering/vulkan/error.hh
)

include_directories(${SDL_INCLUDE_DIRS} ${SDL3_IMAGE_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${VULKAN_INCLUDE_DIRS})

add_library(spear ${SOURCES}
        include/spear/rendering/vulkan/error.hh)
target_include_directories(spear PUBLIC include)
target_link_libraries(spear
    PRIVATE SDL3::SDL3 SDL3_image::SDL3_image
    PRIVATE OpenGL::GL GLEW::GLEW Vulkan
)
