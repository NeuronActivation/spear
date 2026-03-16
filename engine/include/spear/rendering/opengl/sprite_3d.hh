#ifndef SPEAR_RENDERING_OPENGL_SPRITE_3D_HH
#define SPEAR_RENDERING_OPENGL_SPRITE_3D_HH

#include <spear/rendering/base_sprite_3d.hh>

namespace spear::rendering::opengl
{

class Sprite3D : public BaseSprite3D
{
public:
    /// Constructor
    Sprite3D(std::shared_ptr<rendering::BaseTexture> texture,
             physics::bullet::ObjectData&& object_data);

    /// Destructor
    ~Sprite3D();

    void render(Camera& camera) override;

    void initialize() override;

private:
    // Vertex Array Object, Vertex Buffer Object, Element Buffer Object.
    uint32_t m_vao = 0, m_vbo = 0, m_ebo = 0;

    // Vertices for a quad.
    // clang-format off
    const float m_vertices[20] = {
        // Positions        // Texture Coords
       -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // Top-left
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // Top-right
        0.5f, 0.5f,  0.0f,  1.0f, 1.0f, // Bottom-right
       -0.5f, 0.5f,  0.0f,  0.0f, 1.0f  // Bottom-left
    };

    const uint32_t m_indices[6] = {
        0, 1, 2,
        2, 3, 0
    };
    // clang-format on

    int32_t m_sampler;
};

} // namespace spear::rendering::opengl

#endif
