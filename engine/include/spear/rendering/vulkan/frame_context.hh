#ifndef SPEAR_RENDERING_VULKAN_FRAME_CONTEXT_HH
#define SPEAR_RENDERING_VULKAN_FRAME_CONTEXT_HH

#include <vulkan/vulkan.h>

namespace spear::rendering::vulkan
{

// Set by the renderer at the start of each render pass, cleared at the end.
// Vulkan shapes read this in render(Camera&), mirroring OpenGL's implicit context.
struct FrameContext
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
};

extern FrameContext g_frameContext;

} // namespace spear::rendering::vulkan

#endif
