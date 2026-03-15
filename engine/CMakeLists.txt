cmake_minimum_required(VERSION 3.27)
project(spear)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Vulkan REQUIRED)
find_package(Bullet REQUIRED)
find_package(glslang REQUIRED)
find_package(SPIRV-Tools REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SHADERC REQUIRED shaderc_combined)

add_compile_options(-Wall -Wextra -pedantic -std=c++20)

set(SPEAR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE SOURCES "src/*.cc")

message(STATUS "Bullet include dirs: ${BULLET_INCLUDE_DIRS}")
message(STATUS "Bullet libraries: ${BULLET_LIBRARIES}")

include_directories(${SDL_INCLUDE_DIRS} ${SDL3_IMAGE_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS}
    ${BULLET_INCLUDE_DIRS} ${SHADERC_INCLUDE_DIRS})

add_library(spear ${SOURCES})
target_include_directories(spear PUBLIC include PUBLIC ${BULLET_INCLUDE_DIRS} PRIVATE ${VULKAN_INCLUDE_DIRS})
target_compile_definitions(spear PUBLIC SPEAR_ROOT="${SPEAR_ROOT}")

target_link_libraries(spear
    PRIVATE SDL3::SDL3 SDL3_image::SDL3_image
    PRIVATE OpenGL::GL GLEW::GLEW
    PRIVATE Vulkan::Vulkan
    PRIVATE ${BULLET_LIBRARIES}
    PRIVATE ${SHADERC_LIBRARIES}
    PRIVATE glslang
    PRIVATE SPIRV-Tools SPIRV-Tools-opt
)
