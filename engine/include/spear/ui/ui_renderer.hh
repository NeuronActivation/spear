#ifndef SPEAR_UI_UI_RENDERER_HH
#define SPEAR_UI_UI_RENDERER_HH

#include <spear/ui/menu_list.hh>
#include <spear/ui/quad_2d.hh>
#include <spear/ui/text.hh>

#include <memory>
#include <vector>

namespace spear::ui
{

class UIRenderer
{
public:
    UIRenderer(VkDevice device,
               VkPhysicalDevice physDevice,
               VkCommandPool commandPool,
               VkQueue graphicsQueue,
               VkDescriptorPool descriptorPool,
               VkDescriptorSetLayout descriptorSetLayout,
               const std::string& fontPath,
               int fontSize = 24);

    void addText(const std::string& text, float x, float y);
    void addExternalText(Text& text);
    void addQuad(std::shared_ptr<rendering::vulkan::Texture> texture, float x, float y, float w, float h);

    MenuList& createMenuList();

    void clear();
    void render(VkCommandBuffer cmd);

private:
    VkDevice m_device;
    VkPhysicalDevice m_physDevice;
    VkCommandPool m_commandPool;
    VkQueue m_graphicsQueue;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;

    std::string m_fontPath;
    int m_fontSize;

    std::vector<std::unique_ptr<Text>> m_texts;
    std::vector<Text*> m_externalTexts;
    std::vector<std::unique_ptr<Quad2D>> m_quads;
    std::vector<std::unique_ptr<MenuList>> m_menuLists;
};

} // namespace spear::ui

#endif
