#ifndef SPEAR_SPRITE_3D_HH
#define SPEAR_SPRITE_3D_HH

#include <spear/rendering/base_texture.hh>
#include <spear/rendering/shapes/shape.hh>

namespace spear::rendering
{

class BaseSprite3D : public Shape
{
public:
    // Constructor with texture.
    BaseSprite3D(std::shared_ptr<BaseTexture> texture,
                 std::shared_ptr<BaseShader> shader,
                 physics::bullet::ObjectData&& object_data);

    // Destructor.
    virtual ~BaseSprite3D() {};

    virtual void initialize() {};

    /// Mesh::render implementation.
    virtual void render(Camera& camera) = 0;

protected:
    // Texture
    std::shared_ptr<BaseTexture> m_texture;
};

} // namespace spear::rendering

#endif
