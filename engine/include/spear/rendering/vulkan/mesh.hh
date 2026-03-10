#ifndef SPEAR_RENDERING_VULKAN_MESH_HH
#define SPEAR_RENDERING_VULKAN_MESH_HH

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <vulkan/vulkan.h>

#include <vector>

namespace spear::rendering::vulkan
{

class Mesh
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
    };

    void initialize(VkDevice device, VkPhysicalDevice physDevice, const std::vector<Vertex>& vertices);
    void record(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const glm::mat4& mvp);
    void cleanup(VkDevice device);

private:
    uint32_t findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexMemory = VK_NULL_HANDLE;
    uint32_t m_vertexCount = 0;
};

} // namespace spear::rendering::vulkan

#endif
