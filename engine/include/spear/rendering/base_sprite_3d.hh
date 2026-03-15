#ifndef SPEAR_SPRITE_3D_HH
#define SPEAR_SPRITE_3D_HH

#include <spear/rendering/base_texture.hh>

#include <spear/mesh.hh>
#include <spear/transform.hh>

namespace spear::rendering
{

class BaseSprite3D : public Mesh, public Transform
{
public:
    // Constructor with texture.
    BaseSprite3D(glm::vec3 position, std::shared_ptr<BaseTexture> texture, std::shared_ptr<BaseShader> shader);

    // Destructor.
    virtual ~BaseSprite3D() {};

    virtual void initialize() = 0;

    /// Mesh::render implementation.
    virtual void render(Camera& camera) = 0;

    void setPosition(const glm::vec3& newPosition)
    {
        m_position = newPosition;
    }

    void setRotation(float newRotation)
    {
        m_rotation = newRotation;
    }

protected:
    // Texture
    std::shared_ptr<BaseTexture> m_texture;

private:
    // Input data.
    glm::vec3 m_position;
    float m_rotation; // degrees
};

} // namespace spear::rendering

#endif
