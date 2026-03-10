#ifndef SPEAR_RENDERING_VULKAN_PIPELINE_MANAGER_HH
#define SPEAR_RENDERING_VULKAN_PIPELINE_MANAGER_HH

#include <vulkan/vulkan.h>

#include <shaderc/shaderc.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace spear::rendering::vulkan
{

class PipelineManager
{
public:
    void initialize(VkDevice device, VkRenderPass renderPass, VkExtent2D extent);
    void cleanup(VkDevice device);

    VkPipeline getPipeline() const
    {
        return m_graphicsPipeline;
    }

    VkPipelineLayout getPipelineLayout() const
    {
        return m_pipelineLayout;
    }

private:
    std::string readGLSLFile(const std::string& filepath);
    std::vector<uint32_t> compileGLSLToSPIRV(const std::string& source, shaderc_shader_kind kind);
    VkShaderModule createShaderModule(VkDevice device, const std::vector<uint32_t>& spirv);

private:
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};

} // namespace spear::rendering::vulkan

#endif
