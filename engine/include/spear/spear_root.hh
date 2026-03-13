#ifndef SPEAR_SPEAR_ROOT
#define SPEAR_SPEAR_ROOT

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
    return spearRoot() + "/assets/" + file_name;
}

static std::string getShaderPath(const std::string& shader_name = "")
{
    return spearRoot() + "/shaders/" + shader_name;
}

} // namespace spear

#endif
