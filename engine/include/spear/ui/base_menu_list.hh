#ifndef SPEAR_UI_BASE_MENU_LIST_HH
#define SPEAR_UI_BASE_MENU_LIST_HH

#include <spear/ui/base_text.hh>

#include <glm/vec2.hpp>

#include <string>
#include <vector>

namespace spear::ui
{

class BaseMenuList
{
public:
    BaseMenuList() = default;
    virtual ~BaseMenuList() = default;

    BaseMenuList(const BaseMenuList&) = delete;
    BaseMenuList& operator=(const BaseMenuList&) = delete;
    BaseMenuList(BaseMenuList&&) = delete;
    BaseMenuList& operator=(BaseMenuList&&) = delete;

    virtual void addItem(const std::string& item) = 0;
    virtual void clearItems() = 0;
    virtual void setPosition(const glm::vec2& position) = 0;
    virtual void setSpacing(float spacing) = 0;

    virtual void selectNext() = 0;
    virtual void selectPrevious() = 0;

    int getSelectedIndex() const
    {
        return m_selectedIndex;
    }

    const std::string& getSelectedItem() const;

    virtual void render(RenderContext ctx) = 0;

protected:
    glm::vec2 m_position{0.0f, 0.0f};
    float m_spacing = 50.0f;
    int m_selectedIndex = 0;
    std::vector<std::string> m_items;
};

} // namespace spear::ui

#endif
