#include <spear/rendering/vulkan/texture/texture.hh>

#include <iostream>
#include <stdexcept>

namespace spear::rendering::vulkan
{

Texture::Texture()
    : BaseTexture(),
      m_device(VK_NULL_HANDLE),
      m_image(VK_NULL_HANDLE),
      m_imageMemory(VK_NULL_HANDLE),
      m_imageView(VK_NULL_HANDLE),
      m_sampler(VK_NULL_HANDLE),
      m_descriptorSet(VK_NULL_HANDLE)
{
}

Texture::Texture(Texture&& other)
    : BaseTexture(std::move(other)),
      m_device(other.m_device),
      m_image(other.m_image),
      m_imageMemory(other.m_imageMemory),
      m_imageView(other.m_imageView),
      m_sampler(other.m_sampler),
      m_descriptorSet(other.m_descriptorSet)
{
    other.m_image = VK_NULL_HANDLE;
    other.m_imageView = VK_NULL_HANDLE;
    other.m_sampler = VK_NULL_HANDLE;
    other.m_descriptorSet = VK_NULL_HANDLE;
}

Texture& Texture::operator=(Texture&& other)
{
    if (this != &other)
    {
        BaseTexture::operator=(std::move(other));
        m_device = other.m_device;
        m_image = other.m_image;
        m_imageMemory = other.m_imageMemory;
        m_imageView = other.m_imageView;
        m_sampler = other.m_sampler;
        m_descriptorSet = other.m_descriptorSet;

        other.m_image = VK_NULL_HANDLE;
        other.m_imageView = VK_NULL_HANDLE;
        other.m_sampler = VK_NULL_HANDLE;
        other.m_descriptorSet = VK_NULL_HANDLE;
    }
    return *this;
}

Texture::~Texture()
{
    cleanup();
}

void Texture::cleanup()
{
    if (m_imageView)
        vkDestroyImageView(m_device, m_imageView, nullptr);
    if (m_image)
        vkDestroyImage(m_device, m_image, nullptr);
    if (m_imageMemory)
        vkFreeMemory(m_device, m_imageMemory, nullptr);
    if (m_sampler)
        vkDestroySampler(m_device, m_sampler, nullptr);
    // Descriptor sets are freed implicitly when the pool is reset/destroyed.
}

void Texture::createDescriptorSet(VkDescriptorPool pool,
                                  VkDescriptorSetLayout layout,
                                  VkImageView imageView,
                                  VkSampler sampler)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("Texture: failed to allocate descriptor set!");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_descriptorSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
}

void Texture::bindDescriptorSet(VkCommandBuffer cmd,
                                VkPipelineLayout pipelineLayout,
                                uint32_t set) const
{
    if (m_descriptorSet == VK_NULL_HANDLE)
        return;

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, set, 1, &m_descriptorSet, 0, nullptr);
}

VkDescriptorPool Texture::createDescriptorPool(VkDevice device, uint32_t maxSets)
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = maxSets;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = maxSets;

    VkDescriptorPool pool;
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS)
        throw std::runtime_error("Texture: failed to create descriptor pool!");
    return pool;
}

VkDescriptorSetLayout Texture::createDescriptorSetLayout(VkDevice device)
{
    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 1;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerBinding;

    VkDescriptorSetLayout layout;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
        throw std::runtime_error("Texture: failed to create descriptor set layout!");
    return layout;
}

void Texture::bind(uint32_t unit) const
{
    std::cout << "Not allowed in Vulkan" << std::endl;
}

void Texture::unbind(uint32_t unit)
{
    std::cout << "Not allowed in Vulkan" << std::endl;
}

} // namespace spear::rendering::vulkan
