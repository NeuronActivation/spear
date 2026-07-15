#include <SDL3/SDL_video.h>
#include <spear/window/base_window.hh>

#include <SDL3/SDL.h>

#include <iostream>

namespace spear
{

BaseWindow::BaseWindow(const std::string& window_name, Size size)
    : m_window(nullptr),
      m_window_name(window_name),
      m_size(size)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Couldn't initialize SDL3: " << SDL_GetError() << std::endl;
        return;
    }
    std::cout << "SDL initialized" << std::endl;
}

BaseWindow::~BaseWindow()
{
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void BaseWindow::createWindow(uint32_t flags)
{
    m_window = SDL_CreateWindow(m_window_name.c_str(), m_size.x, m_size.y, flags);
    if (!m_window)
    {
        throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
    }
}

void BaseWindow::hideCursor(bool hide)
{
    if (hide)
    {
        SDL_HideCursor();
    }
    else
    {
        SDL_ShowCursor();
    }
}

void BaseWindow::setMouseGrab(bool grabbed)
{
    if (!SDL_SetWindowMouseGrab(m_window, grabbed))
    {
        std::cerr << "Window: Could not set mouse grab" << std::endl;
    }
}

void BaseWindow::setRelativeMouseMode(bool is_relative)
{
    if (!SDL_SetWindowRelativeMouseMode(m_window, is_relative))
    {
        std::cerr << "Window: Could not set relative mouse mode!" << std::endl;
    }
}

void BaseWindow::resize()
{
    SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
}

} // namespace spear
