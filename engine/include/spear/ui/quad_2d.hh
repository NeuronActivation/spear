#ifndef SPEAR_UI_QUAD_2D_HH
#define SPEAR_UI_QUAD_2D_HH

#include <spear/rendering/vulkan/texture/texture.hh>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace spear::ui
{

class Quad2D
{
public:
    Quad2D(VkDevice device,
           VkPhysicalDevice physDevice,
           VkDescriptorPool descriptorPool,
           VkDescriptorSetLayout descriptorSetLayout,
           std::shared_ptr<rendering::vulkan::Texture> texture);

    ~Quad2D();

    Quad2D(const Quad2D&) = delete;
    Quad2D& operator=(const Quad2D&) = delete;
    Quad2D(Quad2D&& other) noexcept;
    Quad2D& operator=(Quad2D&& other) noexcept;

    void setPosition(const glm::vec2& position);
    void setSize(const glm::vec2& size);

    const glm::vec2& getPosition() const
    {
        return m_position;
    }
    const glm::vec2& getSize() const
    {
        return m_size;
    }

    void render(VkCommandBuffer cmd);

private:
    void createVertexBuffer(VkPhysicalDevice physDevice);
    void createDescriptorSet();

    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
    };

    std::vector<Vertex> buildVertices() const;

    VkDevice m_device;
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexMemory = VK_NULL_HANDLE;
    uint32_t m_vertexCount = 0;

    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

    std::shared_ptr<rendering::vulkan::Texture> m_texture;

    glm::vec2 m_position{0.0f, 0.0f};
    glm::vec2 m_size{1.0f, 1.0f};

    uint32_t findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

} // namespace spear::ui

#endif
