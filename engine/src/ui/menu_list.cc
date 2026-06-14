#include <spear/ui/menu_list.hh>

#include <stdexcept>

namespace spear::ui
{

MenuList::MenuList(VkDevice device,
                   VkPhysicalDevice physDevice,
                   VkCommandPool commandPool,
                   VkQueue graphicsQueue,
                   VkDescriptorPool descriptorPool,
                   VkDescriptorSetLayout descriptorSetLayout,
                   const std::string& fontPath,
                   int fontSize)
    : m_device(device),
      m_physDevice(physDevice),
      m_commandPool(commandPool),
      m_graphicsQueue(graphicsQueue),
      m_descriptorPool(descriptorPool),
      m_descriptorSetLayout(descriptorSetLayout),
      m_fontPath(fontPath),
      m_fontSize(fontSize)
{
}

void MenuList::addItem(const std::string& item)
{
    m_items.push_back(item);
    rebuildItems();
}

void MenuList::clearItems()
{
    vkDeviceWaitIdle(m_device);
    m_items.clear();
    m_texts.clear();
    m_selectedIndex = 0;
}

void MenuList::setPosition(const glm::vec2& position)
{
    m_position = position;
    rebuildItems();
}

void MenuList::setSpacing(float spacing)
{
    m_spacing = spacing;
    rebuildItems();
}

void MenuList::selectNext()
{
    if (!m_items.empty())
    {
        m_selectedIndex = (m_selectedIndex + 1) % m_items.size();
        rebuildItems();
    }
}

void MenuList::selectPrevious()
{
    if (!m_items.empty())
    {
        m_selectedIndex = (m_selectedIndex - 1 + m_items.size()) % m_items.size();
        rebuildItems();
    }
}

const std::string& MenuList::getSelectedItem() const
{
    if (m_items.empty())
        throw std::runtime_error("MenuList: no items");
    return m_items[m_selectedIndex];
}

void MenuList::render(VkCommandBuffer cmd)
{
    for (auto& text : m_texts)
        text->render(cmd);
}

void MenuList::rebuildItems()
{
    // Wait for GPU to finish before destroying resources that may still be in use.
    vkDeviceWaitIdle(m_device);
    m_texts.clear();

    for (size_t i = 0; i < m_items.size(); i++)
    {
        auto text = std::make_unique<Text>(
                m_device, m_physDevice, m_commandPool, m_graphicsQueue,
                m_descriptorPool, m_descriptorSetLayout,
                m_fontPath, m_fontSize);

        // Highlight selected item in yellow
        if (static_cast<int>(i) == m_selectedIndex)
            text->setColor(SDL_Color{255, 255, 0, 255});
        else
            text->setColor(SDL_Color{255, 255, 255, 255});

        text->setString("> " + m_items[i]);
        text->setPosition(glm::vec2(m_position.x, m_position.y + static_cast<float>(i) * m_spacing * 0.002f));

        m_texts.push_back(std::move(text));
    }
}

} // namespace spear::ui
