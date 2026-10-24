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
    void initializeTextured(VkDevice device,
                            VkRenderPass renderPass,
                            VkExtent2D extent,
                            VkDescriptorSetLayout descriptorSetLayout);
    void initializeUI(VkDevice device,
                      VkRenderPass renderPass,
                      VkExtent2D extent,
                      VkDescriptorSetLayout descriptorSetLayout);
    void initializeShadow(VkDevice device, VkRenderPass renderPass);
    /// Smoke simulation (compute) and billboard (graphics, subpass 1) pipelines.
    void initializeSmoke(VkDevice device, VkRenderPass renderPass, VkDescriptorSetLayout smokeLayout);
    void cleanup(VkDevice device);

    VkPipeline getPipeline() const
    {
        return m_graphicsPipeline;
    }
    VkPipelineLayout getPipelineLayout() const
    {
        return m_pipelineLayout;
    }

    VkPipeline getTexturedPipeline() const
    {
        return m_texturedPipeline;
    }
    VkPipelineLayout getTexturedPipelineLayout() const
    {
        return m_texturedPipelineLayout;
    }
    /// Textured pipeline variants sharing the textured pipeline layout.
    VkPipeline getTexturedDoubleSidedPipeline() const
    {
        return m_texturedDoubleSidedPipeline;
    }
    VkPipeline getTransparentPipeline() const
    {
        return m_transparentPipeline;
    }
    VkPipeline getTransparentDoubleSidedPipeline() const
    {
        return m_transparentDoubleSidedPipeline;
    }

    VkPipeline getUIPipeline() const
    {
        return m_uiPipeline;
    }
    VkPipeline getShadowColorPipeline() const
    {
        return m_shadowColorPipeline;
    }
    VkPipeline getShadowTexturedPipeline() const
    {
        return m_shadowTexturedPipeline;
    }
    VkPipelineLayout getShadowPipelineLayout() const
    {
        return m_shadowPipelineLayout;
    }
    void setSunLayout(VkDescriptorSetLayout layout)
    {
        m_sunLayout = layout;
    }
    VkPipeline getSmokeComputePipeline() const
    {
        return m_smokeComputePipeline;
    }
    VkPipelineLayout getSmokeComputePipelineLayout() const
    {
        return m_smokeComputePipelineLayout;
    }
    VkPipeline getSmokePipeline() const
    {
        return m_smokePipeline;
    }
    VkPipelineLayout getSmokePipelineLayout() const
    {
        return m_smokePipelineLayout;
    }
    VkPipelineLayout getUIPipelineLayout() const
    {
        return m_uiPipelineLayout;
    }

private:
    std::string readGLSLFile(const std::string& filepath);
    std::vector<uint32_t> compileGLSLToSPIRV(const std::string& source, shaderc_shader_kind kind);
    VkShaderModule createShaderModule(VkDevice device, const std::vector<uint32_t>& spirv);

private:
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

    VkPipeline m_texturedPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_texturedPipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_texturedDoubleSidedPipeline = VK_NULL_HANDLE;
    VkPipeline m_transparentPipeline = VK_NULL_HANDLE;
    VkPipeline m_transparentDoubleSidedPipeline = VK_NULL_HANDLE;

    VkPipeline m_uiPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_uiPipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_sunLayout = VK_NULL_HANDLE;
    VkPipeline m_shadowColorPipeline = VK_NULL_HANDLE;
    VkPipeline m_shadowTexturedPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_shadowPipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_smokeComputePipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_smokeComputePipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_smokePipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_smokePipelineLayout = VK_NULL_HANDLE;
};

} // namespace spear::rendering::vulkan

#endif
