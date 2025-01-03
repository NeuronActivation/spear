#include <spear/rendering/opengl/error.hh>
#include <spear/rendering/opengl/model/obj_model.hh>

#include <GL/glew.h>

namespace spear::rendering::opengl
{

OBJModel::OBJModel(const std::string& object_file_path, std::shared_ptr<BaseTexture> texture)
    : Model(std::shared_ptr<rendering::BaseShader>(rendering::opengl::Shader::create(rendering::ShaderType::material))),
      m_texture(texture)
{
    m_loader.load(object_file_path, "test.mtl");
    initialize();
    rendering::opengl::openglError("OBJModel: initialize");
}

// TODO part of this logic could happen in "Mesh"
void OBJModel::initialize()
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Vertex buffer
    GLuint vertexVBO;
    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, m_loader.getVertices().size() * sizeof(spear::ModelLoader::Vertex),
                 m_loader.getVertices().data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(spear::ModelLoader::Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate buffer
    if (!m_loader.getUvs().empty())
    {
        GLuint uvVBO;
        glGenBuffers(1, &uvVBO);
        glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
        glBufferData(GL_ARRAY_BUFFER, m_loader.getUvs().size() * sizeof(spear::ModelLoader::TextureCoord),
                     m_loader.getUvs().data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(spear::ModelLoader::TextureCoord), (void*)0);
        glEnableVertexAttribArray(1);
    }

    // Normal buffer
    if (!m_loader.getNormals().empty())
    {
        GLuint normalVBO;
        glGenBuffers(1, &normalVBO);
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
        glBufferData(GL_ARRAY_BUFFER, m_loader.getNormals().size() * sizeof(spear::ModelLoader::Normal),
                     m_loader.getNormals().data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(spear::ModelLoader::Normal), (void*)0);
        glEnableVertexAttribArray(2);
    }

    // Index buffer
    for (const auto& face : m_loader.getFaces())
    {
        m_indices.insert(m_indices.end(), face.vertexIndices.begin(), face.vertexIndices.end());
    }

    uint32_t ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);

    // Unbind VAO.
    glBindVertexArray(0);

    // Get material data.
    m_material = m_loader.getMaterial();
}

void OBJModel::render(Camera& camera)
{
    m_texture->bind();

    glm::mat4 mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * Entity::Transform::getModel();
    Mesh::m_shader->use();
    m_shader->setMat4("mvp", mvp);

    m_shader->setSampler2D("textureSampler", 0);

    // Light properties (example values)
    glm::vec3 lightPosition = glm::vec3(10.0f, 10.0f, 10.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float lightIntensity = 1.5f;

    m_shader->setVec3f("light.position", lightPosition);
    m_shader->setVec3f("light.color", lightColor);
    m_shader->setFloat("light.intensity", lightIntensity);

    // Camera position (for specular lighting)
    glm::vec3 cameraPosition = camera.getPosition();
    m_shader->setVec3f("cameraPosition", cameraPosition);

    // Stored material data from OBJLoader.
    m_shader->setVec3f("material.ambientColor", m_material.ambientColor);
    m_shader->setVec3f("material.diffuseColor", m_material.diffuseColor);
    m_shader->setVec3f("material.specularColor", m_material.specularColor);
    m_shader->setFloat("material.specularExponent", m_material.specularExponent);

    // Draw
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

    // Unset, unbind
    glBindVertexArray(0);
    m_texture->unbind();
    glUseProgram(0);
}

} // namespace spear::rendering::opengl
