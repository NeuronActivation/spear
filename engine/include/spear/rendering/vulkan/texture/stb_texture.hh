#ifndef SPEAR_RENDERING_VULKAN_TEXTURE_STB_TEXTURE_HH
#define SPEAR_RENDERING_VULKAN_TEXTURE_STB_TEXTURE_HH

#include <spear/rendering/vulkan/texture/texture.hh>

namespace spear::rendering::vulkan
{

class STBTexture : public Texture
{
public:
    /// Constructor.
    STBTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

    void loadFromFile(const std::string& filePath);

private:
    void createTextureImage(const void* pixelData, VkDeviceSize imageSize);
    void createImageView();
    void createSampler();

    void createBuffer(VkDeviceSize size,
                      VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);

    void transitionImageLayout(VkImage image,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
    VkPhysicalDevice m_physicalDevice;
    VkCommandPool m_commandPool;
    VkQueue m_graphicsQueue;
};

} // namespace spear::rendering::vulkan

#endif
