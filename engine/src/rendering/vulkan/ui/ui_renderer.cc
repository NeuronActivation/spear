#include <spear/rendering/vulkan/ui/ui_renderer.hh>

namespace spear::ui::vulkan
{

UIRenderer::UIRenderer(VkDevice device,
                       VkPhysicalDevice physDevice,
                       VkCommandPool commandPool,
                       VkQueue graphicsQueue,
                       VkDescriptorPool descriptorPool,
                       VkDescriptorSetLayout descriptorSetLayout,
                       const std::string& fontPath,
                       int fontSize,
                       float textScale)
    : m_device(device),
      m_physDevice(physDevice),
      m_commandPool(commandPool),
      m_graphicsQueue(graphicsQueue),
      m_descriptorPool(descriptorPool),
      m_descriptorSetLayout(descriptorSetLayout),
      m_fontPath(fontPath),
      m_fontSize(fontSize),
      m_textScale(textScale)
{
}

BaseText& UIRenderer::addText(const std::string& text, float x, float y)
{
    auto t = std::make_unique<Text>(
            m_device, m_physDevice, m_commandPool, m_graphicsQueue,
            m_descriptorPool, m_descriptorSetLayout,
            m_fontPath, m_fontSize, m_textScale);
    t->setString(text);
    t->setPosition(glm::vec2(x, y));
    auto& ref = *t;
    m_texts.push_back(std::move(t));
    return ref;
}

void UIRenderer::addExternalText(BaseText& text)
{
    m_externalTexts.push_back(&text);
}

BaseQuad2D& UIRenderer::addQuad(std::shared_ptr<spear::rendering::BaseTexture> texture, float x, float y, float w, float h)
{
    auto vulkanTexture = std::dynamic_pointer_cast<spear::rendering::vulkan::Texture>(texture);
    auto q = std::make_unique<Quad2D>(
            m_device, m_physDevice, m_descriptorPool, m_descriptorSetLayout,
            std::move(vulkanTexture));
    q->setPosition(glm::vec2(x, y));
    q->setSize(glm::vec2(w, h));
    auto& ref = *q;
    m_quads.push_back(std::move(q));
    return ref;
}

BaseMenuList& UIRenderer::createMenuList()
{
    auto menu = std::make_unique<MenuList>(
            m_device, m_physDevice, m_commandPool, m_graphicsQueue,
            m_descriptorPool, m_descriptorSetLayout,
            m_fontPath, m_fontSize, m_textScale);
    auto& ref = *menu;
    m_menuLists.push_back(std::move(menu));
    return ref;
}

void UIRenderer::clear()
{
    m_texts.clear();
    m_externalTexts.clear();
    m_quads.clear();
    m_menuLists.clear();
}

void UIRenderer::render(RenderContext ctx)
{
    for (auto& t : m_texts)
        t->render(ctx);
    for (auto* t : m_externalTexts)
        t->render(ctx);
    for (auto& q : m_quads)
        q->render(ctx);
    for (auto& m : m_menuLists)
        m->render(ctx);
}

} // namespace spear::ui::vulkan
