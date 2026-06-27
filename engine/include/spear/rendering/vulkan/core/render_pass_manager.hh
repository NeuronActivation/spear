#ifndef SPEAR_RENDERING_VULKAN_RENDER_PASS_MANAGER_HH
#define SPEAR_RENDERING_VULKAN_RENDER_PASS_MANAGER_HH

#include <vulkan/vulkan.h>

namespace spear::rendering::vulkan
{

class RenderPassManager
{
public:
    void initialize(VkDevice device, VkFormat swapchain_image_format, VkFormat depth_format = VK_FORMAT_D32_SFLOAT);
    void cleanup(VkDevice device);

    VkRenderPass getRenderPass() const
    {
        return m_renderPass;
    }

private:
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
};

} // namespace spear::rendering::vulkan

#endif
