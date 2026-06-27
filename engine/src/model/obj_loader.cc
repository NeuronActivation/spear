#include <spear/model/obj_loader.hh>
#include <spear/spear_root.hh>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

namespace spear
{

bool OBJLoader::load(const std::string& obj_file_path, const std::string& material_file_path, bool asset_path)
{
    std::string obj_path = asset_path ? getAssetPath(obj_file_path) : obj_file_path;
    std::ifstream file(obj_path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open OBJ file: " << obj_path << std::endl;
        return false;
    }

    if (!material_file_path.empty())
    {
        std::string mtl_path = asset_path ? getAssetPath(material_file_path) : material_file_path;
        if (!loadMaterial(mtl_path))
        {
            std::cerr << "Failed to load material file: " << mtl_path << std::endl;
        }
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream lineStream(line);
        std::string prefix;
        lineStream >> prefix;

        if (prefix == "v")
        {
            Vertex vertex;
            lineStream >> vertex.x >> vertex.y >> vertex.z;
            m_vertices.push_back(vertex);
        }
        else if (prefix == "vt")
        {
            TextureCoord texCoord;
            lineStream >> texCoord.u >> texCoord.v;
            m_textureCoords.push_back(texCoord);
        }
        else if (prefix == "vn")
        {
            Normal normal;
            lineStream >> normal.x >> normal.y >> normal.z;
            m_normals.push_back(normal);
        }
        else if (prefix == "usemtl")
        {
            std::string matName;
            lineStream >> matName;
            // Find material index by name.
            auto it = std::find_if(m_materials.begin(), m_materials.end(),
                                   [&](const MaterialEntry& m)
                                   { return m.name == matName; });
            if (it != m_materials.end())
            {
                m_currentMaterialIndex = static_cast<int>(std::distance(m_materials.begin(), it));
            }
            else
            {
                // Unknown material – create a placeholder entry.
                m_currentMaterialIndex = static_cast<int>(m_materials.size());
                m_materials.push_back({});
                m_materials.back().name = matName;
            }
            // Ensure face group exists for this material.
            while (static_cast<size_t>(m_currentMaterialIndex) >= m_facesByMaterial.size())
            {
                m_facesByMaterial.emplace_back();
            }
        }
        else if (prefix == "f")
        {
            Face face;
            std::string vertexData;
            while (lineStream >> vertexData)
            {
                std::istringstream vertexStream(vertexData);
                std::string vIndex, tIndex, nIndex;

                std::getline(vertexStream, vIndex, '/');
                std::getline(vertexStream, tIndex, '/');
                std::getline(vertexStream, nIndex, '/');

                if (!vIndex.empty())
                    face.vertexIndices.push_back(std::stoi(vIndex) - 1);
                if (!tIndex.empty())
                    face.textureCoordIndices.push_back(std::stoi(tIndex) - 1);
                if (!nIndex.empty())
                    face.normalIndices.push_back(std::stoi(nIndex) - 1);
            }

            // If no usemtl was seen, create a default material/group.
            if (m_currentMaterialIndex < 0)
            {
                m_currentMaterialIndex = 0;
                if (m_materials.empty())
                    m_materials.push_back({});
                if (m_facesByMaterial.empty())
                    m_facesByMaterial.emplace_back();
            }
            while (static_cast<size_t>(m_currentMaterialIndex) >= m_facesByMaterial.size())
            {
                m_facesByMaterial.emplace_back();
            }

            // Fan triangulation for convex polygons with 3+ vertices.
            // Triangle 0: (0, 1, 2), Triangle i: (0, i+1, i+2) for i >= 1
            for (size_t vi = 2; vi < face.vertexIndices.size(); ++vi)
            {
                Face tri;
                tri.vertexIndices = {face.vertexIndices[0], face.vertexIndices[vi - 1], face.vertexIndices[vi]};

                if (!face.textureCoordIndices.empty() && vi < face.textureCoordIndices.size())
                {
                    tri.textureCoordIndices = {face.textureCoordIndices[0], face.textureCoordIndices[vi - 1], face.textureCoordIndices[vi]};
                }
                if (!face.normalIndices.empty() && vi < face.normalIndices.size())
                {
                    tri.normalIndices = {face.normalIndices[0], face.normalIndices[vi - 1], face.normalIndices[vi]};
                }

                m_facesByMaterial[m_currentMaterialIndex].push_back(tri);
                m_faces.push_back(tri);
            }
        }
    }

    file.close();
    return true;
}

bool OBJLoader::loadMaterial(const std::string& mtlFilePath)
{
    std::ifstream file(mtlFilePath);
    if (!file.is_open())
    {
        return false;
    }

    std::string mtlDir = getDirectory(mtlFilePath);
    MaterialEntry* current = nullptr;

    m_materials.clear();

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "newmtl")
        {
            m_materials.push_back({});
            current = &m_materials.back();
            iss >> current->name;
        }
        else if (token == "Ka" && current)
        {
            iss >> current->ambientColor.r >> current->ambientColor.g >> current->ambientColor.b;
        }
        else if (token == "Kd" && current)
        {
            iss >> current->diffuseColor.r >> current->diffuseColor.g >> current->diffuseColor.b;
        }
        else if (token == "Ks" && current)
        {
            iss >> current->specularColor.r >> current->specularColor.g >> current->specularColor.b;
        }
        else if (token == "Ns" && current)
        {
            iss >> current->specularExponent;
        }
        else if (token == "map_Kd" && current)
        {
            std::string filename;
            iss >> filename;
            // Resolve relative to the MTL directory.
            current->texturePath = mtlDir + "/" + filename;
        }
    }

    return !m_materials.empty();
}

std::string OBJLoader::getDirectory(const std::string& filePath)
{
    auto pos = filePath.find_last_of("/\\");
    if (pos == std::string::npos)
        return ".";
    return filePath.substr(0, pos);
}

} // namespace spear
