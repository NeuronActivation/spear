#include <spear/rendering/vulkan/core/command_buffer_manager.hh>

#include <iostream>

namespace spear::rendering::vulkan
{

void CommandBufferManager::initialize(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount)
{
    m_commandBuffers.resize(commandBufferCount);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBufferCount;

    if (vkAllocateCommandBuffers(device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    std::cout << "Command buffers allocated successfully!" << std::endl;
}

void CommandBufferManager::beginCommandBuffer(uint32_t image_index)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    if (vkBeginCommandBuffer(m_commandBuffers[image_index], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
}

void CommandBufferManager::endCommandBuffer(VkCommandBuffer commandBuffer)
{
    VkResult result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to end command buffer!");
    }
}

void CommandBufferManager::cleanup(VkDevice device, VkCommandPool commandPool)
{
    vkDeviceWaitIdle(device);
    if (!m_commandBuffers.empty())
    {
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        m_commandBuffers.clear();
    }
}

} // namespace spear::rendering::vulkan
