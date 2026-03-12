#ifndef SPEAR_RENDERING_VULKAN_FRAME_CONTEXT_HH
#define SPEAR_RENDERING_VULKAN_FRAME_CONTEXT_HH

#include <vulkan/vulkan.h>

namespace spear::rendering::vulkan
{

// Set by the renderer at the start of each render pass, cleared at the end.
// Vulkan shapes read this in render(Camera&) to bind their pipeline and record draws.
struct FrameContext
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    // Color (untextured) pipeline — used by Cube.
    VkPipeline colorPipeline = VK_NULL_HANDLE;
    VkPipelineLayout colorPipelineLayout = VK_NULL_HANDLE;

    // Textured pipeline — used by TexturedCube.
    VkPipeline texturedPipeline = VK_NULL_HANDLE;
    VkPipelineLayout texturedPipelineLayout = VK_NULL_HANDLE;
};

extern FrameContext g_frameContext;

} // namespace spear::rendering::vulkan

#endif
