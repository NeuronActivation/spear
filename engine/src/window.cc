#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>
#include <spear/window.hh>

#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <iostream>

namespace spear
{

Window::Window(const std::string& window_name, Size size, rendering::API api) :
    m_window(nullptr), m_size(size), m_api(api)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL2: %s\n", SDL_GetError());
        return;
    }
    std::cout << "SDL initialized" << std::endl;

    createWindow(window_name, size, api);
    initializeContext(api);
}

Window::~Window()
{
    switch (m_api)
    {
        case rendering::API::OpenGL:
            SDL_GL_DestroyContext(m_glContext);
            SDL_DestroyWindow(m_window);
            break;
        case rendering::API::Vulkan:
        case rendering::API::Metal:
        case rendering::API::DirectX:
            perror("Unimplemented");
    }
    SDL_Quit();
}

void Window::createWindow(const std::string& title, Size size, rendering::API api)
{
    uint32_t flags;
    switch (api)
    {
        case rendering::API::OpenGL:
            flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
            SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "0"); // Disable OpenGL ES if on desktop
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

            m_window = SDL_CreateWindow(title.c_str(), size.x, size.y, flags);
            if (!m_window)
            {
                throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
            }
            break;
        case rendering::API::Vulkan:
            //flags = SDL_WINDOW_VULKAN;
        case rendering::API::Metal:
        case rendering::API::DirectX:
            perror("Unimplemented");
    }
}
void Window::initializeContext(rendering::API api)
{
    switch (api)
    {
        case rendering::API::OpenGL:
            m_glContext = SDL_GL_CreateContext(m_window);
            if (!m_glContext)
            {
                throw std::runtime_error("Failed to create OpenGL context: " + std::string(SDL_GetError()));
            }
            if (!SDL_GL_MakeCurrent(m_window, m_glContext))
            {
                std::cerr << "Failed to make OpenGL context current: " << SDL_GetError() << std::endl;
                return;
            }
            // vsync
            //SDL_GL_SetSwapInterval(1);
            std::cout << "OpenGL context initialized.\n";
            break;
        case rendering::API::Vulkan:
        case rendering::API::Metal:
        case rendering::API::DirectX:
            perror("Unimplemented");
    }
}

void Window::update(rendering::API api)
{
    switch (api)
    {
        case rendering::API::OpenGL:
            SDL_GL_SwapWindow(m_window);
            break;
        case rendering::API::Vulkan:
        case rendering::API::Metal:
        case rendering::API::DirectX:
            perror("Unimplemented");
    }
}

void Window::resize()
{
    SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
}

}

