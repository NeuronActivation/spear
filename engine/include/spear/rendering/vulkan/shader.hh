#ifndef SPEAR_RENDERING_VULKAN_SHADER_HH
#define SPEAR_RENDERING_VULKAN_SHADER_HH

#include <spear/rendering/base_shader.hh>
#include <spear/rendering/shader_type.hh>

#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan.h>

#include <array>
#include <memory>
#include <vector>

namespace spear::rendering::vulkan
{

class Shader : public BaseShader
{
public:
    /// Constructor. Loads, compiles, and creates shader modules for the given type.
    Shader(ShaderType type, VkDevice device);

    /// Destructor. Destroys VkShaderModules.
    ~Shader();

    /// Move constructor.
    Shader(Shader&& other);

    /// Move assignment operator.
    Shader& operator=(Shader&& other);

    /// Deleted copy constructor.
    Shader(const Shader&) = delete;

    /// Deleted copy assignment operator.
    Shader& operator=(const Shader&) = delete;

    static std::shared_ptr<Shader> create(ShaderType type, VkDevice device)
    {
        return std::make_shared<Shader>(Shader(type, device));
    }

    /// Returns the two shader stage create infos (vertex + fragment) for pipeline creation.
    std::array<VkPipelineShaderStageCreateInfo, 2> getShaderStages() const;

    VkShaderModule getVertexModule() const
    {
        return m_vertexModule;
    }
    VkShaderModule getFragmentModule() const
    {
        return m_fragmentModule;
    }

    // BaseShader interface.
    // Vulkan does not use per-draw uniform locations; use push constants or
    // descriptor sets instead. These methods log a warning and do nothing.
    void setInt(const std::string& name, const int& value) override;
    void setFloat(const std::string& name, const float& value) override;
    void setVec2f(const std::string& name, const glm::vec2& vec) override;
    void setVec2i(const std::string& name, const glm::vec<2, int>& vec) override;
    void setVec2ui(const std::string& name, const glm::vec<2, uint32_t>& vec) override;
    void setVec3f(const std::string& name, const glm::vec3& vec) override;
    void setVec3i(const std::string& name, const glm::vec<3, int>& vec) override;
    void setVec3ui(const std::string& name, const glm::vec<3, uint32_t>& vec) override;
    void setVec4f(const std::string& name, const glm::vec4& vec) override;
    void setVec4i(const std::string& name, const glm::vec<4, int>& vec) override;
    void setVec4ui(const std::string& name, const glm::vec<4, uint32_t>& vec) override;
    void setMat4(const std::string& name, const glm::mat4& mat) override;
    void setSampler2D(const std::string& name, int textureUnit) override;

    /// No-op in Vulkan — pipeline binding is done via vkCmdBindPipeline.
    void use() override;

    /// Creates the vertex and fragment VkShaderModules.
    void createShaderProgram() override;

private:
    std::vector<uint32_t> compileGLSLToSPIRV(const std::string& source, shaderc_shader_kind kind);
    VkShaderModule createShaderModule(const std::vector<uint32_t>& spirv);

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkShaderModule m_vertexModule = VK_NULL_HANDLE;
    VkShaderModule m_fragmentModule = VK_NULL_HANDLE;
};

} // namespace spear::rendering::vulkan

#endif
