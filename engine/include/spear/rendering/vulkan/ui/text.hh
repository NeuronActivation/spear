#ifndef SPEAR_RENDERING_VULKAN_UI_TEXT_HH
#define SPEAR_RENDERING_VULKAN_UI_TEXT_HH

#include <spear/rendering/vulkan/texture/texture.hh>
#include <spear/ui/base_text.hh>

#include <SDL3_ttf/SDL_ttf.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

namespace spear::ui::vulkan
{

class Text : public spear::ui::BaseText
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
    };

    Text(VkDevice device,
         VkPhysicalDevice physDevice,
         VkCommandPool commandPool,
         VkQueue graphicsQueue,
         VkDescriptorPool descriptorPool,
         VkDescriptorSetLayout descriptorSetLayout,
         const std::string& fontPath,
         int fontSize);

    ~Text() override;

    Text(const Text&) = delete;
    Text& operator=(const Text&) = delete;
    Text(Text&& other) noexcept;
    Text& operator=(Text&& other) noexcept;

    void setString(const std::string& text) override;
    void setPosition(const glm::vec2& position) override;
    void setScale(float scale) override;
    void setColor(const SDL_Color& color) override;

    glm::vec2 getSize() const override;

    void render(RenderContext ctx) override;

private:
    void rebuildTexture();
    void rebuildQuad();

    std::vector<Vertex> buildVertices(float texWidth, float texHeight) const;

    uint32_t findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkDevice m_device;
    VkPhysicalDevice m_physDevice;
    VkCommandPool m_commandPool;
    VkQueue m_graphicsQueue;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;

    std::string m_fontPath;
    int m_fontSize;

    TTF_Font* m_font = nullptr;
    std::shared_ptr<spear::rendering::vulkan::Texture> m_texture;

    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexMemory = VK_NULL_HANDLE;
    uint32_t m_vertexCount = 0;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
};

} // namespace spear::ui::vulkan

#endif
