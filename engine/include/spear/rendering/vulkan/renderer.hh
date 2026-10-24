#ifndef SPEAR_RENDERING_VULKAN_RENDERER_HH
#define SPEAR_RENDERING_VULKAN_RENDERER_HH

#include <spear/camera.hh>
#include <spear/rendering/base_renderer.hh>
#include <spear/rendering/lighting_uniform.hh>
#include <spear/rendering/vulkan/frame_context.hh>
#include <spear/rendering/vulkan/smoke_renderer.hh>
#include <spear/rendering/vulkan/ui/ui_renderer.hh>
#include <spear/window/vulkan_window.hh>

#include <spear/rendering/vulkan/core/command_buffer_manager.hh>
#include <spear/rendering/vulkan/core/device_manager.hh>
#include <spear/rendering/vulkan/core/framebuffer_manager.hh>
#include <spear/rendering/vulkan/core/pipeline_manager.hh>
#include <spear/rendering/vulkan/core/render_pass_manager.hh>
#include <spear/rendering/vulkan/core/swapchain.hh>
#include <spear/rendering/vulkan/core/synchronization.hh>

#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.h>

#include <array>
#include <chrono>

namespace spear
{
class Scene;
}

namespace spear::rendering::vulkan
{

class Renderer : public BaseRenderer
{
public:
    /// Constructor.
    Renderer(VulkanWindow& vulkan_window);

    // Destructor.
    ~Renderer();

    void render() override;
    void setViewPort(int width, int height) override;
    void setBackgroundColor(float r, float g, float b, float a) override;
    void init() override;

    /// Initialize the textured rendering pipeline. Call once after construction,
    /// passing the descriptor set layout created via Texture::createDescriptorSetLayout().
    void initializeTexturedPipeline(VkDescriptorSetLayout descriptorSetLayout);

    /// Initialize the UI overlay pipeline. Call once after construction.
    void initializeUIPipeline(VkDescriptorSetLayout descriptorSetLayout);

    void setUIRenderer(ui::vulkan::UIRenderer* uiRenderer);
    /// Moves the sun along its default day orbit. Same as setting getLights().getSun().direction.
    void setSunTime(float seconds)
    {
        getLights().getSun().direction = sunDirectionAt(seconds);
    }
    glm::vec3 getSunDirection() const
    {
        return getLights().getSun().direction;
    }

    void drawFrame();

    void setCamera(Camera* camera)
    {
        m_camera = camera;
    }
    void setScene(Scene* scene)
    {
        m_scene = scene;
    }

    VkDevice getDevice()
    {
        return m_deviceManager.getDevice();
    }
    VkPhysicalDevice getPhysicalDevice()
    {
        return m_deviceManager.getPhysicalDevice();
    }
    VkCommandPool getCommandPool()
    {
        return m_deviceManager.getCommandPool();
    }
    VkQueue getGraphicsQueue()
    {
        return m_deviceManager.getGraphicsQueue();
    }

private:
    void cleanSwapchain();
    void cleanup();
    VkInstance createInstance();
    VkSurfaceKHR createSurface();
    void recreateSwapchain();

private:
    DeviceManager m_deviceManager;
    Swapchain m_swapchain;
    CommandBufferManager m_commandBufferManager;
    FramebufferManager m_frameBufferManager;
    PipelineManager m_pipelineManager;
    RenderPassManager m_renderPassManager;
    Synchronization m_synchronization;

    VkDescriptorSetLayout m_texturedDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_uiDescriptorSetLayout = VK_NULL_HANDLE;

    uint32_t m_currentFrame = 0;
    uint32_t m_framesInFlight = 2;
    bool m_framebufferResized = false;

    Camera* m_camera = nullptr;
    Scene* m_scene = nullptr;
    ui::vulkan::UIRenderer* m_uiRenderer = nullptr;

    VkInstance m_instance;
    VkSurfaceKHR m_surface;

    // Depth buffer resources.
    VkFormat m_depthFormat = VK_FORMAT_D32_SFLOAT;
    std::vector<VkImage> m_depthImages;
    std::vector<VkDeviceMemory> m_depthMemories;
    std::vector<VkImageView> m_depthImageViews;

    VkFormat findDepthFormat();
    void createDepthResources(VkExtent2D extent);
    void cleanupDepthResources();
    void createSunResources();
    void cleanupSunResources();
    void createSunLayout();
    void createShadowRenderPass();
    void transitionShadowImages();
    void recordLighting(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    /// Draws the frame's queued transparent materials back to front, after the opaque scene.
    void recordTransparentDraws(VkCommandBuffer commandBuffer, std::vector<TransparentDraw>& draws,
                                VkDescriptorSet lightingSet);
    void recordShadowPass(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, uint32_t size,
                          const glm::mat4& lightViewProjection);
    struct PointShadow
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView cubeView = VK_NULL_HANDLE;
        std::array<VkImageView, 6> faceViews{};
        std::array<VkFramebuffer, 6> framebuffers{};
    };
    struct SunFrame
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkBuffer uniform = VK_NULL_HANDLE;
        VkDeviceMemory uniformMemory = VK_NULL_HANDLE;
        VkDescriptorSet descriptor = VK_NULL_HANDLE;
        std::array<PointShadow, LightManager::maxShadowedPointLights> pointShadows;
    };
    std::vector<SunFrame> m_sunFrames;
    VkDescriptorSetLayout m_sunLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_sunPool = VK_NULL_HANDLE;
    VkSampler m_shadowSampler = VK_NULL_HANDLE;
    VkRenderPass m_shadowRenderPass = VK_NULL_HANDLE;

    SmokeRenderer m_smokeRenderer;
    std::chrono::steady_clock::time_point m_lastFrameTime{};
    float m_smokeTime = 0.0f;
    /// Seconds since the previous frame for the particle simulation, clamped against hitches.
    float advanceSmokeClock();
};

} // namespace spear::rendering::vulkan

#endif
