#include "spear/rendering/base_shader.hh"
#include "spear/rendering/opengl/error.hh"
#include <spear/rendering/opengl/shader.hh>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <iostream>

namespace spear::rendering::opengl
{

Shader::Shader(ShaderType type)
{
    rendering::opengl::openglError("before opengl shader contructor");

    loadShaderFiles(type, API::OpenGL);

    rendering::opengl::openglError("before compiling shaders");

    // Compile shaders.
    const char* vShaderCode = m_vertexCode.c_str();
    const char* fShaderCode = m_fragmentCode.c_str();

    // Vertex shader.
    m_vertexId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexId, 1, &vShaderCode, nullptr);
    glCompileShader(m_vertexId);
    checkCompileErrors(m_vertexId, "VERTEX");

    // Fragment shader.
    m_fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentId, 1, &fShaderCode, nullptr);
    glCompileShader(m_fragmentId);
    checkCompileErrors(m_fragmentId, "FRAGMENT");

    rendering::opengl::openglError("after compiling shaders");

    createShaderProgram();

    // Delete the shaders.
    glDeleteShader(m_vertexId);
    glDeleteShader(m_fragmentId);

    rendering::opengl::openglError("opengl shader contructor");
}

Shader::~Shader()
{
}

Shader::Shader(Shader&& other)
    : BaseShader(std::move(other)),
      m_vertexId(std::move(other.m_vertexId)),
      m_fragmentId(std::move(other.m_fragmentId))
{
}

Shader& Shader::operator=(Shader&& other)
{
    if (this != &other)
    {
        BaseShader::operator=(std::move(other));
        m_vertexId = std::move(other.m_vertexId);
        m_fragmentId = std::move(other.m_fragmentId);
    }
    return *this;
}

void Shader::checkCompileErrors(uint32_t shader, const std::string& type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type == "PROGRAM")
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n"
                      << infoLog << std::endl;
        }
    }
    else
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        }
    }
}

int Shader::getLocation(const std::string& name)
{
    int location = glGetUniformLocation(BaseShader::getId(), name.c_str());

    if (location == -1)
    {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader program." << std::endl;
    }
    return location;
}

void Shader::setInt(const std::string& name, const int& value)
{
    int location = getLocation(name);
    glUniform1i(location, value);
}

void Shader::setFloat(const std::string& name, const float& value)
{
    int location = getLocation(name);
    glUniform1f(location, value);
}

void Shader::setVec2f(const std::string& name, const glm::vec2& vec)
{
    int location = getLocation(name);
    glUniform2f(location, vec[0], vec[1]);
}

void Shader::setVec2i(const std::string& name, const glm::vec<2, int>& vec)
{
    int location = getLocation(name);
    glUniform2i(location, vec[0], vec[1]);
}

void Shader::setVec2ui(const std::string& name, const glm::vec<2, uint32_t>& vec)
{
    int location = getLocation(name);
    glUniform2ui(location, vec[0], vec[1]);
}

void Shader::setVec3f(const std::string& name, const glm::vec3& vec)
{
    int location = getLocation(name);
    glUniform3f(location, vec[0], vec[1], vec[2]);
}

void Shader::setVec3i(const std::string& name, const glm::vec<3, int>& vec)
{
    int location = getLocation(name);
    glUniform3i(location, vec[0], vec[1], vec[2]);
}

void Shader::setVec3ui(const std::string& name, const glm::vec<3, uint32_t>& vec)
{
    int location = getLocation(name);
    glUniform3ui(location, vec[0], vec[1], vec[2]);
}

void Shader::setVec4f(const std::string& name, const glm::vec4& vec)
{
    int location = getLocation(name);
    glUniform4f(location, vec[0], vec[1], vec[2], vec[3]);
}

void Shader::setVec4i(const std::string& name, const glm::vec<4, int>& vec)
{
    int location = getLocation(name);
    glUniform4i(location, vec[0], vec[1], vec[2], vec[3]);
}

void Shader::setVec4ui(const std::string& name, const glm::vec<4, uint32_t>& vec)
{
    int location = getLocation(name);
    glUniform4ui(location, vec[0], vec[1], vec[2], vec[3]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat)
{
    int location = getLocation(name);
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
    }
}

void Shader::setSampler2D(const std::string& name, int textureUnit)
{
    GLint maxTextureUnits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

    if (textureUnit < 0 || textureUnit >= maxTextureUnits)
    {
        std::cerr << "Set sampler failed: Invalid texture unit " << textureUnit << "!" << std::endl;
        return;
    }

    int location = getLocation(name);
    if (location != -1)
    {
        glUniform1i(location, textureUnit);
    }
    else
    {
        std::cerr << "Set sampler failed!" << std::endl;
    }
}

void Shader::use()
{
    if (BaseShader::getId() == 0)
    {
        std::cerr << "Error: Shader program is not initialized." << std::endl;
        return;
    }
    glUseProgram(BaseShader::getId());
}

void Shader::createShaderProgram()
{
    int program = glCreateProgram();
    glAttachShader(program, m_vertexId);
    glAttachShader(program, m_fragmentId);
    glLinkProgram(program);

    // Check for linking errors
    int linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Shader program linking failed: " << log << std::endl;
        glDeleteProgram(program);
    }
    BaseShader::setId(program);
}

} // namespace spear::rendering::opengl
