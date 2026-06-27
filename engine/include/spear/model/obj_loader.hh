#ifndef SPEAR_OBJ_LOADER_HH
#define SPEAR_OBJ_LOADER_HH

#include <spear/model/model_loader.hh>

namespace spear
{

class OBJLoader : public ModelLoader
{
public:
    bool load(const std::string& obj_file_path, const std::string& material_file_path = "", bool asset_path = true) override;

    const std::vector<ModelLoader::TextureCoord>& getUvs() const
    {
        return m_textureCoords;
    }
    const std::vector<ModelLoader::Normal>& getNormals() const
    {
        return m_normals;
    }
    const std::vector<ModelLoader::MaterialEntry>& getMaterials() const
    {
        return m_materials;
    }
    const std::vector<std::vector<ModelLoader::Face>>& getFacesByMaterial() const
    {
        return m_facesByMaterial;
    }
    bool hasMaterials() const
    {
        return !m_materials.empty();
    }

private:
    bool loadMaterial(const std::string& mtlFilePath);

    /// Extract the directory portion of a file path.
    static std::string getDirectory(const std::string& filePath);

private:
    std::vector<ModelLoader::TextureCoord> m_textureCoords;
    std::vector<ModelLoader::Normal> m_normals;
    std::vector<ModelLoader::MaterialEntry> m_materials;
    std::vector<std::vector<ModelLoader::Face>> m_facesByMaterial;
    int m_currentMaterialIndex = -1;
};

} // namespace spear

#endif
