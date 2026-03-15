#include <spear/rendering/base_shader.hh>
#include <spear/rendering/base_sprite_3d.hh>

#include <GL/glew.h>

namespace spear::rendering
{

BaseSprite3D::BaseSprite3D(glm::vec3 position, std::shared_ptr<rendering::BaseTexture> texture, std::shared_ptr<BaseShader> shader)
    : Mesh(shader), Transform(), m_texture(texture), m_position(position)
{
    Transform::translate(m_position);
}

} // namespace spear::rendering
