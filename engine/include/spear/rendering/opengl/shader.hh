#ifndef SPEAR_RENDERING_OPENGL_SHADER_HH
#define SPEAR_RENDERING_OPENGL_SHADER_HH

#include <spear/rendering/base_shader.hh>
#include <spear/rendering/shader_type.hh>

#include <memory>

namespace spear::rendering::opengl
{

class Shader : public BaseShader
{
public:
    /// Constructor.
    Shader(ShaderType shader_type);

    /// Destructor.
    ~Shader();

    /// Move constructor.
    Shader(Shader&& other);

    /// Move assignment operator.
    Shader& operator=(Shader&& other);

    /// Deleted copy constructor.
    Shader(const Shader&) = delete;

    /// Deleted copy assignment operator.
    Shader& operator=(const Shader&) = delete;

    static std::shared_ptr<Shader> create(ShaderType shader_type)
    {
        return std::make_shared<Shader>(Shader(shader_type));
    }

    // \defgroup openglShaderUniformSetters

    void setInt(const std::string&, const int&) override;
    void setFloat(const std::string&, const float&) override;
    void setVec2f(const std::string&, const glm::vec2&) override;
    void setVec2i(const std::string&, const glm::vec<2, int>&) override;
    void setVec2ui(const std::string&, const glm::vec<2, uint32_t>&) override;
    void setVec3f(const std::string&, const glm::vec3&) override;
    void setVec3i(const std::string&, const glm::vec<3, int>&) override;
    void setVec3ui(const std::string&, const glm::vec<3, uint32_t>&) override;
    void setVec4f(const std::string&, const glm::vec4&) override;
    void setVec4i(const std::string&, const glm::vec<4, int>&) override;
    void setVec4ui(const std::string&, const glm::vec<4, uint32_t>&) override;
    void setMat4(const std::string&, const glm::mat4&) override;
    void setSampler2D(const std::string& name, int textureUnit) override;

    void use() override;
    void createShaderProgram() override;

    int getVertexId() const
    {
        return m_vertexId;
    }
    int getFragmentId() const
    {
        return m_fragmentId;
    }

private:
    int getLocation(const std::string& name);
    void checkCompileErrors(uint32_t shader, const std::string& type);

private:
    uint32_t m_vertexId;
    uint32_t m_fragmentId;
};

} // namespace spear::rendering::opengl

#endif
