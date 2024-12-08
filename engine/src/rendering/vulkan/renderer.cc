#include <spear/rendering/vulkan/renderer.hh>
#include <iostream>
#include <GL/glew.h>

namespace spear::rendering::vulkan
{

    Renderer::Renderer(SDL_Window* window) : m_window(window)
    {}

    Renderer::~Renderer()
    {}

    void Renderer::init()
    {
        m_buffer = reinterpret_cast<VkBuffer>(PFN_vkCreateBuffer());

        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            std::cerr << "GLEW initialization error: " << glewGetErrorString(err) << std::endl;
        }
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }

    void Renderer::render()
    {}

    void Renderer::setBackgroundColor(VkDevice_T *r, const VkImageCreateInfo *g, const VkAllocationCallbacks *b, VkImage *a,
                                 SDL_Renderer *pRenderer, nullptr_t pVoid, nullptr_t pVoid1)
    {
        setBackgroundColor(r, g, b, a, SDL_GetRenderer(m_window), nullptr, nullptr);
    }

}
