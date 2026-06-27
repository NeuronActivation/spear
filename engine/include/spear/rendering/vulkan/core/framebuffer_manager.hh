#ifndef SPEAR_RENDERING_FRAMEBUFFER_MANAGER_HH
#define SPEAR_RENDERING_FRAMEBUFFER_MANAGER_HH

#include <vulkan/vulkan.h>

#include <vector>

namespace spear::rendering::vulkan
{

class FramebufferManager
{
public:
    void initialize(VkDevice device, VkRenderPass render_pass, const std::vector<VkImageView>& image_views, const std::vector<VkImageView>& depth_image_views, VkExtent2D extent);
    void cleanup(VkDevice device);

    const std::vector<VkFramebuffer>& getFrameBuffers() const
    {
        return m_frameBuffers;
    }

private:
    std::vector<VkFramebuffer> m_frameBuffers;
};

} // namespace spear::rendering::vulkan

#endif
