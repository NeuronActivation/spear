#ifndef SPEAR_UI_BASE_TEXT_HH
#define SPEAR_UI_BASE_TEXT_HH

#include <glm/vec2.hpp>

#include <SDL3/SDL.h>

#include <memory>
#include <string>

namespace spear::ui
{

struct RenderContext
{
    void* nativeHandle = nullptr;
};

class BaseText
{
public:
    BaseText() = default;
    virtual ~BaseText() = default;

    BaseText(const BaseText&) = delete;
    BaseText& operator=(const BaseText&) = delete;
    BaseText(BaseText&&) = delete;
    BaseText& operator=(BaseText&&) = delete;

    virtual void setString(const std::string& text) = 0;
    virtual void setPosition(const glm::vec2& position) = 0;
    virtual void setScale(float scale) = 0;
    virtual void setColor(const SDL_Color& color) = 0;

    virtual void render(RenderContext ctx) = 0;

    const std::string& getString() const
    {
        return m_string;
    }

    const glm::vec2& getPosition() const
    {
        return m_position;
    }

    float getScale() const
    {
        return m_scale;
    }

    virtual glm::vec2 getSize() const = 0;

protected:
    std::string m_string;
    glm::vec2 m_position{0.0f, 0.0f};
    float m_scale = 0.0f;
    SDL_Color m_color{255, 255, 255, 255};
};

} // namespace spear::ui

#endif
