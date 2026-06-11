#include <spear/ui/ui_renderer.hh>

namespace spear::ui
{

UIRenderer::UIRenderer(VkDevice device,
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

void UIRenderer::addText(const std::string& text, float x, float y)
{
    auto t = std::make_unique<Text>(
            m_device, m_physDevice, m_commandPool, m_graphicsQueue,
            m_descriptorPool, m_descriptorSetLayout,
            m_fontPath, m_fontSize);
    t->setString(text);
    t->setPosition(glm::vec2(x, y));
    m_texts.push_back(std::move(t));
}

void UIRenderer::addExternalText(Text& text)
{
    m_externalTexts.push_back(&text);
}

void UIRenderer::addQuad(std::shared_ptr<rendering::vulkan::Texture> texture, float x, float y, float w, float h)
{
    auto q = std::make_unique<Quad2D>(
            m_device, m_physDevice, m_descriptorPool, m_descriptorSetLayout,
            std::move(texture));
    q->setPosition(glm::vec2(x, y));
    q->setSize(glm::vec2(w, h));
    m_quads.push_back(std::move(q));
}

MenuList& UIRenderer::createMenuList()
{
    auto menu = std::make_unique<MenuList>(
            m_device, m_physDevice, m_commandPool, m_graphicsQueue,
            m_descriptorPool, m_descriptorSetLayout,
            m_fontPath, m_fontSize);
    auto* ptr = menu.get();
    m_menuLists.push_back(std::move(menu));
    return *ptr;
}

void UIRenderer::clear()
{
    m_texts.clear();
    m_externalTexts.clear();
    m_quads.clear();
    m_menuLists.clear();
}

void UIRenderer::render(VkCommandBuffer cmd)
{
    for (auto& t : m_texts)
        t->render(cmd);
    for (auto* t : m_externalTexts)
        t->render(cmd);
    for (auto& q : m_quads)
        q->render(cmd);
    for (auto& m : m_menuLists)
        m->render(cmd);
}

} // namespace spear::ui
