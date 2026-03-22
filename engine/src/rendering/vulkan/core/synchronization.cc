#include <spear/rendering/vulkan/core/synchronization.hh>

#include <iostream>

namespace spear::rendering::vulkan
{

void Synchronization::initialize(VkDevice device, uint32_t framesInFlight, uint32_t swapchainImageCount)
{
    m_device = device;
    m_framesInFlight = framesInFlight;
    m_swapchainImageCount = swapchainImageCount;

    m_imageAvailableSemaphores.resize(framesInFlight);
    m_renderFinishedSemaphores.resize(framesInFlight);
    m_inFlightFences.resize(framesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < framesInFlight; ++i)
    {
        m_renderFinishedSemaphores[i].resize(swapchainImageCount);

        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image available semaphore!");
        }

        if (vkCreateFence(device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create in-flight fence!");
        }

        for (uint32_t j = 0; j < swapchainImageCount; ++j)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i][j]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create render finished semaphore!");
            }
        }
    }
    std::cout << "Synchronization objects created successfully!" << std::endl;
}

void Synchronization::cleanup(VkDevice device)
{
    for (uint32_t i = 0; i < m_framesInFlight; ++i)
    {
        vkDestroySemaphore(device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, m_inFlightFences[i], nullptr);

        for (uint32_t j = 0; j < m_swapchainImageCount; ++j)
        {
            vkDestroySemaphore(device, m_renderFinishedSemaphores[i][j], nullptr);
        }
    }

    m_imageAvailableSemaphores.clear();
    m_renderFinishedSemaphores.clear();
    m_inFlightFences.clear();
}

} // namespace spear::rendering::vulkan
