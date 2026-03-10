#include <spear/rendering/opengl/error.hh>
#include <spear/rendering/opengl/shader.hh>
#include <spear/rendering/opengl/shapes/cube.hh>

#include <GL/glew.h>

namespace spear::rendering::opengl
{

Cube::Cube(std::shared_ptr<rendering::BaseTexture> texture, physics::bullet::ObjectData&& object_data, const glm::vec4& color)
    : TexturedShape(std::shared_ptr<rendering::BaseShader>(rendering::opengl::Shader::create(rendering::ShaderType::cube)), texture, std::move(object_data), color)
{
    create(createVertexBufferData(object_data.getPosition()), createUvData());
}

void Cube::create(std::vector<float>&& vertex_buffer_data, std::vector<float>&& uv_data)
{
    uint32_t vertexDataSize = vertex_buffer_data.size();
    assert(vertexDataSize == 108);

    uint32_t uvDataSize = uv_data.size();
    assert(uvDataSize == 72);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    rendering::opengl::openglError("Cube::create glBindVertexArray");

    uint32_t vertexVBO;
    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, vertexDataSize * sizeof(float), vertex_buffer_data.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    uint32_t uvVBO;
    glGenBuffers(1, &uvVBO);
    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glBufferData(GL_ARRAY_BUFFER, uvDataSize * sizeof(float), uv_data.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    rendering::opengl::openglError("Cube::create VBO");

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Cube::render(Camera& camera)
{
    m_texture->bind();

    glm::mat4 mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * Shape::Transform::getModel();
    Shape::Mesh::m_shader->use();

    m_shader->setMat4("mvp", mvp);
    m_shader->setSampler2D("textureSampler", 0);
    m_shader->setVec4f("color", m_color);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Unset, unbind
    glBindVertexArray(0);
    m_texture->unbind();
    glUseProgram(0);
}

std::vector<float> Cube::createVertexBufferData(const glm::vec3& v)
{
    // clang-format off
    return std::vector<float>
    {
        -v.x, -v.y, -v.z,
         v.x, -v.y, -v.z,
         v.x,  v.y, -v.z,
         v.x,  v.y, -v.z,
        -v.x,  v.y, -v.z,
        -v.x, -v.y, -v.z,

        -v.x, -v.y,  v.z,
         v.x, -v.y,  v.z,
         v.x,  v.y,  v.z,
         v.x,  v.y,  v.z,
        -v.x,  v.y,  v.z,
        -v.x, -v.y,  v.z,

        -v.x,  v.y,  v.z,
        -v.x,  v.y, -v.z,
        -v.x, -v.y, -v.z,
        -v.x, -v.y, -v.z,
        -v.x, -v.y,  v.z,
        -v.x,  v.y,  v.z,

         v.x,  v.y,  v.z,
         v.x,  v.y, -v.z,
         v.x, -v.y, -v.z,
         v.x, -v.y, -v.z,
         v.x, -v.y,  v.z,
         v.x,  v.y,  v.z,

        -v.x, -v.y, -v.z,
         v.x, -v.y, -v.z,
         v.x, -v.y,  v.z,
         v.x, -v.y,  v.z,
        -v.x, -v.y,  v.z,
        -v.x, -v.y, -v.z,

        -v.x,  v.y, -v.z,
         v.x,  v.y, -v.z,
         v.x,  v.y,  v.z,
         v.x,  v.y,  v.z,
        -v.x,  v.y,  v.z,
        -v.x,  v.y, -v.z
    };
    // clang-format on
}

std::vector<float> Cube::createUvData()
{
    // clang-format off
    return std::vector<float>
    {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,

        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
    };
    // clang-format on
}

} // namespace spear::rendering::opengl
