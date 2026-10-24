#ifndef SPEAR_RENDERING_VULKAN_FRAME_CONTEXT_HH
#define SPEAR_RENDERING_VULKAN_FRAME_CONTEXT_HH

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <vector>

namespace spear::rendering::vulkan
{

/// A blended draw deferred until all opaque geometry is drawn. Uses the textured pipeline layout.
struct TransparentDraw
{
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    uint32_t vertexCount = 0;
    VkDescriptorSet textureSet = VK_NULL_HANDLE;
    glm::mat4 mvp{1.0f};
    glm::vec4 params{0.0f}; ///< Second push constant of the textured pipeline (x view model, y alpha cutoff).
    float depth = 0.0f;     ///< View-space distance used to draw back to front.
};

// Set by the renderer at the start of each render pass, cleared at the end.
// Vulkan shapes read this in render(Camera&) to bind their pipeline and record draws.
struct FrameContext
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    bool shadowPass = false;
    bool viewModel = false;
    glm::mat4 lightViewProjection{1.0f};
    VkPipeline shadowColorPipeline = VK_NULL_HANDLE;
    VkPipeline shadowTexturedPipeline = VK_NULL_HANDLE;
    VkPipelineLayout shadowPipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet sunDescriptorSet = VK_NULL_HANDLE;

    // Color (untextured) pipeline — used by Cube.
    VkPipeline colorPipeline = VK_NULL_HANDLE;
    VkPipelineLayout colorPipelineLayout = VK_NULL_HANDLE;

    // Textured pipeline — used by TexturedCube.
    VkPipeline texturedPipeline = VK_NULL_HANDLE;
    VkPipelineLayout texturedPipelineLayout = VK_NULL_HANDLE;
    VkPipeline texturedDoubleSidedPipeline = VK_NULL_HANDLE;
    VkPipeline transparentPipeline = VK_NULL_HANDLE;
    VkPipeline transparentDoubleSidedPipeline = VK_NULL_HANDLE;

    /// Filled while the scene draws, then sorted and drawn by the renderer.
    std::vector<TransparentDraw> transparentDraws;

    // UI pipeline — used by UI elements.
    VkPipeline uiPipeline = VK_NULL_HANDLE;
    VkPipelineLayout uiPipelineLayout = VK_NULL_HANDLE;
};

extern FrameContext g_frameContext;

} // namespace spear::rendering::vulkan

#endif
