#ifndef SPEAR_RENDERING_VULKAN_ERROR_HH
#define SPEAR_RENDERING_VULKAN_ERROR_HH

#include <iostream>
#include <vulkan/vulkan.h>

#ifndef SPEAR_RENDERIN_VULKAN_RENDERER
#include "renderer.hh"
#endif

namespace spear::rendering::vulkan
{

    static void vkError()
    {
        // TODO: Use Vulkan Validation Layer for error handling.

        if (Renderer::m_instance == nullptr)
        {
            std::cerr << "Vulkan error code: " << VK_ERROR_INITIALIZATION_FAILED << std::endl;
        }
    }

}

#endif
