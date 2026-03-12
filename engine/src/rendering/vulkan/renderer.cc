#include <spear/rendering/vulkan/frame_context.hh>
#include <spear/rendering/vulkan/renderer.hh>
#include <spear/scene.hh>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <array>
#include <iostream>

namespace spear::rendering::vulkan
{

Renderer::Renderer(VulkanWindow& vulkan_window)
    : BaseRenderer(vulkan_window)
{
    auto window_size = vulkan_window.getSize();
    m_instance = vulkan_window.createVulkanInstance();
    m_surface = vulkan_window.createVulkanSurface(m_instance);

    m_deviceManager.initialize(m_instance, m_surface);
    m_swapchain.initialize(m_deviceManager.getDevice(), m_deviceManager.getPhysicalDevice(), m_surface, window_size.x, window_size.y);
    m_renderPassManager.initialize(m_deviceManager.getDevice(), m_swapchain.getFormat());
    m_frameBufferManager.initialize(m_deviceManager.getDevice(), m_renderPassManager.getRenderPass(), m_swapchain.getImageViews(), m_swapchain.getExtent());
    m_pipelineManager.initialize(m_deviceManager.getDevice(), m_renderPassManager.getRenderPass(), m_swapchain.getExtent());
    m_commandBufferManager.initialize(m_deviceManager.getDevice(), m_deviceManager.getCommandPool(), m_swapchain.getImageCount());
    m_synchronization.initialize(m_deviceManager.getDevice(), m_framesInFlight);
}

Renderer::~Renderer()
{
    cleanup();
}

void Renderer::init()
{
}

void Renderer::initializeTexturedPipeline(VkDescriptorSetLayout descriptorSetLayout)
{
    m_texturedDescriptorSetLayout = descriptorSetLayout;
    m_pipelineManager.initializeTextured(m_deviceManager.getDevice(),
                                         m_renderPassManager.getRenderPass(),
                                         m_swapchain.getExtent(),
                                         descriptorSetLayout);
}

void Renderer::render()
{
    drawFrame();
}

void Renderer::drawFrame()
{
    std::cout << "CurrentFrame: " << m_currentFrame << std::endl;
    auto* device = m_deviceManager.getDevice();
    if (device == VK_NULL_HANDLE)
    {
        std::cerr << "Device is null" << std::endl;
        return;
    }
    VkQueue graphicsQueue = m_deviceManager.getGraphicsQueue();
    VkQueue presentQueue = m_deviceManager.getPresentQueue();

    auto* fence = m_synchronization.getInFlightFence(m_currentFrame);
    if (fence == VK_NULL_HANDLE)
    {
        std::cerr << "Fence is null" << std::endl;
        return;
    }

    std::cout << "Before drawFrame vkWaitForFences" << std::endl;
    VkResult waitResult = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
    if (waitResult != VK_SUCCESS)
    {
        std::cerr << "Failed to wait for fence! Error code: " << waitResult << std::endl;
        return;
    }

    std::cout << "Before drawFrame vkResetFences" << std::endl;
    VkResult resetResult = vkResetFences(device, 1, &fence);
    if (resetResult != VK_SUCCESS)
    {
        std::cerr << "Failed to reset fence! Error code: " << resetResult << std::endl;
        return;
    }

    // Acquire an image from the swap chain
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device,
                                            m_swapchain.getSwapchain(),
                                            UINT64_MAX,
                                            m_synchronization.getImageAvailableSemaphore(m_currentFrame),
                                            VK_NULL_HANDLE,
                                            &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
    {
        m_framebufferResized = false;
        recreateSwapchain();
        return;
    }
    else if (result == VK_ERROR_DEVICE_LOST)
    {
        throw std::runtime_error("Device lost during vkAcquireNextImageKHR!");
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    VkCommandBuffer commandBuffer = m_commandBufferManager.getCommandBuffers()[imageIndex];
    if (commandBuffer == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Command buffer is null!");
    }
    if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS)
    {
        std::cerr << "Failed to reset command buffer!" << std::endl;
        return;
    }

    // Record command buffer
    m_commandBufferManager.beginCommandBuffer(imageIndex);
    if (commandBuffer == VK_NULL_HANDLE)
    {
        std::cerr << "Command buffer is null" << std::endl;
        return;
    }

    auto render_pass = m_renderPassManager.getRenderPass();
    if (render_pass == VK_NULL_HANDLE)
    {
        std::cerr << "Render pass is null" << std::endl;
        return;
    }
    auto frame_buffer = m_frameBufferManager.getFrameBuffers()[imageIndex];
    if (frame_buffer == VK_NULL_HANDLE)
    {
        std::cerr << "Frame buffer is null" << std::endl;
        return;
    }
    auto extent = m_swapchain.getExtent();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer = frame_buffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    std::array<VkClearValue, 2> clearValues{};
    if (hasBackgroundColor())
    {
        auto bg_color = BaseRenderer::getBackgroundColor().value();
        clearValues[0].color = {{bg_color.r, bg_color.g, bg_color.b, bg_color.a}};
    }
    else
    {
        clearValues[0].color = {{1.0f, 1.0f, 1.0f, 1.0f}};
    }
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    if (m_pipelineManager.getPipeline() == VK_NULL_HANDLE)
        throw std::runtime_error("Color pipeline is null");

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    if (m_camera && m_scene)
    {
        g_frameContext.commandBuffer = commandBuffer;
        g_frameContext.colorPipeline = m_pipelineManager.getPipeline();
        g_frameContext.colorPipelineLayout = m_pipelineManager.getPipelineLayout();
        g_frameContext.texturedPipeline = m_pipelineManager.getTexturedPipeline();
        g_frameContext.texturedPipelineLayout = m_pipelineManager.getTexturedPipelineLayout();
        m_scene->update(*m_camera);
        g_frameContext = {};
    }

    vkCmdEndRenderPass(commandBuffer);

    m_commandBufferManager.endCommandBuffer(commandBuffer);

    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_synchronization.getImageAvailableSemaphore(m_currentFrame)};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {m_synchronization.getRenderFinishedSemaphore(m_currentFrame)};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    // Present the image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapchain.getSwapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreateSwapchain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;
}

void Renderer::cleanSwapchain()
{
    auto* device = m_deviceManager.getDevice();
    auto* command_pool = m_deviceManager.getCommandPool();

    m_frameBufferManager.cleanup(device);
    m_commandBufferManager.cleanup(device, command_pool);
    m_swapchain.cleanup(device);
}

void Renderer::cleanup()
{
    cleanSwapchain();
    auto* device = m_deviceManager.getDevice();
    m_synchronization.cleanup(device);
    m_pipelineManager.cleanup(device);
    m_renderPassManager.cleanup(device);
    m_deviceManager.cleanup();
}

void Renderer::setViewPort(int width, int height)
{
    m_framebufferResized = true;
}

void Renderer::setBackgroundColor(float r, float g, float b, float a)
{
    BaseRenderer::setBackgroundColor(glm::vec4(r, g, b, a));
}

void Renderer::recreateSwapchain()
{
    auto* device = m_deviceManager.getDevice();
    vkDeviceWaitIdle(device);
    cleanSwapchain();
    const auto& vulkan_window = *dynamic_cast<const VulkanWindow*>(&BaseRenderer::getWindow());
    auto window_size = vulkan_window.getSize();
    std::cout << "New size x: " << window_size.x << " y: " << window_size.y << std::endl;

    m_swapchain.recreate(m_deviceManager.getPhysicalDevice(), device, m_surface, window_size.x, window_size.y);
    m_pipelineManager.cleanup(device);
    m_pipelineManager.initialize(device, m_renderPassManager.getRenderPass(), m_swapchain.getExtent());
    if (m_texturedDescriptorSetLayout != VK_NULL_HANDLE)
        m_pipelineManager.initializeTextured(device, m_renderPassManager.getRenderPass(), m_swapchain.getExtent(), m_texturedDescriptorSetLayout);
    m_frameBufferManager.initialize(device, m_renderPassManager.getRenderPass(), m_swapchain.getImageViews(), m_swapchain.getExtent());
    vkDeviceWaitIdle(device);
    m_commandBufferManager.initialize(device, m_deviceManager.getCommandPool(), m_swapchain.getImageCount());

    m_synchronization.cleanup(device);
    m_synchronization.initialize(device, m_framesInFlight);
}

} // namespace spear::rendering::vulkan
