#include <spear/rendering/opengl/error.hh>
#include <spear/rendering/opengl/frame_context.hh>
#include <spear/rendering/opengl/renderer.hh>
#include <spear/scene.hh>

#include <GL/glew.h>

#include <algorithm>
#include <iostream>

namespace spear::rendering::opengl
{

namespace
{
constexpr int kShadowSize = 2048;
constexpr int kPointShadowSize = 512;

// Texture units used by shaders/lighting_common.glsl. Unit 0 is the mesh texture.
constexpr int kSunShadowUnit = 1;
constexpr int kPointShadowUnit = 2;

void setShadowCompare(GLenum target)
{
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}
} // namespace

Renderer::Renderer(OpenGLWindow& window)
    : BaseRenderer(window)
{
}

Renderer::~Renderer()
{
    if (m_lightingBuffer)
        glDeleteBuffers(1, &m_lightingBuffer);
    if (m_shadowFramebuffer)
        glDeleteFramebuffers(1, &m_shadowFramebuffer);
    if (m_sunShadowMap)
        glDeleteTextures(1, &m_sunShadowMap);
    for (auto texture : m_pointShadowMaps)
        if (texture)
            glDeleteTextures(1, &texture);
}

void Renderer::init()
{
    m_context = SDL_GL_CreateContext(getWindow().getSDLWindow());
    if (!m_context)
    {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
    }
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "GLEW initialization error: " << glewGetErrorString(err) << std::endl;
    }
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    createShadowResources();
}

void Renderer::createShadowResources()
{
    m_shadowShader = Shader::create(ShaderType::shadow);

    glGenBuffers(1, &m_lightingBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_lightingBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LightingUniform), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenTextures(1, &m_sunShadowMap);
    glBindTexture(GL_TEXTURE_2D, m_sunShadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, kShadowSize, kShadowSize, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    setShadowCompare(GL_TEXTURE_2D);
    // Outside the shadow map counts as lit.
    const float border[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glGenTextures(static_cast<GLsizei>(m_pointShadowMaps.size()), m_pointShadowMaps.data());
    for (auto texture : m_pointShadowMaps)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        for (int face = 0; face < 6; ++face)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT32F, kPointShadowSize,
                         kPointShadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        setShadowCompare(GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &m_shadowFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    openglError("Renderer::createShadowResources");
}

void Renderer::setViewPort(int width, int height)
{
    m_viewportWidth = width;
    m_viewportHeight = height;
    glViewport(0, 0, width, height);
}

void Renderer::renderShadowPass(uint32_t size, const glm::mat4& lightViewProjection)
{
    glViewport(0, 0, size, size);
    glClear(GL_DEPTH_BUFFER_BIT);
    g_frameContext.shadowPass = true;
    g_frameContext.lightViewProjection = lightViewProjection;
    g_frameContext.shadowShader = m_shadowShader.get();
    m_scene->update(*m_camera);
    g_frameContext = {};
}

void Renderer::render()
{
    LightingFrame lighting = buildLightingFrame(getLights(), m_camera,
                                                glm::vec2(m_viewportWidth, m_viewportHeight), API::OpenGL);
    lighting.uniform.shadowParams.y = 1.0f / static_cast<float>(kShadowSize);

    if (m_camera && m_scene && (lighting.sunShadow || !lighting.pointShadows.empty()))
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.75f, 1.25f);

        if (lighting.sunShadow)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_sunShadowMap, 0);
            renderShadowPass(kShadowSize, lighting.uniform.sunViewProjection);
        }
        for (const auto& point : lighting.pointShadows)
        {
            for (int face = 0; face < 6; ++face)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                       m_pointShadowMaps[point.slot], 0);
                renderShadowPass(kPointShadowSize, point.faces[face]);
            }
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        openglError("Renderer::render shadow passes");
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightingBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightingUniform), &lighting.uniform);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_lightingBuffer);

    glActiveTexture(GL_TEXTURE0 + kSunShadowUnit);
    glBindTexture(GL_TEXTURE_2D, m_sunShadowMap);
    for (size_t i = 0; i < m_pointShadowMaps.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + kPointShadowUnit + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_pointShadowMaps[i]);
    }
    glActiveTexture(GL_TEXTURE0);

    glViewport(0, 0, m_viewportWidth, m_viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_camera && m_scene)
        m_scene->update(*m_camera);

    // Transparent materials, back to front over the opaque scene.
    auto transparent = std::move(g_frameContext.transparentDraws);
    g_frameContext.transparentDraws.clear();
    if (!transparent.empty())
    {
        std::stable_sort(transparent.begin(), transparent.end(), [](const TransparentDraw& a, const TransparentDraw& b)
                         { return a.depth > b.depth; });
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        for (const auto& draw : transparent)
            draw.draw();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        openglError("Renderer::render transparent");
    }
}

void Renderer::setBackgroundColor(float r, float g, float b, float a)
{
    BaseRenderer::setBackgroundColor(glm::vec4(r, g, b, a));
    auto color = BaseRenderer::getBackgroundColor();
    glClearColor(color->r, color->g, color->b, color->a);
}

} // namespace spear::rendering::opengl
