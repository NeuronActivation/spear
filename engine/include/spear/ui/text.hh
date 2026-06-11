#ifndef SPEAR_UI_TEXT_HH
#define SPEAR_UI_TEXT_HH

#include <spear/rendering/vulkan/texture/texture.hh>

#include <SDL3_ttf/SDL_ttf.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

namespace spear::ui
{

class Text
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

    ~Text();

    Text(const Text&) = delete;
    Text& operator=(const Text&) = delete;
    Text(Text&& other) noexcept;
    Text& operator=(Text&& other) noexcept;

    void setString(const std::string& text);
    void setPosition(const glm::vec2& position);
    void setScale(float scale);
    void setColor(const SDL_Color& color);

    const std::string& getString() const
    {
        return m_string;
    }
    const glm::vec2& getPosition() const
    {
        return m_position;
    }
    float getScale() const
    {
        return m_scale;
    }
    glm::vec2 getSize() const;

    void render(VkCommandBuffer cmd);

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

    std::string m_string;
    glm::vec2 m_position{0.0f, 0.0f};
    float m_scale = 0.002f;
    SDL_Color m_color{255, 255, 255, 255};

    TTF_Font* m_font = nullptr;
    std::shared_ptr<rendering::vulkan::Texture> m_texture;

    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexMemory = VK_NULL_HANDLE;
    uint32_t m_vertexCount = 0;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
};

} // namespace spear::ui

#endif
