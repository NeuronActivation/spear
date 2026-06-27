#ifndef SPEAR_MODEL_LOADER_HH
#define SPEAR_MODEL_LOADER_HH

#include <string>
#include <vector>

#include <glm/vec3.hpp>

namespace spear
{

class ModelLoader
{
public:
    struct Vertex
    {
        float x, y, z;
    };

    struct TextureCoord
    {
        float u, v;
    };

    struct Normal
    {
        float x, y, z;
    };

    struct Face
    {
        std::vector<int> vertexIndices;
        std::vector<int> textureCoordIndices;
        std::vector<int> normalIndices;
    };

    struct Material
    {
        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        float specularExponent;
    };

    struct MaterialEntry
    {
        std::string name;
        glm::vec3 ambientColor{0.0f};
        glm::vec3 diffuseColor{0.0f};
        glm::vec3 specularColor{0.0f};
        float specularExponent = 0.0f;
        std::string texturePath;
    };

    struct Animation
    {
        std::string name;
        // TODO
    };

    virtual ~ModelLoader() = default;

    /// Load model file.
    virtual bool load(const std::string& file_path, const std::string& material_file_path, bool asset_path) = 0;

    // Generic accessors
    virtual const std::vector<Vertex>& getVertices() const
    {
        return m_vertices;
    }
    virtual const std::vector<Face>& getFaces() const
    {
        return m_faces;
    }
    virtual const std::vector<Animation>& getAnimations() const
    {
        return m_animations;
    }

protected:
    std::vector<Vertex> m_vertices;
    std::vector<Face> m_faces;
    std::vector<Animation> m_animations;
};

} // namespace spear

#endif
