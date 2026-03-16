#ifndef SPEAR_RENDERING_VULKAN_TEXTURE_TEXTURE_HH
#define SPEAR_RENDERING_VULKAN_TEXTURE_TEXTURE_HH

#include <spear/rendering/base_texture.hh>

#include <vulkan/vulkan.h>

namespace spear::rendering::vulkan
{

class Texture : public BaseTexture
{
public:
    /// Default constructor.
    Texture();

    /// Destructor.
    ~Texture() override;

    /// Move constructor.
    Texture(Texture&& other);

    /// Move assignment operator.
    Texture& operator=(Texture&& other);

    /// Deleted copy constructor.
    Texture(const Texture& other) = delete;

    /// Deleted copy assignment operator.
    Texture& operator=(const Texture& other) = delete;

    void bind(uint32_t unit = 0) const override;
    void unbind(uint32_t unit = 0) override;

    VkImageView getImageView() const
    {
        return m_imageView;
    }
    VkSampler getSampler() const
    {
        return m_sampler;
    }

    void setDevice(VkDevice device)
    {
        m_device = device;
    }

    VkDevice getDevice()
    {
        return m_device;
    }

    /// Create a descriptor pool that holds \p maxSets combined-image-sampler descriptors.
    static VkDescriptorPool createDescriptorPool(VkDevice device, uint32_t maxSets);

    /// Create a descriptor set layout with a single combined-image-sampler at binding 0
    /// (fragment stage).
    static VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device);

    /// Allocate a descriptor set from \p pool and write a combined-image-sampler
    /// at binding 0 using the provided \p imageView and \p sampler.
    void createDescriptorSet(VkDescriptorPool pool,
                             VkDescriptorSetLayout layout,
                             VkImageView imageView,
                             VkSampler sampler);

    /// Bind the descriptor set at the given \p set index.
    void bindDescriptorSet(VkCommandBuffer cmd,
                           VkPipelineLayout pipelineLayout,
                           uint32_t set = 0) const;

protected:
    void cleanup();

protected:
    VkDevice m_device;
    VkImage m_image;
    VkDeviceMemory m_imageMemory;
    VkImageView m_imageView;
    VkSampler m_sampler;
    VkDescriptorSet m_descriptorSet;
};

} // namespace spear::rendering::vulkan

#endif
