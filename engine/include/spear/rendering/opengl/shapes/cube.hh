#ifndef SPEAR_RENDERING_OPENGL_SHAPES_CUBE_HH
#define SPEAR_RENDERING_OPENGL_SHAPES_CUBE_HH

#include <spear/camera.hh>
#include <spear/rendering/opengl/texture/texture.hh>
#include <spear/rendering/shapes/textured_shape.hh>

#include <vector>

namespace spear::rendering::opengl
{

class Cube : public TexturedShape
{
public:
    /// Constructor.
    /// \param texture The texture used in the cube.
    /// \param object_data The physics data associated with the cube.
    /// \param color The color of the sides of the cube.
    Cube(std::shared_ptr<rendering::BaseTexture> texture, physics::bullet::ObjectData&& object_data, const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // Mesh::render implementation.
    void render(Camera& camera) override;

    static std::unique_ptr<Cube> create(std::shared_ptr<rendering::BaseTexture> texture, physics::bullet::ObjectData&& object_data, const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        return std::make_unique<Cube>(texture, std::move(object_data), color);
    }

private:
    std::vector<float> createVertexBufferData(const glm::vec3& v);
    std::vector<float> createUvData();
    void create(std::vector<float>&& vertexBufferData, std::vector<float>&& uvData);

private:
    uint32_t m_vao;
    Texture* m_openGLTexture;
};

} // namespace spear::rendering::opengl

#endif
