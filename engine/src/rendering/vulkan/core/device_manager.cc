#include <spear/rendering/vulkan/core/device_manager.hh>

#include <iostream>
#include <vector>

namespace spear::rendering::vulkan
{

void DeviceManager::initialize(VkInstance instance, VkSurfaceKHR surface)
{
    // Enumerate physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // Find a suitable physical device
    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device, surface))
        {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    // Create logical device
    uint32_t queueFamilyIndex = findQueueFamilies(m_physicalDevice, surface);

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    const char* deviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pNext = nullptr;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(std::size(deviceExtensions));
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &m_presentQueue);
    createCommandPool(queueFamilyIndex);

    std::cout << "Device and queues initialized successfully!" << std::endl;
}

void DeviceManager::cleanup()
{
    if (m_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_device, nullptr);
    }
}

bool DeviceManager::isDeviceSuitable(VkPhysicalDevice, VkSurfaceKHR)
{
    // Assume all devices are suitable.
    return true;
}

uint32_t DeviceManager::findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR)
{
    // Assume queue family index 0 is suitable.
    return 0;
}

void DeviceManager::createCommandPool(uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_device, &poolCreateInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool.");
    }
}

} // namespace spear::rendering::vulkan
