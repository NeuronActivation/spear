#ifndef SPEAR_RENDERING_VULKAN_SHAPES_CUBE_HH
#define SPEAR_RENDERING_VULKAN_SHAPES_CUBE_HH

#include <spear/camera.hh>
#include <spear/rendering/shapes/shape.hh>

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace spear::rendering::vulkan
{

class Cube : public rendering::Shape
{
public:
    /// Constructor.
    /// \param device Logical device used to create GPU buffers.
    /// \param physDevice Physical device for memory type selection.
    /// \param object_data Physics and transform data.
    /// \param color Tint applied to the face colors.
    Cube(VkDevice device,
         VkPhysicalDevice physDevice,
         physics::bullet::ObjectData&& object_data,
         const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    ~Cube();

    void render(Camera& camera) override;

    static std::unique_ptr<Cube> create(VkDevice device,
                                        VkPhysicalDevice physDevice,
                                        physics::bullet::ObjectData&& object_data,
                                        const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        return std::make_unique<Cube>(device, physDevice, std::move(object_data), color);
    }

private:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
    };

    std::vector<Vertex> createVertices() const;
    uint32_t findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
    VkDevice m_device;
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexMemory = VK_NULL_HANDLE;
    uint32_t m_vertexCount = 0;
};

} // namespace spear::rendering::vulkan

#endif
