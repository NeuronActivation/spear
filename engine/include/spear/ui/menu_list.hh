#ifndef SPEAR_UI_MENU_LIST_HH
#define SPEAR_UI_MENU_LIST_HH

#include <spear/ui/text.hh>

#include <memory>
#include <string>
#include <vector>

namespace spear::ui
{

class MenuList
{
public:
    MenuList(VkDevice device,
             VkPhysicalDevice physDevice,
             VkCommandPool commandPool,
             VkQueue graphicsQueue,
             VkDescriptorPool descriptorPool,
             VkDescriptorSetLayout descriptorSetLayout,
             const std::string& fontPath,
             int fontSize);

    void addItem(const std::string& item);
    void clearItems();
    void setPosition(const glm::vec2& position);
    void setSpacing(float spacing);

    void selectNext();
    void selectPrevious();
    int getSelectedIndex() const
    {
        return m_selectedIndex;
    }
    const std::string& getSelectedItem() const;

    void render(VkCommandBuffer cmd);

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

    glm::vec2 m_position{0.0f, 0.0f};
    float m_spacing = 50.0f;
    int m_selectedIndex = 0;

    std::vector<std::string> m_items;
    std::vector<std::unique_ptr<Text>> m_texts;
};

} // namespace spear::ui

#endif
