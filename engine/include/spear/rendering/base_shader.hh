#ifndef SPEAR_SHADER_HH
#define SPEAR_SHADER_HH

#include <cstdint>
#include <memory>
#include <string>

#include <glm/mat4x4.hpp>

#include <spear/rendering/api.hh>
#include <spear/rendering/shader_type.hh>

namespace spear::rendering
{

class BaseShader
{
public:
    /// Constructor.
    BaseShader() = default;

    /// Move constructor.
    BaseShader(BaseShader&& other);

    /// Move assignment operator.
    BaseShader& operator=(BaseShader&& other);

    /// Deleted copy constructor.
    BaseShader(const BaseShader&) = delete;

    /// Deleted copy assignment operator.
    BaseShader& operator=(const BaseShader&) = delete;

    /// Destructor.
    virtual ~BaseShader();

    uint32_t getId() const
    {
        return m_id;
    }

    bool isProgramLinked() const
    {
        return m_id != 0;
    }

    virtual void setInt(const std::string&, const int&) = 0;
    virtual void setFloat(const std::string&, const float&) = 0;
    virtual void setVec2f(const std::string&, const glm::vec2&) = 0;
    virtual void setVec2i(const std::string&, const glm::vec<2, int>&) = 0;
    virtual void setVec2ui(const std::string&, const glm::vec<2, uint32_t>&) = 0;
    virtual void setVec3f(const std::string&, const glm::vec3&) = 0;
    virtual void setVec3i(const std::string&, const glm::vec<3, int>&) = 0;
    virtual void setVec3ui(const std::string&, const glm::vec<3, uint32_t>&) = 0;
    virtual void setVec4f(const std::string&, const glm::vec4&) = 0;
    virtual void setVec4i(const std::string&, const glm::vec<4, int>&) = 0;
    virtual void setVec4ui(const std::string&, const glm::vec<4, uint32_t>&) = 0;
    virtual void setMat4(const std::string&, const glm::mat4&) = 0;
    virtual void setSampler2D(const std::string& name, int textureUnit) = 0;

    virtual void use() = 0;
    virtual void createShaderProgram() = 0;

protected:
    /// Read shader files for the given type and API into m_vertexCode / m_fragmentCode.
    void loadShaderFiles(ShaderType type, API api);

    /// Read a single file into a string.
    static std::string readFile(const std::string& filepath);

    void setId(uint32_t id)
    {
        m_id = id;
    }

    std::string m_vertexCode;
    std::string m_fragmentCode;

private:
    uint32_t m_id = 0;
};

} // namespace spear::rendering

#endif
