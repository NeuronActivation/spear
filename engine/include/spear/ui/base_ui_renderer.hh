#ifndef SPEAR_UI_BASE_UI_RENDERER_HH
#define SPEAR_UI_BASE_UI_RENDERER_HH

#include <spear/ui/base_menu_list.hh>
#include <spear/ui/base_quad_2d.hh>
#include <spear/ui/base_text.hh>

#include <memory>
#include <string>
#include <vector>

namespace spear::rendering
{
class BaseTexture;
}

namespace spear::ui
{

class BaseUIRenderer
{
public:
    BaseUIRenderer() = default;
    virtual ~BaseUIRenderer() = default;

    BaseUIRenderer(const BaseUIRenderer&) = delete;
    BaseUIRenderer& operator=(const BaseUIRenderer&) = delete;
    BaseUIRenderer(BaseUIRenderer&&) = delete;
    BaseUIRenderer& operator=(BaseUIRenderer&&) = delete;

    virtual BaseText& addText(const std::string& text, float x, float y) = 0;
    virtual void addExternalText(BaseText& text) = 0;
    virtual BaseQuad2D& addQuad(std::shared_ptr<rendering::BaseTexture> texture, float x, float y, float w, float h) = 0;
    virtual BaseMenuList& createMenuList() = 0;

    virtual void clear() = 0;
    virtual void render(RenderContext ctx) = 0;
};

} // namespace spear::ui

#endif
