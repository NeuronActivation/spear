#ifndef SPEAR_RENDERING_VULKAN_SPRITE_3D_HH
#define SPEAR_RENDERING_VULKAN_SPRITE_3D_HH

#include <spear/rendering/base_sprite_3d.hh>
#include <spear/rendering/vulkan/texture/texture.hh>

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace spear::rendering::vulkan
{

/// A textured 2D quad rendered in 3D space. Inherits BaseSprite3D (which chains
/// through Shape → GameObject) so it can be placed in Scene::Container.
/// A {pos, uv} vertex buffer is created on construction and rendered using the
/// textured pipeline provided by the frame context.
class Sprite3D : public BaseSprite3D
{
public:
    /// Constructor.
    /// \param device            Logical device.
    /// \param physDevice        Physical device (memory allocation).
    /// \param texture           Source texture — imageView/sampler are borrowed;
    ///                          the shared_ptr keeps the Vulkan objects alive.
    /// \param descriptorPool    Pool from which the descriptor set is allocated.
    /// \param descriptorSetLayout Layout with a combined-image-sampler at binding 0.
    /// \param object_data       Transform / physics data (use mass=0 for a static sprite).
    Sprite3D(VkDevice device,
             VkPhysicalDevice physDevice,
             std::shared_ptr<vulkan::Texture> texture,
             VkDescriptorPool descriptorPool,
             VkDescriptorSetLayout descriptorSetLayout,
             physics::bullet::ObjectData&& object_data);

    ~Sprite3D();

    void render(Camera& camera) override;

private:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
    };

    std::vector<Vertex> createVertices() const;
    void createVertexBuffer(VkPhysicalDevice physDevice);

private:
    /// Kept alive so that the borrowed VkImageView / VkSampler remain valid.
    std::shared_ptr<vulkan::Texture> m_vkTexture;

    VkDevice m_device = VK_NULL_HANDLE;
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexMemory = VK_NULL_HANDLE;
    uint32_t m_vertexCount = 0;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
};

} // namespace spear::rendering::vulkan

#endif
