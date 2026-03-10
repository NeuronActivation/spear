#ifndef SPEAR_MATERIAL_HH
#define SPEAR_MATERIAL_HH

#include <spear/rendering/base_shader.hh>
#include <spear/rendering/base_texture.hh>

#include <memory>

namespace spear
{

class Material
{
public:
    Material(std::shared_ptr<rendering::BaseShader> shader,
             std::shared_ptr<rendering::BaseTexture> texture);

    virtual ~Material() = default;

    virtual void use() = 0;

protected:
    std::shared_ptr<rendering::BaseShader> m_shader;
    std::shared_ptr<rendering::BaseTexture> m_texture;
};

} // namespace spear

#endif
