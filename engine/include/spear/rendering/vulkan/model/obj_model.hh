#ifndef SPEAR_RENDERING_VULKAN_MODEL_OBJ_MODEL_HH
#define SPEAR_RENDERING_VULKAN_MODEL_OBJ_MODEL_HH

#include <spear/model/obj_loader.hh>
#include <spear/rendering/base_model.hh>
#include <spear/rendering/vulkan/texture/texture.hh>

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace spear::rendering::vulkan
{

class OBJModel : public BaseModel
{
public:
    /// Constructor.
    /// \param device              Logical device.
    /// \param physDevice          Physical device (memory allocation).
    /// \param object_file_path    Path to the .obj file.
    /// \param material_file_path  Path to the .mtl file (may be empty).
    /// \param texture             Texture to apply — imageView/sampler borrowed;
    ///                            the shared_ptr keeps the Vulkan objects alive.
    /// \param descriptorPool      Pool from which the descriptor set is allocated.
    /// \param descriptorSetLayout Layout with a combined-image-sampler at binding 0.
    /// \param object_data         Transform / physics data.
    OBJModel(VkDevice device,
             VkPhysicalDevice physDevice,
             const std::string& object_file_path,
             const std::string& material_file_path,
             std::shared_ptr<vulkan::Texture> texture,
             VkDescriptorPool descriptorPool,
             VkDescriptorSetLayout descriptorSetLayout,
             physics::bullet::ObjectData&& object_data);

    ~OBJModel();

    /// BaseModel::initialize implementation (no-op — setup is done in the constructor).
    void initialize() override
    {
    }

    /// Mesh::render implementation.
    void render(Camera& camera) override;

private:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
    };

    /// Expand OBJ faces into a flat interleaved vertex list for the Vulkan vertex buffer.
    std::vector<Vertex> buildVertices();
    void createVertexBuffer(VkPhysicalDevice physDevice);

private:
    OBJLoader m_loader;
    std::shared_ptr<vulkan::Texture> m_texture;

    VkDevice m_device = VK_NULL_HANDLE;
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexMemory = VK_NULL_HANDLE;
    uint32_t m_vertexCount = 0;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
};

} // namespace spear::rendering::vulkan

#endif
