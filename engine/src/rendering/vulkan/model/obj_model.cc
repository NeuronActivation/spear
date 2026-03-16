#include <spear/rendering/vulkan/frame_context.hh>
#include <spear/rendering/vulkan/model/obj_model.hh>

#include <glm/mat4x4.hpp>

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace spear::rendering::vulkan
{

static uint32_t findMemoryType(VkPhysicalDevice physDevice,
                               uint32_t typeFilter,
                               VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1u << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    throw std::runtime_error("OBJModel: failed to find suitable memory type!");
}

OBJModel::OBJModel(VkDevice device,
                   VkPhysicalDevice physDevice,
                   const std::string& object_file_path,
                   const std::string& material_file_path,
                   std::shared_ptr<vulkan::Texture> texture,
                   VkDescriptorPool descriptorPool,
                   VkDescriptorSetLayout descriptorSetLayout,
                   physics::bullet::ObjectData&& object_data)
    : BaseModel(nullptr, std::move(object_data)),
      m_texture(std::move(texture)),
      m_device(device)
{
    m_loader.load(object_file_path, material_file_path);
    createVertexBuffer(physDevice);

    VkDescriptorSetAllocateInfo dsAllocInfo{};
    dsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAllocInfo.descriptorPool = descriptorPool;
    dsAllocInfo.descriptorSetCount = 1;
    dsAllocInfo.pSetLayouts = &descriptorSetLayout;
    if (vkAllocateDescriptorSets(m_device, &dsAllocInfo, &m_descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("OBJModel: failed to allocate descriptor set!");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_texture->getImageView();
    imageInfo.sampler = m_texture->getSampler();

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

OBJModel::~OBJModel()
{
    if (m_vertexBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    if (m_vertexMemory != VK_NULL_HANDLE)
        vkFreeMemory(m_device, m_vertexMemory, nullptr);
    // Descriptor set freed implicitly when the pool is reset/destroyed.
}

void OBJModel::render(Camera& camera)
{
    VkCommandBuffer cmd = g_frameContext.commandBuffer;
    VkPipeline pipeline = g_frameContext.texturedPipeline;
    VkPipelineLayout layout = g_frameContext.texturedPipelineLayout;
    if (cmd == VK_NULL_HANDLE || pipeline == VK_NULL_HANDLE || layout == VK_NULL_HANDLE)
        return;

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &m_descriptorSet, 0, nullptr);

    glm::mat4 mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * Transform::getModel();

    VkBuffer buffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
    vkCmdPushConstants(cmd, layout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(glm::mat4), &mvp);
    vkCmdDraw(cmd, m_vertexCount, 1, 0, 0);
}

void OBJModel::createVertexBuffer(VkPhysicalDevice physDevice)
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
        throw std::runtime_error("OBJModel: failed to create vertex buffer!");

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(m_device, m_vertexBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(physDevice, memReq.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_vertexMemory) != VK_SUCCESS)
        throw std::runtime_error("OBJModel: failed to allocate vertex buffer memory!");

    vkBindBufferMemory(m_device, m_vertexBuffer, m_vertexMemory, 0);

    void* data;
    vkMapMemory(m_device, m_vertexMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_device, m_vertexMemory);
}

std::vector<OBJModel::Vertex> OBJModel::buildVertices()
{
    const auto& positions = m_loader.getVertices();
    const auto& uvs = m_loader.getUvs();

    std::vector<Vertex> vertices;
    for (const auto& face : m_loader.getFaces())
    {
        std::vector<Vertex> tri;
        for (size_t i = 0; i < face.vertexIndices.size(); ++i)
        {
            Vertex v{};
            int vi = face.vertexIndices[i];
            v.position = {positions[vi].x, positions[vi].y, positions[vi].z};

            if (!uvs.empty() && i < face.textureCoordIndices.size())
            {
                int ti = face.textureCoordIndices[i];
                v.uv = {uvs[ti].u, uvs[ti].v};
            }

            tri.push_back(v);
        }
        // Reverse winding (swap v1 and v2) so that after the shader's Y-flip
        // the triangles are CW in Vulkan framebuffer space (VK_FRONT_FACE_CLOCKWISE).
        if (tri.size() == 3)
            std::swap(tri[1], tri[2]);
        vertices.insert(vertices.end(), tri.begin(), tri.end());
    }
    return vertices;
}

} // namespace spear::rendering::vulkan
