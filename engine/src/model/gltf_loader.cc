#include <spear/model/gltf_loader.hh>
#include <spear/spear_root.hh>

#include "../../third_party/cgltf.h"
#include "../../third_party/stb_image.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>

namespace spear
{

namespace
{

std::string directoryOf(const std::string& path)
{
    const auto pos = path.find_last_of("/\\");
    return pos == std::string::npos ? "." : path.substr(0, pos);
}

GLTFLoader::Image decodeImage(const unsigned char* bytes, size_t size)
{
    GLTFLoader::Image image;
    int channels = 0;
    // The OpenGL STB texture turns flipping on globally; glTF images are stored top row first.
    stbi_set_flip_vertically_on_load(false);
    unsigned char* pixels = stbi_load_from_memory(bytes, static_cast<int>(size), &image.width, &image.height,
                                                  &channels, 4);
    if (!pixels)
        return {};
    image.pixels.assign(pixels, pixels + static_cast<size_t>(image.width) * image.height * 4);
    stbi_image_free(pixels);
    return image;
}

GLTFLoader::Image loadImage(const cgltf_image* image, const cgltf_options& options, const std::string& directory)
{
    if (image->buffer_view)
    {
        // Embedded in a buffer, as in .glb files.
        const auto* view = image->buffer_view;
        const auto* bytes = static_cast<const unsigned char*>(view->buffer->data) + view->offset;
        return decodeImage(bytes, view->size);
    }
    if (!image->uri)
        return {};

    const std::string uri = image->uri;
    if (uri.rfind("data:", 0) == 0)
    {
        const auto comma = uri.find(";base64,");
        if (comma == std::string::npos)
            return {};
        const char* base64 = uri.c_str() + comma + 8;
        const size_t length = std::strlen(base64);
        const size_t padding = (length > 0 && base64[length - 1] == '=') + (length > 1 && base64[length - 2] == '=');
        const size_t size = length / 4 * 3 - padding;
        void* decoded = nullptr;
        if (cgltf_load_buffer_base64(&options, size, base64, &decoded) != cgltf_result_success)
            return {};
        auto result = decodeImage(static_cast<const unsigned char*>(decoded), size);
        std::free(decoded);
        return result;
    }

    // External file relative to the .gltf. URIs may be percent-encoded.
    std::string path = uri;
    cgltf_decode_uri(path.data());
    path.resize(std::strlen(path.c_str()));
    int width = 0, height = 0, channels = 0;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* pixels = stbi_load((directory + "/" + path).c_str(), &width, &height, &channels, 4);
    if (!pixels)
        return {};
    GLTFLoader::Image result{width, height, {}};
    result.pixels.assign(pixels, pixels + static_cast<size_t>(width) * height * 4);
    stbi_image_free(pixels);
    return result;
}

const cgltf_accessor* findAttribute(const cgltf_primitive& primitive, cgltf_attribute_type type)
{
    for (cgltf_size i = 0; i < primitive.attributes_count; ++i)
    {
        const auto& attribute = primitive.attributes[i];
        if (attribute.type == type && attribute.index == 0)
            return attribute.data;
    }
    return nullptr;
}

} // namespace

bool GLTFLoader::load(const std::string& file_path, bool asset_path)
{
    m_materials.clear();
    const std::string path = asset_path ? getAssetPath(file_path) : file_path;

    cgltf_options options{};
    cgltf_data* data = nullptr;
    if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success)
    {
        std::cerr << "GLTFLoader: failed to parse " << path << std::endl;
        return false;
    }
    if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success)
    {
        std::cerr << "GLTFLoader: failed to load buffers of " << path << std::endl;
        cgltf_free(data);
        return false;
    }

    const std::string directory = directoryOf(path);
    std::map<const cgltf_image*, Image> images;
    std::map<const cgltf_material*, size_t> materialIndices;

    auto materialFor = [&](const cgltf_material* material) -> Material&
    {
        auto found = materialIndices.find(material);
        if (found != materialIndices.end())
            return m_materials[found->second];

        Material result;
        glm::vec4 factor(1.0f);
        const cgltf_image* image = nullptr;
        if (material)
        {
            result.name = material->name ? material->name : "";
            result.doubleSided = material->double_sided;
            result.alphaCutoff = material->alpha_cutoff;
            if (material->alpha_mode == cgltf_alpha_mode_mask)
                result.alphaMode = AlphaMode::Mask;
            else if (material->alpha_mode == cgltf_alpha_mode_blend)
                result.alphaMode = AlphaMode::Blend;
            if (material->has_pbr_metallic_roughness)
            {
                const auto& pbr = material->pbr_metallic_roughness;
                factor = glm::make_vec4(pbr.base_color_factor);
                if (pbr.base_color_texture.texture)
                    image = pbr.base_color_texture.texture->image;
            }
        }

        if (image)
        {
            auto cached = images.find(image);
            if (cached == images.end())
            {
                cached = images.emplace(image, loadImage(image, options, directory)).first;
                if (cached->second.pixels.empty())
                    std::cerr << "GLTFLoader: failed to load image "
                              << (image->uri ? image->uri : image->name ? image->name
                                                                        : "(embedded)")
                              << std::endl;
            }
            result.baseColor = cached->second;
        }
        if (result.baseColor.pixels.empty())
            result.baseColor = Image{1, 1, {255, 255, 255, 255}};

        for (size_t i = 0; i < result.baseColor.pixels.size(); ++i)
        {
            const float value = result.baseColor.pixels[i] * factor[static_cast<int>(i % 4)];
            result.baseColor.pixels[i] = static_cast<unsigned char>(std::clamp(value, 0.0f, 255.0f));
        }

        materialIndices.emplace(material, m_materials.size());
        m_materials.push_back(std::move(result));
        return m_materials.back();
    };

    bool skippedPrimitives = false;
    auto addMesh = [&](const cgltf_node* node)
    {
        float world[16];
        cgltf_node_transform_world(node, world);
        const glm::mat4 model = glm::make_mat4(world);
        const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        // A mirroring transform flips the winding, so swap two corners to keep faces counter-clockwise.
        const bool mirrored = glm::determinant(glm::mat3(model)) < 0.0f;

        for (cgltf_size p = 0; p < node->mesh->primitives_count; ++p)
        {
            const auto& primitive = node->mesh->primitives[p];
            const auto* positions = findAttribute(primitive, cgltf_attribute_type_position);
            if (primitive.type != cgltf_primitive_type_triangles || !positions)
            {
                skippedPrimitives = true;
                continue;
            }
            const auto* normals = findAttribute(primitive, cgltf_attribute_type_normal);
            const auto* uvs = findAttribute(primitive, cgltf_attribute_type_texcoord);
            const cgltf_size count = primitive.indices ? primitive.indices->count : positions->count;

            auto& vertices = materialFor(primitive.material).vertices;
            for (cgltf_size i = 0; i + 2 < count; i += 3)
            {
                Vertex corners[3]{};
                for (int c = 0; c < 3; ++c)
                {
                    const cgltf_size index = primitive.indices ? cgltf_accessor_read_index(primitive.indices, i + c)
                                                               : i + c;
                    auto& v = corners[c];
                    cgltf_accessor_read_float(positions, index, glm::value_ptr(v.position), 3);
                    v.position = glm::vec3(model * glm::vec4(v.position, 1.0f));
                    if (uvs)
                        cgltf_accessor_read_float(uvs, index, glm::value_ptr(v.uv), 2);
                    if (normals)
                    {
                        cgltf_accessor_read_float(normals, index, glm::value_ptr(v.normal), 3);
                        v.normal = glm::normalize(normalMatrix * v.normal);
                    }
                }
                if (mirrored)
                    std::swap(corners[1], corners[2]);
                if (!normals)
                {
                    const glm::vec3 flat = glm::normalize(glm::cross(corners[1].position - corners[0].position,
                                                                     corners[2].position - corners[0].position));
                    for (auto& v : corners)
                        v.normal = flat;
                }
                vertices.insert(vertices.end(), corners, corners + 3);
            }
        }
    };

    auto visit = [&](auto& self, const cgltf_node* node) -> void
    {
        if (node->mesh)
            addMesh(node);
        for (cgltf_size i = 0; i < node->children_count; ++i)
            self(self, node->children[i]);
    };

    const cgltf_scene* scene = data->scene ? data->scene : (data->scenes_count > 0 ? &data->scenes[0] : nullptr);
    if (scene)
    {
        for (cgltf_size i = 0; i < scene->nodes_count; ++i)
            visit(visit, scene->nodes[i]);
    }
    else
    {
        for (cgltf_size i = 0; i < data->nodes_count; ++i)
            if (!data->nodes[i].parent)
                visit(visit, &data->nodes[i]);
    }

    if (skippedPrimitives)
        std::cerr << "GLTFLoader: " << path << " has non-triangle primitives, which are skipped" << std::endl;

    cgltf_free(data);

    // Drop materials no triangles ended up using.
    m_materials.erase(std::remove_if(m_materials.begin(), m_materials.end(),
                                     [](const Material& m)
                                     { return m.vertices.empty(); }),
                      m_materials.end());
    return true;
}

} // namespace spear
