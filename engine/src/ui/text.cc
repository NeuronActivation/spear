#include <spear/rendering/vulkan/frame_context.hh>
#include <spear/rendering/vulkan/texture/stb_texture.hh>
#include <spear/ui/text.hh>

#include <cstring>
#include <stdexcept>
#include <vector>

namespace spear::ui
{

static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool pool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);
    return cmd;
}

static void endSingleTimeCommands(VkDevice device, VkCommandPool pool, VkQueue queue, VkCommandBuffer cmd)
{
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, pool, 1, &cmd);
}

Text::Text(VkDevice device,
           VkPhysicalDevice physDevice,
           VkCommandPool commandPool,
           VkQueue graphicsQueue,
           VkDescriptorPool descriptorPool,
           VkDescriptorSetLayout descriptorSetLayout,
           const std::string& fontPath,
           int fontSize)
    : m_device(device),
      m_physDevice(physDevice),
      m_commandPool(commandPool),
      m_graphicsQueue(graphicsQueue),
      m_descriptorPool(descriptorPool),
      m_descriptorSetLayout(descriptorSetLayout),
      m_fontPath(fontPath),
      m_fontSize(fontSize)
{
    if (!TTF_WasInit())
    {
        TTF_Init();
    }

    m_font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!m_font)
        throw std::runtime_error("Text: failed to load font: " + fontPath);
}

Text::~Text()
{
    if (m_vertexBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    if (m_vertexMemory != VK_NULL_HANDLE)
        vkFreeMemory(m_device, m_vertexMemory, nullptr);
    if (m_font)
        TTF_CloseFont(m_font);
}

Text::Text(Text&& other) noexcept
    : m_device(other.m_device),
      m_physDevice(other.m_physDevice),
      m_commandPool(other.m_commandPool),
      m_graphicsQueue(other.m_graphicsQueue),
      m_descriptorPool(other.m_descriptorPool),
      m_descriptorSetLayout(other.m_descriptorSetLayout),
      m_fontPath(std::move(other.m_fontPath)),
      m_fontSize(other.m_fontSize),
      m_string(std::move(other.m_string)),
      m_position(other.m_position),
      m_color(other.m_color),
      m_font(other.m_font),
      m_texture(std::move(other.m_texture)),
      m_vertexBuffer(other.m_vertexBuffer),
      m_vertexMemory(other.m_vertexMemory),
      m_vertexCount(other.m_vertexCount),
      m_descriptorSet(other.m_descriptorSet)
{
    other.m_font = nullptr;
    other.m_vertexBuffer = VK_NULL_HANDLE;
    other.m_vertexMemory = VK_NULL_HANDLE;
    other.m_descriptorSet = VK_NULL_HANDLE;
}

Text& Text::operator=(Text&& other) noexcept
{
    if (this != &other)
    {
        if (m_vertexBuffer != VK_NULL_HANDLE)
            vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        if (m_vertexMemory != VK_NULL_HANDLE)
            vkFreeMemory(m_device, m_vertexMemory, nullptr);
        if (m_font)
            TTF_CloseFont(m_font);

        m_device = other.m_device;
        m_physDevice = other.m_physDevice;
        m_commandPool = other.m_commandPool;
        m_graphicsQueue = other.m_graphicsQueue;
        m_descriptorPool = other.m_descriptorPool;
        m_descriptorSetLayout = other.m_descriptorSetLayout;
        m_fontPath = std::move(other.m_fontPath);
        m_fontSize = other.m_fontSize;
        m_string = std::move(other.m_string);
        m_position = other.m_position;
        m_color = other.m_color;
        m_font = other.m_font;
        m_texture = std::move(other.m_texture);
        m_vertexBuffer = other.m_vertexBuffer;
        m_vertexMemory = other.m_vertexMemory;
        m_vertexCount = other.m_vertexCount;
        m_descriptorSet = other.m_descriptorSet;

        other.m_font = nullptr;
        other.m_vertexBuffer = VK_NULL_HANDLE;
        other.m_vertexMemory = VK_NULL_HANDLE;
        other.m_descriptorSet = VK_NULL_HANDLE;
    }
    return *this;
}

void Text::setString(const std::string& text)
{
    m_string = text;
    rebuildTexture();
}

void Text::setPosition(const glm::vec2& position)
{
    m_position = position;
    if (m_texture)
        rebuildQuad();
}

void Text::setColor(const SDL_Color& color)
{
    m_color = color;
    if (!m_string.empty())
        rebuildTexture();
}

void Text::setScale(float scale)
{
    m_scale = scale;
    if (m_texture)
        rebuildQuad();
}

glm::vec2 Text::getSize() const
{
    if (m_texture)
        return glm::vec2(static_cast<float>(m_texture->getWidth()) * m_scale,
                         static_cast<float>(m_texture->getHeight()) * m_scale);
    return glm::vec2(0.0f, 0.0f);
}

void Text::render(VkCommandBuffer cmd)
{
    if (m_string.empty() || m_vertexBuffer == VK_NULL_HANDLE || m_descriptorSet == VK_NULL_HANDLE)
        return;

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

void Text::rebuildTexture()
{
    // Clean up old vertex buffer if it exists
    if (m_vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        m_vertexBuffer = VK_NULL_HANDLE;
    }
    if (m_vertexMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_device, m_vertexMemory, nullptr);
        m_vertexMemory = VK_NULL_HANDLE;
    }

    if (m_string.empty())
        return;

    // Render text to SDL surface
    SDL_Surface* surface = TTF_RenderText_Blended(m_font, m_string.c_str(), m_string.length(), m_color);
    if (!surface)
        throw std::runtime_error("Text: TTF_RenderText_Blended failed");

    // Convert to ABGR8888 so byte order on little-endian is R,G,B,A matching VK_FORMAT_R8G8B8A8_SRGB
    SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ABGR8888);
    if (!converted)
    {
        SDL_DestroySurface(surface);
        throw std::runtime_error("Text: SDL_ConvertSurface failed");
    }
    SDL_DestroySurface(surface);
    surface = converted;

    int texWidth = surface->w;
    int texHeight = surface->h;
    VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4);

    // Create staging buffer for pixel data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = imageSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS)
        {
            SDL_DestroySurface(surface);
            throw std::runtime_error("Text: failed to create staging buffer");
        }

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memReq);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = findMemoryType(m_physDevice, memReq.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &stagingMemory) != VK_SUCCESS)
        {
            vkDestroyBuffer(m_device, stagingBuffer, nullptr);
            SDL_DestroySurface(surface);
            throw std::runtime_error("Text: failed to allocate staging memory");
        }

        vkBindBufferMemory(m_device, stagingBuffer, stagingMemory, 0);

        void* data;
        vkMapMemory(m_device, stagingMemory, 0, imageSize, 0, &data);
        memcpy(data, surface->pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device, stagingMemory);
    }

    SDL_DestroySurface(surface);

    // Create VkImage
    VkImage newImage;
    VkDeviceMemory newImageMemory;
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(texWidth);
        imageInfo.extent.height = static_cast<uint32_t>(texHeight);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(m_device, &imageInfo, nullptr, &newImage) != VK_SUCCESS)
        {
            vkDestroyBuffer(m_device, stagingBuffer, nullptr);
            vkFreeMemory(m_device, stagingMemory, nullptr);
            throw std::runtime_error("Text: failed to create image");
        }

        VkMemoryRequirements imgMemReq;
        vkGetImageMemoryRequirements(m_device, newImage, &imgMemReq);

        VkMemoryAllocateInfo imgAllocInfo{};
        imgAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        imgAllocInfo.allocationSize = imgMemReq.size;
        imgAllocInfo.memoryTypeIndex = findMemoryType(m_physDevice, imgMemReq.memoryTypeBits,
                                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(m_device, &imgAllocInfo, nullptr, &newImageMemory) != VK_SUCCESS)
        {
            vkDestroyImage(m_device, newImage, nullptr);
            vkDestroyBuffer(m_device, stagingBuffer, nullptr);
            vkFreeMemory(m_device, stagingMemory, nullptr);
            throw std::runtime_error("Text: failed to allocate image memory");
        }

        vkBindImageMemory(m_device, newImage, newImageMemory, 0);
    }

    // Transition: UNDEFINED -> TRANSFER_DST
    {
        VkCommandBuffer cmd = beginSingleTimeCommands(m_device, m_commandPool);
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = newImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
        endSingleTimeCommands(m_device, m_commandPool, m_graphicsQueue, cmd);
    }

    // Copy buffer to image
    {
        VkCommandBuffer cmd = beginSingleTimeCommands(m_device, m_commandPool);
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1};

        vkCmdCopyBufferToImage(cmd, stagingBuffer, newImage,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        endSingleTimeCommands(m_device, m_commandPool, m_graphicsQueue, cmd);
    }

    // Clean up staging resources
    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingMemory, nullptr);

    // Transition: TRANSFER_DST -> SHADER_READ_ONLY
    VkImageView newImageView;
    VkSampler newSampler;
    {
        VkCommandBuffer cmd = beginSingleTimeCommands(m_device, m_commandPool);
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = newImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
        endSingleTimeCommands(m_device, m_commandPool, m_graphicsQueue, cmd);

        // Create image view
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = newImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device, &viewInfo, nullptr, &newImageView) != VK_SUCCESS)
            throw std::runtime_error("Text: failed to create image view");

        // Create sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(m_device, &samplerInfo, nullptr, &newSampler) != VK_SUCCESS)
            throw std::runtime_error("Text: failed to create sampler");
    }

    // Create texture wrapper and set its internals via public setters
    m_texture = std::make_shared<rendering::vulkan::STBTexture>(
            m_device, m_physDevice, m_commandPool, m_graphicsQueue);
    m_texture->setWidth(texWidth);
    m_texture->setHeight(texHeight);
    m_texture->setImage(newImage);
    m_texture->setImageMemory(newImageMemory);
    m_texture->setImageView(newImageView);
    m_texture->setSampler(newSampler);

    // Now rebuild the quad with the new texture dimensions
    rebuildQuad();
}

void Text::rebuildQuad()
{
    if (!m_texture || m_string.empty())
        return;

    // Clean up old vertex buffer
    if (m_vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        m_vertexBuffer = VK_NULL_HANDLE;
    }
    if (m_vertexMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_device, m_vertexMemory, nullptr);
        m_vertexMemory = VK_NULL_HANDLE;
    }

    float texWidth = static_cast<float>(m_texture->getWidth());
    float texHeight = static_cast<float>(m_texture->getHeight());

    auto vertices = buildVertices(texWidth, texHeight);
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_vertexBuffer) != VK_SUCCESS)
        throw std::runtime_error("Text: failed to create vertex buffer");

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(m_device, m_vertexBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(m_physDevice, memReq.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_vertexMemory) != VK_SUCCESS)
        throw std::runtime_error("Text: failed to allocate vertex buffer memory");

    vkBindBufferMemory(m_device, m_vertexBuffer, m_vertexMemory, 0);

    void* data;
    vkMapMemory(m_device, m_vertexMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_device, m_vertexMemory);

    // Create descriptor set
    VkDescriptorSetAllocateInfo descAllocInfo{};
    descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descAllocInfo.descriptorPool = m_descriptorPool;
    descAllocInfo.descriptorSetCount = 1;
    descAllocInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vkAllocateDescriptorSets(m_device, &descAllocInfo, &m_descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("Text: failed to allocate descriptor set");

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

std::vector<Text::Vertex> Text::buildVertices(float texWidth, float texHeight) const
{
    float w = texWidth * m_scale;
    float h = texHeight * m_scale;
    float left = m_position.x;
    float right = m_position.x + w;
    float top = m_position.y;
    float bottom = m_position.y + h;

    return {
            {{left, top, 0.0f}, {0.0f, 1.0f}},
            {{right, top, 0.0f}, {1.0f, 1.0f}},
            {{right, bottom, 0.0f}, {1.0f, 0.0f}},

            {{right, bottom, 0.0f}, {1.0f, 0.0f}},
            {{left, bottom, 0.0f}, {0.0f, 0.0f}},
            {{left, top, 0.0f}, {0.0f, 1.0f}},
    };
}

uint32_t Text::findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1u << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    throw std::runtime_error("Text: failed to find suitable memory type");
}

} // namespace spear::ui
