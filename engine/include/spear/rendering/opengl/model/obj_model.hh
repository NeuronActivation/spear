#ifndef SPEAR_RENDERING_OPENGL_MODEL_OBJ_MODEL_HH
#define SPEAR_RENDERING_OPENGL_MODEL_OBJ_MODEL_HH

#include <spear/model/obj_loader.hh>
#include <spear/rendering/base_model.hh>
#include <spear/rendering/base_texture.hh>

#include <spear/physics/bullet/object_data.hh>

namespace spear::rendering::opengl
{

class OBJModel : public BaseModel
{
public:
    class Light
    {
    public:
        /// Constructor.
        Light(float light_intensity, glm::vec3 light_position, glm::vec3 light_color);

        // Default constructor.
        Light() = default;

        float getLightIntensity() const
        {
            return m_lightIntensity;
        }

        glm::vec3 getLightPosition() const
        {
            return m_lightPosition;
        }

        glm::vec3 getLightColor() const
        {
            return m_lightColor;
        }

    private:
        float m_lightIntensity = 1.5f;
        glm::vec3 m_lightPosition = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 m_lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    };

    /// Constructor.
    OBJModel(const std::string& object_file_path, const std::string& material_file_path, std::shared_ptr<BaseTexture> texture, physics::bullet::ObjectData&& object_data);

    /// Model::initialize implementation.
    void initialize() override;

    /// Mesh::render implementation.
    void render(Camera& camera) override;

private:
    OBJLoader m_loader;
    std::shared_ptr<BaseTexture> m_texture;

    uint32_t m_vao;
    uint32_t m_vertexCount = 0;

    ModelLoader::Material m_material;

    Light m_light;
};

} // namespace spear::rendering::opengl

#endif
