#ifndef SPEAR_WINDOW_HH
#define SPEAR_WINDOW_HH

#include <spear/rendering/api.hh>

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>

#include <string>
#include <cstdint>

namespace spear
{

class Window
{
public:
    struct Size
    {
        uint32_t x, y;
    };

    /// Constructor.
    Window(const std::string& window_name, Size size, rendering::API api);

    /// Destructor.
    ~Window();

    SDL_Window* getSDLWindow() const
    {
        return m_window;
    }

    void update(rendering::API api);

private:
    void createWindow(const std::string& title, Size size, rendering::API api);
    void initializeContext(rendering::API api);

private:
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
    Size m_size;
    rendering::API m_api;
};

}

#endif
