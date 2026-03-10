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
add_compile_definitions(SPEAR_ROOT="${SPEAR_ROOT}")

set(SOURCES
    #src/material.cc
    #src/sprite3d.cc

    src/time.cc
    src/util.cc
    src/transform.cc
    src/event_handler.cc
    src/game_object.cc
    src/camera.cc
    src/movement_controller.cc
    src/scene_manager.cc
    src/scene.cc
    src/create_scene.cc
    src/mesh.cc

    # Window
    src/window/base_window.cc
    src/window/opengl_window.cc
    src/window/vulkan_window.cc

    # Physics
    src/physics/constants.cc
    src/physics/physics_object.cc

    # Physics bullet
    src/physics/bullet/object_data.cc
    src/physics/bullet/object.cc
    src/physics/bullet/world.cc

    # Model loaders
    src/model/obj_loader.cc

    # Rendering
    src/rendering/api.cc
    src/rendering/base_shader.cc
    src/rendering/base_texture.cc
    src/rendering/base_model.cc
    src/rendering/base_renderer.cc
    src/rendering/shader_type.cc

    # Rendering Shapes
    src/rendering/shapes/shape.cc
    src/rendering/shapes/textured_shape.cc

    # Rendering OpenGL
    src/rendering/opengl/renderer.cc
    src/rendering/opengl/shader.cc

    # Rendering OpenGL Texture
    src/rendering/opengl/texture/texture.cc
    src/rendering/opengl/texture/sdl_texture.cc
    src/rendering/opengl/texture/stb_texture.cc

    # Rendering OpenGL Shapes
    src/rendering/opengl/shapes/cube.cc
    src/rendering/opengl/shapes/quad.cc
    src/rendering/opengl/shapes/sphere.cc

    # Rendering OpenGL Model
    src/rendering/opengl/model/obj_model.cc

    # Rendering Vulkan
    src/rendering/vulkan/renderer.cc
    src/rendering/vulkan/mesh.cc
    src/rendering/vulkan/frame_context.cc

    # Rendering Vulkan Shapes
    src/rendering/vulkan/shapes/cube.cc

    # Rendering Vulkan core
    src/rendering/vulkan/core/command_buffer_manager.cc
    src/rendering/vulkan/core/framebuffer_manager.cc
    src/rendering/vulkan/core/render_pass_manager.cc
    src/rendering/vulkan/core/synchronization.cc
    src/rendering/vulkan/core/device_manager.cc
    src/rendering/vulkan/core/pipeline_manager.cc
    src/rendering/vulkan/core/swapchain.cc

    # Rendering Vulkan Texture
    src/rendering/vulkan/texture/texture.cc
    src/rendering/vulkan/texture/stb_texture.cc
)

message(STATUS "Bullet include dirs: ${BULLET_INCLUDE_DIRS}")
message(STATUS "Bullet libraries: ${BULLET_LIBRARIES}")

include_directories(${SDL_INCLUDE_DIRS} ${SDL3_IMAGE_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS}
    ${BULLET_INCLUDE_DIRS} ${SHADERC_INCLUDE_DIRS})

add_library(spear ${SOURCES})
target_include_directories(spear PUBLIC include PUBLIC ${BULLET_INCLUDE_DIRS} PRIVATE ${VULKAN_INCLUDE_DIRS})

target_link_libraries(spear
    PRIVATE SDL3::SDL3 SDL3_image::SDL3_image
    PRIVATE OpenGL::GL GLEW::GLEW
    PRIVATE Vulkan::Vulkan
    PRIVATE ${BULLET_LIBRARIES}
    PRIVATE ${SHADERC_LIBRARIES}
    PRIVATE glslang
    PRIVATE SPIRV-Tools SPIRV-Tools-opt
)
