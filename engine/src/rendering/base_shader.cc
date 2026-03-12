#include <spear/rendering/base_shader.hh>
#include <spear/rendering/shader_type.hh>

#include <fstream>
#include <iostream>
#include <sstream>

namespace spear::rendering
{

BaseShader::BaseShader(BaseShader&& other)
    : m_vertexCode(std::move(other.m_vertexCode)),
      m_fragmentCode(std::move(other.m_fragmentCode)),
      m_id(std::move(other.m_id))
{
}

BaseShader& BaseShader::operator=(BaseShader&& other)
{
    if (this != &other)
    {
        m_vertexCode = std::move(other.m_vertexCode);
        m_fragmentCode = std::move(other.m_fragmentCode);
        m_id = std::move(other.m_id);
    }
    return *this;
}

BaseShader::~BaseShader()
{
    // TODO
}

std::string BaseShader::readFile(const std::string& filepath)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        file.open(filepath);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        return stream.str();
    }
    catch (const std::ifstream::failure&)
    {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filepath << std::endl;
        return {};
    }
}

void BaseShader::loadShaderFiles(ShaderType type, API api)
{
    auto data = getShaderFiles(type, api);
    m_vertexCode = readFile(data.vertex_shader);
    m_fragmentCode = readFile(data.fragment_shader);
}

} // namespace spear::rendering
