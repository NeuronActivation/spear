#ifndef SPEAR_RENDERING_OPENGL_FRAME_CONTEXT_HH
#define SPEAR_RENDERING_OPENGL_FRAME_CONTEXT_HH

#include <spear/rendering/opengl/shader.hh>

#include <glm/mat4x4.hpp>

#include <functional>
#include <vector>

namespace spear::rendering::opengl
{

/// A blended draw deferred until all opaque geometry is drawn.
struct TransparentDraw
{
    float depth = 0.0f; ///< View-space distance used to draw back to front.
    std::function<void()> draw;
};

// Set by the renderer while rendering shadow maps, cleared afterwards.
// OpenGL meshes check this in render(Camera&) and draw depth only.
struct FrameContext
{
    bool shadowPass = false;
    glm::mat4 lightViewProjection{1.0f};
    Shader* shadowShader = nullptr;

    /// Queued while the scene draws. The renderer draws them with blending on and depth writes off.
    std::vector<TransparentDraw> transparentDraws;
};

extern FrameContext g_frameContext;

/// In a shadow pass, binds the shadow shader for the given model matrix and returns true.
/// The caller then only binds its vertex array and draws.
bool beginShadowDraw(const glm::mat4& model);

} // namespace spear::rendering::opengl

#endif
