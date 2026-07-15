cmake_minimum_required(VERSION 3.27)
project(spear-application-vulkan)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../../engine engine_dir)
add_executable(spear-application-vulkan main.cc)
target_link_libraries(spear-application-vulkan spear)

