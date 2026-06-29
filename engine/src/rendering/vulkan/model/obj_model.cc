#include <spear/rendering/vulkan/frame_context.hh>
#include <spear/rendering/vulkan/model/obj_model.hh>

#include <glm/mat4x4.hpp>

#include <algorithm>
#include <cstring>
#include <iostream>
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
                   VkCommandPool commandPool,
                   VkQueue graphicsQueue,
                   const std::string& object_file_path,
                   const std::string& material_file_path,
                   VkDescriptorPool descriptorPool,
                   VkDescriptorSetLayout descriptorSetLayout,
                   physics::bullet::ObjectData&& object_data)
    : BaseModel(nullptr, std::move(object_data)),
      m_device(device),
      m_physDevice(physDevice),
      m_commandPool(commandPool),
      m_graphicsQueue(graphicsQueue),
      m_descriptorPool(descriptorPool),
      m_descriptorSetLayout(descriptorSetLayout)
{
    m_loader.load(object_file_path, material_file_path);
    createMaterialBuffers(physDevice);
}

OBJModel::~OBJModel()
{
    for (auto& mat : m_materialData)
    {
        if (mat.vertexBuffer != VK_NULL_HANDLE)
            vkDestroyBuffer(m_device, mat.vertexBuffer, nullptr);
        if (mat.vertexMemory != VK_NULL_HANDLE)
            vkFreeMemory(m_device, mat.vertexMemory, nullptr);
        // Texture shared_ptr and descriptor sets are freed automatically.
    }
}

void OBJModel::render(Camera& camera)
{
    VkCommandBuffer cmd = g_frameContext.commandBuffer;
    VkPipeline pipeline = g_frameContext.texturedPipeline;
    VkPipelineLayout layout = g_frameContext.texturedPipelineLayout;
    if (cmd == VK_NULL_HANDLE || pipeline == VK_NULL_HANDLE || layout == VK_NULL_HANDLE)
        return;

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    glm::mat4 mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * Transform::getModel();

    for (const auto& mat : m_materialData)
    {
        if (mat.vertexCount == 0 || mat.descriptorSet == VK_NULL_HANDLE)
            continue;

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &mat.descriptorSet, 0, nullptr);

        VkBuffer buffers[] = {mat.vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
        vkCmdPushConstants(cmd, layout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0, sizeof(glm::mat4), &mvp);
        vkCmdDraw(cmd, mat.vertexCount, 1, 0, 0);
    }
}

void OBJModel::createMaterialBuffers(VkPhysicalDevice physDevice)
{
    const auto& materials = m_loader.getMaterials();
    const auto& facesByMaterial = m_loader.getFacesByMaterial();

    m_materialData.resize(facesByMaterial.size());

    for (size_t matIdx = 0; matIdx < facesByMaterial.size(); ++matIdx)
    {
        const auto& faces = facesByMaterial[matIdx];
        if (faces.empty())
            continue;

        auto& renderData = m_materialData[matIdx];

        // Load texture for this material if available.
        if (matIdx < materials.size() && !materials[matIdx].texturePath.empty())
        {
            try
            {
                auto tex = std::make_shared<STBTexture>(
                        m_device, m_physDevice, m_commandPool, m_graphicsQueue);
                tex->loadFromFile(materials[matIdx].texturePath);
                renderData.texture = tex;
            }
            catch (const std::exception& e)
            {
                std::cerr << "OBJModel: failed to load texture '" << materials[matIdx].texturePath
                          << "' for material '" << materials[matIdx].name << "': " << e.what() << std::endl;
            }
        }

        if (!renderData.texture)
        {
            auto fallback = std::make_shared<STBTexture>(
                    m_device, m_physDevice, m_commandPool, m_graphicsQueue);
            unsigned char whitePixel[4] = {255, 255, 255, 255};
            fallback->loadFromRGBA(whitePixel, 1, 1);
            renderData.texture = fallback;
        }

        // Build vertices for this material's faces.
        std::vector<Vertex> vertices = buildVertices(faces);
        renderData.vertexCount = static_cast<uint32_t>(vertices.size());
        VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

        // Create vertex buffer.
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &renderData.vertexBuffer) != VK_SUCCESS)
            throw std::runtime_error("OBJModel: failed to create vertex buffer for material " + std::to_string(matIdx));

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(m_device, renderData.vertexBuffer, &memReq);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = findMemoryType(physDevice, memReq.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &renderData.vertexMemory) != VK_SUCCESS)
            throw std::runtime_error("OBJModel: failed to allocate vertex memory for material " + std::to_string(matIdx));

        vkBindBufferMemory(m_device, renderData.vertexBuffer, renderData.vertexMemory, 0);

        void* data;
        vkMapMemory(m_device, renderData.vertexMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_device, renderData.vertexMemory);

        // Allocate and write descriptor set.
        VkDescriptorSetAllocateInfo dsAllocInfo{};
        dsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        dsAllocInfo.descriptorPool = m_descriptorPool;
        dsAllocInfo.descriptorSetCount = 1;
        dsAllocInfo.pSetLayouts = &m_descriptorSetLayout;
        if (vkAllocateDescriptorSets(m_device, &dsAllocInfo, &renderData.descriptorSet) != VK_SUCCESS)
            throw std::runtime_error("OBJModel: failed to allocate descriptor set for material " + std::to_string(matIdx));

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = renderData.texture->getImageView();
        imageInfo.sampler = renderData.texture->getSampler();

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = renderData.descriptorSet;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &imageInfo;
        vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);

        std::cout << "OBJModel: loaded material " << matIdx << " (" << materials[matIdx].name
                  << ") with " << renderData.vertexCount << " vertices" << std::endl;
    }

    std::cout << "OBJModel: loaded " << m_materialData.size() << " material groups ("
              << facesByMaterial.size() << " total groups)" << std::endl;
}

std::vector<OBJModel::Vertex> OBJModel::buildVertices(const std::vector<ModelLoader::Face>& faces)
{
    const auto& positions = m_loader.getVertices();
    const auto& uvs = m_loader.getUvs();

    std::vector<Vertex> vertices;
    for (const auto& face : faces)
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
