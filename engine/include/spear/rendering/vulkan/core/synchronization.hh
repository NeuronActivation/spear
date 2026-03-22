#ifndef SPEAR_RENDERING_VULKAN_SYNCHRONIZATION_HH
#define SPEAR_RENDERING_VULKAN_SYNCHRONIZATION_HH

#include <vulkan/vulkan.h>

#include <vector>

namespace spear::rendering::vulkan
{

class Synchronization
{
public:
    void initialize(VkDevice device, uint32_t framesInFlight, uint32_t swapchainImageCount);
    void cleanup(VkDevice device);

    VkSemaphore getImageAvailableSemaphore(uint32_t index) const
    {
        return m_imageAvailableSemaphores[index];
    }
    VkSemaphore getRenderFinishedSemaphore(uint32_t frameIndex, uint32_t imageIndex) const
    {
        return m_renderFinishedSemaphores[frameIndex][imageIndex];
    }
    VkFence getInFlightFence(uint32_t index) const
    {
        return m_inFlightFences[index];
    }

private:
    VkDevice m_device;
    uint32_t m_framesInFlight;
    uint32_t m_swapchainImageCount;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<std::vector<VkSemaphore>> m_renderFinishedSemaphores;
};

} // namespace spear::rendering::vulkan

#endif
