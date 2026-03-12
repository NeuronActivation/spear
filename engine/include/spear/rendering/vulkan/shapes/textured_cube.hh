#ifndef SPEAR_RENDERING_VULKAN_SHAPES_TEXTURED_CUBE_HH
#define SPEAR_RENDERING_VULKAN_SHAPES_TEXTURED_CUBE_HH

#include <spear/rendering/vulkan/shapes/cube.hh>
#include <spear/rendering/vulkan/texture/texture.hh>

#include <memory>

namespace spear::rendering::vulkan
{

/// A textured cube. Inherits Cube (geometry + physics) and Texture (descriptor
/// set management). The vertex buffer is replaced with a {pos, uv} layout and a
/// combined-image-sampler descriptor set is created on construction.
class TexturedCube : public Cube, public Texture
{
public:
    /// Constructor.
    /// \param device            Logical device.
    /// \param physDevice        Physical device (memory allocation).
    /// \param texture           Source texture — imageView/sampler are borrowed;
    ///                          the shared_ptr keeps the Vulkan objects alive.
    /// \param descriptorPool    Pool from which the descriptor set is allocated.
    /// \param descriptorSetLayout Layout with a combined-image-sampler at binding 0.
    /// \param object_data       Physics / transform data.
    /// \param color             Optional colour tint.
    TexturedCube(VkDevice device,
                 VkPhysicalDevice physDevice,
                 std::shared_ptr<vulkan::Texture> texture,
                 VkDescriptorPool descriptorPool,
                 VkDescriptorSetLayout descriptorSetLayout,
                 physics::bullet::ObjectData&& object_data,
                 const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    ~TexturedCube();

    void render(Camera& camera) override;

    static std::unique_ptr<TexturedCube> create(VkDevice device,
                                                VkPhysicalDevice physDevice,
                                                std::shared_ptr<vulkan::Texture> texture,
                                                VkDescriptorPool descriptorPool,
                                                VkDescriptorSetLayout descriptorSetLayout,
                                                physics::bullet::ObjectData&& object_data,
                                                const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        return std::make_unique<TexturedCube>(device, physDevice, std::move(texture),
                                              descriptorPool, descriptorSetLayout,
                                              std::move(object_data), color);
    }

private:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
    };

    std::vector<Vertex> createVertices() const;
    void replaceVertexBuffer(VkPhysicalDevice physDevice);

private:
    /// Kept alive so that the borrowed VkImageView / VkSampler remain valid.
    std::shared_ptr<vulkan::Texture> m_texture;
};

} // namespace spear::rendering::vulkan

#endif
