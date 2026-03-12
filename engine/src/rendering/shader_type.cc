#include <spear/rendering/shader_type.hh>
#include <spear/spear_root.hh>

#include <iostream>

namespace spear::rendering
{

ShaderFileData getShaderFiles(ShaderType type, API api)
{
    switch (api)
    {
        case API::OpenGL:
        {
            switch (type)
            {
                case ShaderType::basic:
                {
                    std::string vertexPath = spearRoot() + "/shaders/basic_vertex.glsl";
                    std::string fragPath = spearRoot() + "/shaders/basic_fragment.glsl";
                    std::cout << "Selecting opengl shaders:\n"
                              << vertexPath << "\n"
                              << fragPath << "\n " << std::endl;
                    return {vertexPath, fragPath};
                }
                case ShaderType::lighting:
                {
                    std::string vertexPath = spearRoot() + "/shaders/lighting_vertex.glsl";
                    std::string fragPath = spearRoot() + "/shaders/lighting_fragment.glsl";
                    std::cout << "Selecting opengl shaders:\n"
                              << vertexPath << "\n"
                              << fragPath << "\n " << std::endl;
                    return {vertexPath, fragPath};
                }
                case ShaderType::sprite3D:
                {
                    std::string vertexPath = spearRoot() + "/shaders/sprite3d_vertex.glsl";
                    std::string fragPath = spearRoot() + "/shaders/sprite3d_fragment.glsl";
                    std::cout << "Selecting opengl shaders:\n"
                              << vertexPath << "\n"
                              << fragPath << "\n " << std::endl;
                    return {vertexPath, fragPath};
                }
                case ShaderType::quad:
                {
                    std::string vertexPath = spearRoot() + "/shaders/quad_vertex.glsl";
                    std::string fragPath = spearRoot() + "/shaders/quad_fragment.glsl";
                    std::cout << "Selecting opengl shaders:\n"
                              << vertexPath << "\n"
                              << fragPath << "\n " << std::endl;
                    return {vertexPath, fragPath};
                }
                case ShaderType::cube:
                {
                    std::string vertexPath = spearRoot() + "/shaders/cube_vertex.glsl";
                    std::string fragPath = spearRoot() + "/shaders/cube_fragment.glsl";
                    std::cout << "Selecting opengl shaders:\n"
                              << vertexPath << "\n"
                              << fragPath << "\n"
                              << std::endl;
                    return {vertexPath, fragPath};
                }
                case ShaderType::material:
                {
                    std::string vertexPath = spearRoot() + "/shaders/material_vertex.glsl";
                    std::string fragPath = spearRoot() + "/shaders/material_fragment.glsl";
                    std::cout << "Selecting opengl shaders:\n"
                              << vertexPath << "\n"
                              << fragPath << "\n"
                              << std::endl;
                    return {vertexPath, fragPath};
                }
            }
            break;
        }
        case API::Vulkan:
        {
            switch (type)
            {
                case ShaderType::basic:
                case ShaderType::lighting:
                case ShaderType::sprite3D:
                case ShaderType::quad:
                case ShaderType::cube:
                case ShaderType::material:
                {
                    std::string vertexPath = spearRoot() + "/shaders/minimal_vulkan_vertex.glsl";
                    std::string fragPath = spearRoot() + "/shaders/minimal_vulkan_fragment.glsl";
                    return {vertexPath, fragPath};
                }
            }
            break;
        }
        case API::Metal:
        case API::DirectX12:
            break;
    }
    return {"", ""};
}

} // namespace spear::rendering
