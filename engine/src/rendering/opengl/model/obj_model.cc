#include <spear/physics/bullet/object_data.hh>
#include <spear/rendering/opengl/error.hh>
#include <spear/rendering/opengl/model/obj_model.hh>
#include <spear/rendering/opengl/shader.hh>

#include <GL/glew.h>

namespace spear::rendering::opengl
{

OBJModel::Light::Light(float light_intensity, glm::vec3 light_position, glm::vec3 light_color)
    : m_lightIntensity(light_intensity),
      m_lightPosition(light_position),
      m_lightColor(light_color)
{
}

OBJModel::OBJModel(const std::string& object_file_path, const std::string& material_file_path, std::shared_ptr<BaseTexture> texture, physics::bullet::ObjectData&& object_data)
    : BaseModel(std::shared_ptr<rendering::BaseShader>(rendering::opengl::Shader::create(rendering::ShaderType::material)), std::move(object_data)),
      m_texture(texture)
{
    m_loader.load(object_file_path, material_file_path);
    initialize();
}

void OBJModel::initialize()
{
    // Build an interleaved vertex buffer by expanding each face using the correct
    // per-attribute indices. OBJ files use separate index arrays for positions, UVs,
    // and normals, so a single shared EBO cannot index all three simultaneously.
    struct Vertex
    {
        float x, y, z;
        float u, v;
        float nx, ny, nz;
    };

    const auto& positions = m_loader.getVertices();
    const auto& uvs = m_loader.getUvs();
    const auto& normals = m_loader.getNormals();

    std::vector<Vertex> vertices;
    for (const auto& face : m_loader.getFaces())
    {
        for (size_t i = 0; i < face.vertexIndices.size(); ++i)
        {
            Vertex v{};
            int vi = face.vertexIndices[i];
            v.x = positions[vi].x;
            v.y = positions[vi].y;
            v.z = positions[vi].z;

            if (!uvs.empty() && i < face.textureCoordIndices.size())
            {
                int ti = face.textureCoordIndices[i];
                v.u = uvs[ti].u;
                v.v = uvs[ti].v;
            }

            if (!normals.empty() && i < face.normalIndices.size())
            {
                int ni = face.normalIndices[i];
                v.nx = normals[ni].x;
                v.ny = normals[ni].y;
                v.nz = normals[ni].z;
            }

            vertices.push_back(v);
        }
    }

    m_vertexCount = static_cast<uint32_t>(vertices.size());

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    constexpr GLsizei stride = sizeof(Vertex);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, nx));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // Get material data.
    m_material = m_loader.getMaterial();
    rendering::opengl::openglError("OBJModel: initialize");
}

void OBJModel::render(Camera& camera)
{
    if (m_texture)
    {
        m_texture->bind();
    }

    Mesh::m_shader->use();
    glm::mat4 mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * GameObject::Transform::getModel();

    m_shader->setMat4("mvp", mvp);
    m_shader->setSampler2D("textureSampler", 0);

    m_shader->setVec3f("light.position", m_light.getLightPosition());
    m_shader->setVec3f("light.color", m_light.getLightColor());
    m_shader->setFloat("light.intensity", m_light.getLightIntensity());

    glm::vec3 cameraPosition = camera.getPosition();
    m_shader->setVec3f("cameraPosition", cameraPosition);

    // Stored material data from OBJLoader.
    m_shader->setVec3f("material.ambientColor", m_material.ambientColor);
    m_shader->setVec3f("material.diffuseColor", m_material.diffuseColor);
    m_shader->setVec3f("material.specularColor", m_material.specularColor);
    m_shader->setFloat("material.specularExponent", m_material.specularExponent);

    // Draw
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);

    // Unset, unbind
    glBindVertexArray(0);
    if (m_texture)
    {
        m_texture->unbind();
    }
    glUseProgram(0);
}

} // namespace spear::rendering::opengl
