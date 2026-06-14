#ifndef SPEAR_SPEAR_ROOT
#define SPEAR_SPEAR_ROOT

#include <filesystem>
#include <string>

namespace spear
{

// Defined in CMake.
static const std::string spearRoot()
{
    return SPEAR_ROOT;
}

static std::string getAssetPath(const std::string& file_name = "")
{
    if (std::filesystem::exists(file_name))
        return file_name;
    return spearRoot() + "/assets/" + file_name;
}

static std::string getShaderPath(const std::string& shader_name = "")
{
    if (std::filesystem::exists(shader_name))
        return shader_name;
    return spearRoot() + "/shaders/" + shader_name;
}

} // namespace spear

#endif
