#include <spear/rendering/vulkan/frame_context.hh>
#include <spear/ui/quad_2d.hh>

#include <cstring>
#include <stdexcept>

namespace spear::ui
{

Quad2D::Quad2D(VkDevice device,
               VkPhysicalDevice physDevice,
               VkDescriptorPool descriptorPool,
               VkDescriptorSetLayout descriptorSetLayout,
               std::shared_ptr<rendering::vulkan::Texture> texture)
    : m_device(device),
      m_descriptorPool(descriptorPool),
      m_descriptorSetLayout(descriptorSetLayout),
      m_texture(std::move(texture))
{
    createVertexBuffer(physDevice);
    createDescriptorSet();
}

Quad2D::~Quad2D()
{
    if (m_vertexBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    if (m_vertexMemory != VK_NULL_HANDLE)
        vkFreeMemory(m_device, m_vertexMemory, nullptr);
}

Quad2D::Quad2D(Quad2D&& other) noexcept
    : m_device(other.m_device),
      m_vertexBuffer(other.m_vertexBuffer),
      m_vertexMemory(other.m_vertexMemory),
      m_vertexCount(other.m_vertexCount),
      m_descriptorPool(other.m_descriptorPool),
      m_descriptorSetLayout(other.m_descriptorSetLayout),
      m_descriptorSet(other.m_descriptorSet),
      m_texture(std::move(other.m_texture)),
      m_position(other.m_position),
      m_size(other.m_size)
{
    other.m_vertexBuffer = VK_NULL_HANDLE;
    other.m_vertexMemory = VK_NULL_HANDLE;
    other.m_descriptorSet = VK_NULL_HANDLE;
}

Quad2D& Quad2D::operator=(Quad2D&& other) noexcept
{
    if (this != &other)
    {
        if (m_vertexBuffer != VK_NULL_HANDLE)
            vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        if (m_vertexMemory != VK_NULL_HANDLE)
            vkFreeMemory(m_device, m_vertexMemory, nullptr);

        m_device = other.m_device;
        m_vertexBuffer = other.m_vertexBuffer;
        m_vertexMemory = other.m_vertexMemory;
        m_vertexCount = other.m_vertexCount;
        m_descriptorPool = other.m_descriptorPool;
        m_descriptorSetLayout = other.m_descriptorSetLayout;
        m_descriptorSet = other.m_descriptorSet;
        m_texture = std::move(other.m_texture);
        m_position = other.m_position;
        m_size = other.m_size;

        other.m_vertexBuffer = VK_NULL_HANDLE;
        other.m_vertexMemory = VK_NULL_HANDLE;
        other.m_descriptorSet = VK_NULL_HANDLE;
    }
    return *this;
}

void Quad2D::setPosition(const glm::vec2& position)
{
    m_position = position;
}

void Quad2D::setSize(const glm::vec2& size)
{
    m_size = size;
}

void Quad2D::render(VkCommandBuffer cmd)
{
    VkPipeline pipeline = rendering::vulkan::g_frameContext.uiPipeline;
    VkPipelineLayout layout = rendering::vulkan::g_frameContext.uiPipelineLayout;

    if (cmd == VK_NULL_HANDLE || pipeline == VK_NULL_HANDLE || layout == VK_NULL_HANDLE)
        return;

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer buffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
                            0, 1, &m_descriptorSet, 0, nullptr);

    vkCmdDraw(cmd, m_vertexCount, 1, 0, 0);
}

void Quad2D::createVertexBuffer(VkPhysicalDevice physDevice)
{
    auto vertices = buildVertices();
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_vertexBuffer) != VK_SUCCESS)
        throw std::runtime_error("Quad2D: failed to create vertex buffer!");

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(m_device, m_vertexBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(physDevice, memReq.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_vertexMemory) != VK_SUCCESS)
        throw std::runtime_error("Quad2D: failed to allocate vertex buffer memory!");

    vkBindBufferMemory(m_device, m_vertexBuffer, m_vertexMemory, 0);

    void* data;
    vkMapMemory(m_device, m_vertexMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_device, m_vertexMemory);
}

void Quad2D::createDescriptorSet()
{
    if (m_descriptorSetLayout == VK_NULL_HANDLE)
        return;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("Quad2D: failed to allocate descriptor set!");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_texture->getImageView();
    imageInfo.sampler = m_texture->getSampler();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
}

std::vector<Quad2D::Vertex> Quad2D::buildVertices() const
{
    float left = m_position.x;
    float right = m_position.x + m_size.x;
    float top = m_position.y;
    float bottom = m_position.y + m_size.y;

    return {
            {{left, top, 0.0f}, {0.0f, 1.0f}},
            {{right, top, 0.0f}, {1.0f, 1.0f}},
            {{right, bottom, 0.0f}, {1.0f, 0.0f}},

            {{right, bottom, 0.0f}, {1.0f, 0.0f}},
            {{left, bottom, 0.0f}, {0.0f, 0.0f}},
            {{left, top, 0.0f}, {0.0f, 1.0f}},
    };
}

uint32_t Quad2D::findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1u << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    throw std::runtime_error("Quad2D: failed to find suitable memory type!");
}

} // namespace spear::ui
