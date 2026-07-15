#ifndef SPEAR_SPEAR_ROOT
#define SPEAR_SPEAR_ROOT

#include <filesystem>
#include <string>

namespace spear
{

// Defined in CMake.
inline const std::string& spearRoot()
{
    static const std::string root = SPEAR_ROOT;
    return root;
}

inline std::string getAssetPath(const std::string& file_name = "")
{
    if (std::filesystem::exists(file_name))
        return file_name;
    return spearRoot() + "/assets/" + file_name;
}

inline std::string getShaderPath(const std::string& shader_name = "")
{
    if (std::filesystem::exists(shader_name))
        return shader_name;
    return spearRoot() + "/shaders/" + shader_name;
}

} // namespace spear

#endif
