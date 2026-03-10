#ifndef SPEAR_RENDERING_OPENGL_MATERIAL_HH
#define SPEAR_RENDERING_OPENGL_MATERIAL_HH

#include <spear/material.hh>
#include <spear/rendering/opengl/shader.hh>
#include <spear/rendering/opengl/texture/texture.hh>

#include <memory>

namespace spear::rendering::opengl
{

class Material : public spear::Material
{
public:
    Material(std::shared_ptr<Shader> shader, std::shared_ptr<Texture> texture);

    void use() override;

    int32_t getColor() const
    {
        return m_color;
    }
    int32_t getMvp() const
    {
        return m_mvp;
    }
    int32_t getSampler() const
    {
        return m_sampler;
    }

private:
    std::shared_ptr<Texture> m_glTexture;
    int32_t m_mvp = -1;
    int32_t m_color = -1;
    int32_t m_sampler = -1;
};

} // namespace spear::rendering::opengl

#endif
