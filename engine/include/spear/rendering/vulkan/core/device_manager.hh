#ifndef SPEAR_RENDERING_VULKAN_DEVICE_MANAGER_HH
#define SPEAR_RENDERING_VULKAN_DEVICE_MANAGER_HH

#include <vulkan/vulkan.h>

namespace spear::rendering::vulkan
{

class DeviceManager
{
public:
    void initialize(VkInstance instance, VkSurfaceKHR surface);
    void cleanup();

    VkDevice getDevice() const
    {
        return m_device;
    }
    VkPhysicalDevice getPhysicalDevice() const
    {
        return m_physicalDevice;
    }
    VkQueue getGraphicsQueue() const
    {
        return m_graphicsQueue;
    }
    VkQueue getPresentQueue() const
    {
        return m_presentQueue;
    }
    VkCommandPool getCommandPool() const
    {
        return m_commandPool;
    }

private:
    bool isDeviceSuitable(VkPhysicalDevice, VkSurfaceKHR);
    uint32_t findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);
    void createCommandPool(uint32_t queueFamilyIndex);

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
};

} // namespace spear::rendering::vulkan

#endif
