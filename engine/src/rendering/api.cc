#include <spear/rendering/api.hh>

namespace spear::rendering
{

bool isActive(API api)
{
    switch (api)
    {
        case spear::rendering::API::OpenGL: return true;
        case spear::rendering::API::Vulkan:
        case spear::rendering::API::DirectX:
        case spear::rendering::API::Metal: break;
    }
    return false;
}

}

