#ifndef SPEAR_RENDERING_VULKAN_UI_UI_RENDERER_HH
#define SPEAR_RENDERING_VULKAN_UI_UI_RENDERER_HH

#include <spear/rendering/vulkan/ui/menu_list.hh>
#include <spear/rendering/vulkan/ui/quad_2d.hh>
#include <spear/rendering/vulkan/ui/text.hh>
#include <spear/ui/base_ui_renderer.hh>

#include <memory>
#include <vector>

namespace spear::ui::vulkan
{

class UIRenderer : public spear::ui::BaseUIRenderer
{
public:
    UIRenderer(VkDevice device,
               VkPhysicalDevice physDevice,
               VkCommandPool commandPool,
               VkQueue graphicsQueue,
               VkDescriptorPool descriptorPool,
               VkDescriptorSetLayout descriptorSetLayout,
               const std::string& fontPath,
               int fontSize = 24,
               float textScale = 0.002f);

    BaseText& addText(const std::string& text, float x, float y) override;
    void addExternalText(BaseText& text) override;
    BaseQuad2D& addQuad(std::shared_ptr<spear::rendering::BaseTexture> texture, float x, float y, float w, float h) override;
    BaseMenuList& createMenuList() override;

    void clear() override;
    void render(RenderContext ctx) override;

private:
    VkDevice m_device;
    VkPhysicalDevice m_physDevice;
    VkCommandPool m_commandPool;
    VkQueue m_graphicsQueue;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;

    std::string m_fontPath;
    int m_fontSize;
    float m_textScale;

    std::vector<std::unique_ptr<Text>> m_texts;
    std::vector<BaseText*> m_externalTexts;
    std::vector<std::unique_ptr<Quad2D>> m_quads;
    std::vector<std::unique_ptr<MenuList>> m_menuLists;
};

} // namespace spear::ui::vulkan

#endif
