#include <spear/rendering/vulkan/core/framebuffer_manager.hh>

#include <array>
#include <iostream>

namespace spear::rendering::vulkan
{

void FramebufferManager::initialize(VkDevice device, VkRenderPass render_pass, const std::vector<VkImageView>& image_views, const std::vector<VkImageView>& depth_image_views, VkExtent2D extent)
{
    m_frameBuffers.resize(image_views.size());

    for (size_t i = 0; i < image_views.size(); i++)
    {
        std::array<VkImageView, 2> attachments = {image_views[i], depth_image_views[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = render_pass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_frameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }

    std::cout << "Framebuffers created succesfully" << std::endl;
}

void FramebufferManager::cleanup(VkDevice device)
{
    for (auto framebuffer : m_frameBuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    m_frameBuffers.clear();
}

} // namespace spear::rendering::vulkan
