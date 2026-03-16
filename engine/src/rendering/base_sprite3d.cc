#include <spear/rendering/base_sprite_3d.hh>

namespace spear::rendering
{

BaseSprite3D::BaseSprite3D(std::shared_ptr<rendering::BaseTexture> texture,
                           std::shared_ptr<BaseShader> shader,
                           physics::bullet::ObjectData&& object_data)
    : Shape(shader, std::move(object_data), glm::vec4(1.0f)),
      m_texture(texture)
{
}

} // namespace spear::rendering
