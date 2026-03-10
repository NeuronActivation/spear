#include <spear/rendering/vulkan/shapes/cube.hh>
#include <spear/rendering/vulkan/frame_context.hh>

#include <glm/mat4x4.hpp>

#include <cstring>
#include <stdexcept>

namespace spear::rendering::vulkan
{

Cube::Cube(VkDevice device,
           VkPhysicalDevice physDevice,
           physics::bullet::ObjectData&& object_data,
           const glm::vec4& color)
    : Shape(nullptr, std::move(object_data), color),
      m_device(device)
{
    auto vertices = createVertices();
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_vertexBuffer) != VK_SUCCESS)
        throw std::runtime_error("Cube: failed to create vertex buffer!");

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, m_vertexBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(physDevice, memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &m_vertexMemory) != VK_SUCCESS)
        throw std::runtime_error("Cube: failed to allocate vertex buffer memory!");

    vkBindBufferMemory(device, m_vertexBuffer, m_vertexMemory, 0);

    void* data;
    vkMapMemory(device, m_vertexMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, m_vertexMemory);
}

Cube::~Cube()
{
    if (m_vertexBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    if (m_vertexMemory != VK_NULL_HANDLE)
        vkFreeMemory(m_device, m_vertexMemory, nullptr);
}

void Cube::render(Camera& camera)
{
    VkCommandBuffer cmd = g_frameContext.commandBuffer;
    VkPipelineLayout layout = g_frameContext.pipelineLayout;
    if (cmd == VK_NULL_HANDLE || layout == VK_NULL_HANDLE)
        return;

    glm::mat4 mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * Transform::getModel();

    VkBuffer buffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
    vkCmdPushConstants(cmd, layout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(glm::mat4), &mvp);
    vkCmdDraw(cmd, m_vertexCount, 1, 0, 0);
}

std::vector<Cube::Vertex> Cube::createVertices() const
{
    const float s = 1.0f;
    // Face colors tinted by m_color.rgb; default white tint preserves the base face colors.
    const glm::vec3 tint = glm::vec3(m_color);
    const glm::vec3 red   = tint * glm::vec3(1, 0.2f, 0.2f);
    const glm::vec3 green = tint * glm::vec3(0.2f, 1, 0.2f);
    const glm::vec3 blue  = tint * glm::vec3(0.2f, 0.2f, 1);
    const glm::vec3 yel   = tint * glm::vec3(1, 1, 0.2f);
    const glm::vec3 cyan  = tint * glm::vec3(0.2f, 1, 1);
    const glm::vec3 mag   = tint * glm::vec3(1, 0.2f, 1);

    // clang-format off
    return {
        // Front (+z) - red
        {{-s,-s,+s}, red},  {{+s,-s,+s}, red},  {{+s,+s,+s}, red},
        {{+s,+s,+s}, red},  {{-s,+s,+s}, red},  {{-s,-s,+s}, red},
        // Back (-z) - green
        {{+s,-s,-s}, green},{{-s,-s,-s}, green},{{-s,+s,-s}, green},
        {{-s,+s,-s}, green},{{+s,+s,-s}, green},{{+s,-s,-s}, green},
        // Left (-x) - blue
        {{-s,-s,-s}, blue}, {{-s,-s,+s}, blue}, {{-s,+s,+s}, blue},
        {{-s,+s,+s}, blue}, {{-s,+s,-s}, blue}, {{-s,-s,-s}, blue},
        // Right (+x) - yellow
        {{+s,-s,+s}, yel},  {{+s,-s,-s}, yel},  {{+s,+s,-s}, yel},
        {{+s,+s,-s}, yel},  {{+s,+s,+s}, yel},  {{+s,-s,+s}, yel},
        // Bottom (-y) - cyan
        {{-s,-s,-s}, cyan}, {{+s,-s,-s}, cyan},  {{+s,-s,+s}, cyan},
        {{+s,-s,+s}, cyan}, {{-s,-s,+s}, cyan},  {{-s,-s,-s}, cyan},
        // Top (+y) - magenta
        {{-s,+s,+s}, mag},  {{+s,+s,+s}, mag},  {{+s,+s,-s}, mag},
        {{+s,+s,-s}, mag},  {{-s,+s,-s}, mag},  {{-s,+s,+s}, mag},
    };
    // clang-format on
}

uint32_t Cube::findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1u << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    throw std::runtime_error("Cube: failed to find suitable memory type!");
}

} // namespace spear::rendering::vulkan
