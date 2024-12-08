#ifndef SPEAR_RENDERING_API_HH
#define SPEAR_RENDERING_API_HH

namespace spear::rendering
{

enum API
{
    OpenGL,
    Vulkan,
    DirectX, // DirectX 12 to be precise.
    Metal
};

bool isActive(API api);

}

#endif
