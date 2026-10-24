#ifndef SPEAR_RENDERING_VULKAN_MODEL_GLTF_MODEL_HH
#define SPEAR_RENDERING_VULKAN_MODEL_GLTF_MODEL_HH

#include <spear/model/gltf_loader.hh>
#include <spear/rendering/base_model.hh>
#include <spear/rendering/vulkan/texture/stb_texture.hh>

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

namespace spear::rendering::vulkan
{

/// glTF 2.0 model (.gltf or .glb) drawn with the textured pipeline, one draw per material.
/// The descriptor pool needs one combined image sampler set per material.
///
/// Opaque and masked materials draw with the scene. Blended materials are queued in the frame
/// context and drawn back to front after all opaque geometry; they do not cast shadows.
class GLTFModel : public BaseModel
{
public:
    GLTFModel(VkDevice device,
              VkPhysicalDevice physDevice,
              VkCommandPool commandPool,
              VkQueue graphicsQueue,
              const std::string& file_path,
              VkDescriptorPool descriptorPool,
              VkDescriptorSetLayout descriptorSetLayout,
              physics::bullet::ObjectData&& object_data);

    ~GLTFModel();

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
        std::shared_ptr<STBTexture> texture;
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
        uint32_t vertexCount = 0;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        GLTFLoader::AlphaMode alphaMode = GLTFLoader::AlphaMode::Opaque;
        float alphaCutoff = 0.5f;
        bool doubleSided = false;
        /// Average vertex position, for sorting blended materials.
        glm::vec3 center{0.0f};
    };

    void createMaterial(const GLTFLoader::Material& material);

private:
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
