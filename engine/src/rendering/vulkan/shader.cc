#include <spear/rendering/vulkan/shader.hh>

#include <iostream>
#include <stdexcept>

namespace spear::rendering::vulkan
{

Shader::Shader(ShaderType type, VkDevice device)
    : m_device(device)
{
    loadShaderFiles(type, API::Vulkan);
    createShaderProgram();
}

Shader::~Shader()
{
    if (m_vertexModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(m_device, m_vertexModule, nullptr);
        m_vertexModule = VK_NULL_HANDLE;
    }
    if (m_fragmentModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(m_device, m_fragmentModule, nullptr);
        m_fragmentModule = VK_NULL_HANDLE;
    }
}

Shader::Shader(Shader&& other)
    : BaseShader(std::move(other)),
      m_device(other.m_device),
      m_vertexModule(other.m_vertexModule),
      m_fragmentModule(other.m_fragmentModule)
{
    other.m_device = VK_NULL_HANDLE;
    other.m_vertexModule = VK_NULL_HANDLE;
    other.m_fragmentModule = VK_NULL_HANDLE;
}

Shader& Shader::operator=(Shader&& other)
{
    if (this != &other)
    {
        BaseShader::operator=(std::move(other));
        m_device = other.m_device;
        m_vertexModule = other.m_vertexModule;
        m_fragmentModule = other.m_fragmentModule;
        other.m_device = VK_NULL_HANDLE;
        other.m_vertexModule = VK_NULL_HANDLE;
        other.m_fragmentModule = VK_NULL_HANDLE;
    }
    return *this;
}

void Shader::createShaderProgram()
{
    std::vector<uint32_t> vertSPIRV = compileGLSLToSPIRV(m_vertexCode, shaderc_vertex_shader);
    std::vector<uint32_t> fragSPIRV = compileGLSLToSPIRV(m_fragmentCode, shaderc_fragment_shader);

    if (vertSPIRV.empty() || fragSPIRV.empty())
    {
        throw std::runtime_error("Vulkan shader compilation failed: SPIR-V is empty.");
    }

    m_vertexModule = createShaderModule(vertSPIRV);
    m_fragmentModule = createShaderModule(fragSPIRV);
}

std::array<VkPipelineShaderStageCreateInfo, 2> Shader::getShaderStages() const
{
    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = m_vertexModule;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = m_fragmentModule;
    fragStage.pName = "main";

    return {vertStage, fragStage};
}

std::vector<uint32_t> Shader::compileGLSLToSPIRV(const std::string& source, shaderc_shader_kind kind)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_size);

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, "shader", options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        throw std::runtime_error("GLSL to SPIR-V compilation failed: " +
                                 std::string(result.GetErrorMessage()));
    }

    return {result.cbegin(), result.cend()};
}

VkShaderModule Shader::createShaderModule(const std::vector<uint32_t>& spirv)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan shader module.");
    }

    return shaderModule;
}

void Shader::use()
{
    // No-op: in Vulkan, shaders are bound as part of the pipeline via vkCmdBindPipeline.
}

void Shader::setInt(const std::string& name, const int&)
{
    std::cerr << "Warning: Shader::setInt('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setFloat(const std::string& name, const float&)
{
    std::cerr << "Warning: Shader::setFloat('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setVec2f(const std::string& name, const glm::vec2&)
{
    std::cerr << "Warning: Shader::setVec2f('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setVec2i(const std::string& name, const glm::vec<2, int>&)
{
    std::cerr << "Warning: Shader::setVec2i('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setVec2ui(const std::string& name, const glm::vec<2, uint32_t>&)
{
    std::cerr << "Warning: Shader::setVec2ui('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setVec3f(const std::string& name, const glm::vec3&)
{
    std::cerr << "Warning: Shader::setVec3f('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setVec3i(const std::string& name, const glm::vec<3, int>&)
{
    std::cerr << "Warning: Shader::setVec3i('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setVec3ui(const std::string& name, const glm::vec<3, uint32_t>&)
{
    std::cerr << "Warning: Shader::setVec3ui('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setVec4f(const std::string& name, const glm::vec4&)
{
    std::cerr << "Warning: Shader::setVec4f('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setVec4i(const std::string& name, const glm::vec<4, int>&)
{
    std::cerr << "Warning: Shader::setVec4i('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setVec4ui(const std::string& name, const glm::vec<4, uint32_t>&)
{
    std::cerr << "Warning: Shader::setVec4ui('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setMat4(const std::string& name, const glm::mat4&)
{
    std::cerr << "Warning: Shader::setMat4('" << name << "') is not supported in Vulkan. Use push constants or descriptors." << std::endl;
}

void Shader::setSampler2D(const std::string& name, int)
{
    std::cerr << "Warning: Shader::setSampler2D('" << name << "') is not supported in Vulkan. Use combined image samplers via descriptors." << std::endl;
}

} // namespace spear::rendering::vulkan
