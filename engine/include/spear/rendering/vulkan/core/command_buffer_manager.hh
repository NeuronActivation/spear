#ifndef SPEAR_RENDERING_VULKAN_FRAMEBUFFER_MANAGER_HH
#define SPEAR_RENDERING_VULKAN_FRAMEBUFFER_MANAGER_HH

#include <vulkan/vulkan.h>

#include <vector>

namespace spear::rendering::vulkan
{

class CommandBufferManager
{
public:
    void initialize(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount);
    void cleanup(VkDevice device, VkCommandPool commandPool);

    void beginCommandBuffer(uint32_t image_index);
    void endCommandBuffer(VkCommandBuffer commandBuffer);

    const std::vector<VkCommandBuffer>& getCommandBuffers() const
    {
        return m_commandBuffers;
    }

private:
    std::vector<VkCommandBuffer> m_commandBuffers;
};
} // namespace spear::rendering::vulkan

#endif
