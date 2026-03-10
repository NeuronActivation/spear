#include <spear/material.hh>

namespace spear
{

Material::Material(std::shared_ptr<rendering::BaseShader> shader,
                   std::shared_ptr<rendering::BaseTexture> texture)
    : m_shader(std::move(shader)),
      m_texture(std::move(texture))
{
}

} // namespace spear
