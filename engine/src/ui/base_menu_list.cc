#include <spear/ui/base_menu_list.hh>

#include <stdexcept>

namespace spear::ui
{

const std::string& BaseMenuList::getSelectedItem() const
{
    if (m_items.empty())
        throw std::runtime_error("MenuList: no items");
    return m_items[m_selectedIndex];
}

} // namespace spear::ui
