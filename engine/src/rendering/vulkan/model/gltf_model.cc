#include <spear/rendering/vulkan/frame_context.hh>
#include <spear/rendering/vulkan/model/gltf_model.hh>

#include <glm/mat4x4.hpp>

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace spear::rendering::vulkan
{

namespace
{

uint32_t findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1u << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    throw std::runtime_error("GLTFModel: failed to find suitable memory type!");
}

} // namespace

GLTFModel::GLTFModel(VkDevice device,
                     VkPhysicalDevice physDevice,
                     VkCommandPool commandPool,
                     VkQueue graphicsQueue,
                     const std::string& file_path,
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
    GLTFLoader loader;
    if (!loader.load(file_path))
        return;
    for (const auto& material : loader.getMaterials())
        createMaterial(material);
    std::cout << "GLTFModel: loaded " << file_path << " with " << m_materialData.size() << " materials" << std::endl;
}

GLTFModel::~GLTFModel()
{
    for (auto& mat : m_materialData)
    {
        if (mat.descriptorSet != VK_NULL_HANDLE)
            vkFreeDescriptorSets(m_device, m_descriptorPool, 1, &mat.descriptorSet);
        if (mat.vertexBuffer != VK_NULL_HANDLE)
            vkDestroyBuffer(m_device, mat.vertexBuffer, nullptr);
        if (mat.vertexMemory != VK_NULL_HANDLE)
            vkFreeMemory(m_device, mat.vertexMemory, nullptr);
    }
}

void GLTFModel::render(Camera& camera)
{
    using AlphaMode = GLTFLoader::AlphaMode;
    auto& frame = g_frameContext;
    VkCommandBuffer cmd = frame.commandBuffer;
    VkPipelineLayout layout = frame.shadowPass ? frame.shadowPipelineLayout : frame.texturedPipelineLayout;
    if (cmd == VK_NULL_HANDLE || layout == VK_NULL_HANDLE)
        return;

    const glm::mat4& model = Transform::getModel();
    const glm::mat4 view = camera.getViewMatrix();
    const glm::mat4 mvp =
            (frame.shadowPass ? frame.lightViewProjection : camera.getProjectionMatrix() * view) * model;

    if (frame.shadowPass)
    {
        if (frame.shadowTexturedPipeline == VK_NULL_HANDLE)
            return;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, frame.shadowTexturedPipeline);
        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(glm::mat4), &mvp);
        for (const auto& mat : m_materialData)
        {
            if (mat.alphaMode == AlphaMode::Blend)
                continue;
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &mat.vertexBuffer, &offset);
            vkCmdDraw(cmd, mat.vertexCount, 1, 0, 0);
        }
        return;
    }

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &frame.sunDescriptorSet, 0, nullptr);
    const float viewModel = frame.viewModel ? 1.0f : 0.0f;
    VkPipeline bound = VK_NULL_HANDLE;
    for (const auto& mat : m_materialData)
    {
        if (mat.alphaMode == AlphaMode::Blend)
        {
            TransparentDraw draw;
            draw.pipeline = mat.doubleSided ? frame.transparentDoubleSidedPipeline : frame.transparentPipeline;
            draw.vertexBuffer = mat.vertexBuffer;
            draw.vertexCount = mat.vertexCount;
            draw.textureSet = mat.descriptorSet;
            draw.mvp = mvp;
            draw.params = glm::vec4(viewModel, 0.0f, 0.0f, 0.0f);
            draw.depth = -(view * model * glm::vec4(mat.center, 1.0f)).z;
            if (draw.pipeline != VK_NULL_HANDLE)
                frame.transparentDraws.push_back(draw);
            continue;
        }

        VkPipeline pipeline = mat.doubleSided ? frame.texturedDoubleSidedPipeline : frame.texturedPipeline;
        if (pipeline == VK_NULL_HANDLE)
            continue;
        if (pipeline != bound)
        {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            bound = pipeline;
        }
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &mat.descriptorSet, 0, nullptr);
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &mat.vertexBuffer, &offset);
        const glm::vec4 params(viewModel, mat.alphaMode == AlphaMode::Mask ? mat.alphaCutoff : 0.0f, 0.0f, 0.0f);
        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(glm::mat4), &mvp);
        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4),
                           sizeof(glm::vec4), &params);
        vkCmdDraw(cmd, mat.vertexCount, 1, 0, 0);
    }
}

void GLTFModel::createMaterial(const GLTFLoader::Material& material)
{
    MaterialRenderData renderData;
    renderData.alphaMode = material.alphaMode;
    renderData.alphaCutoff = material.alphaCutoff;
    renderData.doubleSided = material.doubleSided;
    for (const auto& vertex : material.vertices)
        renderData.center += vertex.position;
    if (!material.vertices.empty())
        renderData.center /= static_cast<float>(material.vertices.size());

    renderData.texture = std::make_shared<STBTexture>(m_device, m_physDevice, m_commandPool, m_graphicsQueue);
    renderData.texture->loadFromRGBA(material.baseColor.pixels.data(), material.baseColor.width,
                                     material.baseColor.height);

    std::vector<Vertex> vertices;
    vertices.reserve(material.vertices.size());
    // glTF faces are counter-clockwise, which the textured pipeline treats as front facing.
    for (const auto& vertex : material.vertices)
        vertices.push_back({vertex.position, vertex.uv});
    renderData.vertexCount = static_cast<uint32_t>(vertices.size());
    const VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &renderData.vertexBuffer) != VK_SUCCESS)
        throw std::runtime_error("GLTFModel: failed to create vertex buffer");

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(m_device, renderData.vertexBuffer, &memReq);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(m_physDevice, memReq.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &renderData.vertexMemory) != VK_SUCCESS)
        throw std::runtime_error("GLTFModel: failed to allocate vertex memory");
    vkBindBufferMemory(m_device, renderData.vertexBuffer, renderData.vertexMemory, 0);

    void* data;
    vkMapMemory(m_device, renderData.vertexMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_device, renderData.vertexMemory);

    VkDescriptorSetAllocateInfo dsAllocInfo{};
    dsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAllocInfo.descriptorPool = m_descriptorPool;
    dsAllocInfo.descriptorSetCount = 1;
    dsAllocInfo.pSetLayouts = &m_descriptorSetLayout;
    if (vkAllocateDescriptorSets(m_device, &dsAllocInfo, &renderData.descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("GLTFModel: failed to allocate descriptor set");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = renderData.texture->getImageView();
    imageInfo.sampler = renderData.texture->getSampler();

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = renderData.descriptorSet;
    write.dstBinding = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);

    m_materialData.push_back(renderData);
}

} // namespace spear::rendering::vulkan
