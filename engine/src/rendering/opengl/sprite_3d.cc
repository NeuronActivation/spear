#include <spear/rendering/opengl/sprite_3d.hh>

#include <spear/mesh.hh>
#include <spear/rendering/opengl/shader.hh>

#include <GL/glew.h>

namespace spear::rendering::opengl
{

Sprite3D::Sprite3D(glm::vec3 position, std::shared_ptr<rendering::BaseTexture> texture)
    : BaseSprite3D(position, texture, std::shared_ptr<BaseShader>(Shader::create(ShaderType::sprite3D))),
      m_sampler(GL_TEXTURE_2D)
{
}

Sprite3D::~Sprite3D()
{
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)
        glDeleteBuffers(1, &m_vbo);
    if (m_ebo)
        glDeleteBuffers(1, &m_ebo);
}

void Sprite3D::render(Camera& camera)
{
    m_texture->bind();

    Mesh::m_shader->use();
    glm::mat4 mvp = Transform::getModel() * camera.getViewMatrix() * camera.getProjectionMatrix();
    m_shader->setMat4("mvp", mvp);
    m_shader->setSampler2D("textureSampler", 0);

    // Render quad
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Sprite3D::initialize()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
} // namespace spear::rendering::opengl
