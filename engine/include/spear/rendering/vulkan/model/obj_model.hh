#ifndef SPEAR_RENDERING_VULKAN_MODEL_OBJ_MODEL_HH
#define SPEAR_RENDERING_VULKAN_MODEL_OBJ_MODEL_HH

#include <spear/model/obj_loader.hh>
#include <spear/rendering/base_model.hh>
#include <spear/rendering/vulkan/texture/stb_texture.hh>

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace spear::rendering::vulkan
{

class OBJModel : public BaseModel
{
public:
    /// Constructor.
    OBJModel(VkDevice device,
             VkPhysicalDevice physDevice,
             VkCommandPool commandPool,
             VkQueue graphicsQueue,
             const std::string& object_file_path,
             const std::string& material_file_path,
             VkDescriptorPool descriptorPool,
             VkDescriptorSetLayout descriptorSetLayout,
             physics::bullet::ObjectData&& object_data);

    ~OBJModel();

    void initialize() override
    {
    }

    void render(Camera& camera) override;

private:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
    };

    struct MaterialRenderData
    {
        std::shared_ptr<vulkan::Texture> texture;
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
        uint32_t vertexCount = 0;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    };

    std::vector<Vertex> buildVertices(const std::vector<ModelLoader::Face>& faces);
    void createMaterialBuffers(VkPhysicalDevice physDevice);

private:
    OBJLoader m_loader;
    std::vector<MaterialRenderData> m_materialData;

    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physDevice = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
};

} // namespace spear::rendering::vulkan

#endif
