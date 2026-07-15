#ifndef SPEAR_UI_BASE_QUAD_2D_HH
#define SPEAR_UI_BASE_QUAD_2D_HH

#include <spear/ui/base_text.hh>

#include <glm/vec2.hpp>

#include <memory>

namespace spear::rendering
{
class BaseTexture;
}

namespace spear::ui
{

class BaseQuad2D
{
public:
    BaseQuad2D() = default;
    virtual ~BaseQuad2D() = default;

    BaseQuad2D(const BaseQuad2D&) = delete;
    BaseQuad2D& operator=(const BaseQuad2D&) = delete;
    BaseQuad2D(BaseQuad2D&&) = delete;
    BaseQuad2D& operator=(BaseQuad2D&&) = delete;

    virtual void setPosition(const glm::vec2& position) = 0;
    virtual void setSize(const glm::vec2& size) = 0;

    virtual void render(RenderContext ctx) = 0;

    const glm::vec2& getPosition() const
    {
        return m_position;
    }

    const glm::vec2& getSize() const
    {
        return m_size;
    }

protected:
    glm::vec2 m_position{0.0f, 0.0f};
    glm::vec2 m_size{1.0f, 1.0f};
};

} // namespace spear::ui

#endif
