#ifndef SPEAR_RENDERING_VULKAN_UI_QUAD_2D_HH
#define SPEAR_RENDERING_VULKAN_UI_QUAD_2D_HH

#include <spear/rendering/vulkan/texture/texture.hh>
#include <spear/ui/base_quad_2d.hh>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace spear::ui::vulkan
{

class Quad2D : public spear::ui::BaseQuad2D
{
public:
    Quad2D(VkDevice device,
           VkPhysicalDevice physDevice,
           VkDescriptorPool descriptorPool,
           VkDescriptorSetLayout descriptorSetLayout,
           std::shared_ptr<spear::rendering::vulkan::Texture> texture);

    ~Quad2D() override;

    Quad2D(const Quad2D&) = delete;
    Quad2D& operator=(const Quad2D&) = delete;
    Quad2D(Quad2D&& other) noexcept;
    Quad2D& operator=(Quad2D&& other) noexcept;

    void setPosition(const glm::vec2& position) override;
    void setSize(const glm::vec2& size) override;

    void render(RenderContext ctx) override;

private:
    void createVertexBuffer(VkPhysicalDevice physDevice);
    void createDescriptorSet();

    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
    };

    std::vector<Vertex> buildVertices() const;

    uint32_t findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkDevice m_device;
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexMemory = VK_NULL_HANDLE;
    uint32_t m_vertexCount = 0;

    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

    std::shared_ptr<spear::rendering::vulkan::Texture> m_texture;
};

} // namespace spear::ui::vulkan

#endif
