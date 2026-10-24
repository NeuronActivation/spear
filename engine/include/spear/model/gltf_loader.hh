#ifndef SPEAR_GLTF_LOADER_HH
#define SPEAR_GLTF_LOADER_HH

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string>
#include <vector>

namespace spear
{

/// Loads glTF 2.0 models (.gltf with external or embedded buffers, and binary .glb).
///
/// The default scene is flattened: node transforms are applied to the vertices, and the
/// triangles are grouped by material so each group can be drawn with one texture.
class GLTFLoader
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec3 normal;
    };

    /// RGBA8 pixels, first row at the top of the image (the glTF UV origin).
    struct Image
    {
        int width = 0;
        int height = 0;
        std::vector<unsigned char> pixels;
    };

    /// How the base color's alpha is used, as in the glTF material's alphaMode.
    enum class AlphaMode
    {
        Opaque, ///< Alpha is ignored.
        Mask,   ///< Fragments with alpha below alphaCutoff are discarded, the rest are opaque.
        Blend,  ///< Blended over what is behind it.
    };

    struct Material
    {
        std::string name;
        AlphaMode alphaMode = AlphaMode::Opaque;
        float alphaCutoff = 0.5f;
        /// Back faces are visible too.
        bool doubleSided = false;
        /// Triangle list, counter-clockwise front faces.
        std::vector<Vertex> vertices;
        /// Base color texture with the material's base color factor multiplied in. Always at
        /// least 1x1, so materials without a texture are a single pixel of their color.
        Image baseColor;
    };

    /// Returns false if the file could not be parsed. Paths that do not exist as given
    /// are looked up in the engine's asset directory when asset_path is set.
    bool load(const std::string& file_path, bool asset_path = true);

    const std::vector<Material>& getMaterials() const
    {
        return m_materials;
    }

private:
    std::vector<Material> m_materials;
};

} // namespace spear

#endif
