#ifndef SPEAR_RENDERING_SHADER_TYPE_HH
#define SPEAR_RENDERING_SHADER_TYPE_HH

#include <string>

#include <spear/rendering/api.hh>

namespace spear::rendering
{

enum class ShaderType
{
    basic,
    lighting,
    sprite3D,
    quad,
    cube,
    material
};

struct ShaderFileData
{
    std::string vertex_shader, fragment_shader;
};

ShaderFileData getShaderFiles(ShaderType type, API api);

} // namespace spear::rendering

#endif
