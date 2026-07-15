#ifndef SPEAR_RENDERING_VULKAN_UI_MENU_LIST_HH
#define SPEAR_RENDERING_VULKAN_UI_MENU_LIST_HH

#include <spear/rendering/vulkan/ui/text.hh>
#include <spear/ui/base_menu_list.hh>

#include <memory>
#include <string>
#include <vector>

namespace spear::ui::vulkan
{

class MenuList : public spear::ui::BaseMenuList
{
public:
    MenuList(VkDevice device,
             VkPhysicalDevice physDevice,
             VkCommandPool commandPool,
             VkQueue graphicsQueue,
             VkDescriptorPool descriptorPool,
             VkDescriptorSetLayout descriptorSetLayout,
             const std::string& fontPath,
             int fontSize,
             float textScale = 0.002f);

    void addItem(const std::string& item) override;
    void clearItems() override;
    void setPosition(const glm::vec2& position) override;
    void setSpacing(float spacing) override;

    void selectNext() override;
    void selectPrevious() override;

    void render(RenderContext ctx) override;

private:
    void rebuildItems();

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
};

} // namespace spear::ui::vulkan

#endif
