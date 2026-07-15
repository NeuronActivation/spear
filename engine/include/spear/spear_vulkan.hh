#ifndef SPEAR_SPEAR_VULKAN_HH
#define SPEAR_SPEAR_VULKAN_HH

#include <spear/spear.hh>

// Vulkan windowing
#include <spear/window/vulkan_window.hh>

// Vulkan rendering core
#include <spear/rendering/vulkan/core/command_buffer_manager.hh>
#include <spear/rendering/vulkan/core/device_manager.hh>
#include <spear/rendering/vulkan/core/framebuffer_manager.hh>
#include <spear/rendering/vulkan/core/pipeline_manager.hh>
#include <spear/rendering/vulkan/core/render_pass_manager.hh>
#include <spear/rendering/vulkan/core/swapchain.hh>
#include <spear/rendering/vulkan/core/synchronization.hh>

// Vulkan rendering
#include <spear/rendering/vulkan/frame_context.hh>
#include <spear/rendering/vulkan/mesh.hh>
#include <spear/rendering/vulkan/model/obj_model.hh>
#include <spear/rendering/vulkan/renderer.hh>
#include <spear/rendering/vulkan/shader.hh>
#include <spear/rendering/vulkan/shapes/cube.hh>
#include <spear/rendering/vulkan/shapes/textured_cube.hh>
#include <spear/rendering/vulkan/sprite_3d.hh>
#include <spear/rendering/vulkan/texture/stb_texture.hh>
#include <spear/rendering/vulkan/texture/texture.hh>

// Vulkan UI
#include <spear/rendering/vulkan/ui/menu_list.hh>
#include <spear/rendering/vulkan/ui/quad_2d.hh>
#include <spear/rendering/vulkan/ui/text.hh>
#include <spear/rendering/vulkan/ui/ui_renderer.hh>

#endif
