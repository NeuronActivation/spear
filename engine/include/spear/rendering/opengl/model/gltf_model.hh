#ifndef SPEAR_RENDERING_OPENGL_MODEL_GLTF_MODEL_HH
#define SPEAR_RENDERING_OPENGL_MODEL_GLTF_MODEL_HH

#include <spear/model/gltf_loader.hh>
#include <spear/rendering/base_model.hh>

#include <spear/physics/bullet/object_data.hh>

#include <cstdint>
#include <string>
#include <vector>

namespace spear::rendering::opengl
{

/// glTF 2.0 model (.gltf or .glb) drawn with the material shader, one draw per material.
///
/// Opaque and masked materials draw with the scene. Blended materials are queued in the frame
/// context and drawn back to front after all opaque geometry; they do not cast shadows.
class GLTFModel : public BaseModel
{
public:
    GLTFModel(const std::string& file_path, physics::bullet::ObjectData&& object_data);
    ~GLTFModel();

    GLTFModel(const GLTFModel&) = delete;
    GLTFModel& operator=(const GLTFModel&) = delete;

    void initialize() override
    {
    }

    void render(Camera& camera) override;

private:
    struct MaterialRenderData
    {
        uint32_t vao = 0;
        uint32_t vbo = 0;
        uint32_t texture = 0;
        int32_t vertexCount = 0;
        GLTFLoader::AlphaMode alphaMode = GLTFLoader::AlphaMode::Opaque;
        float alphaCutoff = 0.5f;
        /// Average vertex position, for sorting blended materials.
        glm::vec3 center{0.0f};
    };

    void createMaterial(const GLTFLoader::Material& material);
    void drawMaterial(const MaterialRenderData& material, const glm::mat4& mvp);

    std::vector<MaterialRenderData> m_materialData;
};

} // namespace spear::rendering::opengl

#endif
