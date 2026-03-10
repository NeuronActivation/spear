#include "spear/physics/bullet/object_data.hh"
#include <GL/glew.h>
#include <cmath>
#include <spear/rendering/opengl/shader.hh>
#include <spear/rendering/opengl/shapes/sphere.hh>

namespace spear::rendering::opengl
{

Sphere::Sphere(std::shared_ptr<rendering::BaseTexture> texture, physics::bullet::ObjectData&& object_data, const glm::vec4& color, int latitudeBands, int longitudeBands)
    : TexturedShape(std::shared_ptr<rendering::BaseShader>(rendering::opengl::Shader::create(rendering::ShaderType::cube)), texture, std::move(object_data), color)
{
    auto vertex_buffer_data = createVertexBufferData(latitudeBands, longitudeBands, object_data.getPosition());
    auto uv_data = createUvData(latitudeBands, longitudeBands);
    auto indices = createIndexBufferData(latitudeBands, longitudeBands);

    create(std::move(vertex_buffer_data), std::move(uv_data), std::move(indices));
}

void Sphere::create(std::vector<float>&& vertex_buffer_data, std::vector<float>&& uv_data, std::vector<uint32_t>&& indices)
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    uint32_t vertexVBO;
    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(float), vertex_buffer_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    uint32_t uvVBO;
    glGenBuffers(1, &uvVBO);
    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glBufferData(GL_ARRAY_BUFFER, uv_data.size() * sizeof(float), uv_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    m_numVertices = indices.size();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

std::vector<float> Sphere::createVertexBufferData(int latitudeBands, int longitudeBands, const glm::vec3& position)
{
    std::vector<float> vertices;
    for (int lat = 0; lat <= latitudeBands; ++lat)
    {
        float theta = lat * glm::pi<float>() / latitudeBands;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= longitudeBands; ++lon)
        {
            float phi = lon * 2.0f * glm::pi<float>() / longitudeBands;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            // Calculate normalized spherical coordinates.
            glm::vec3 normal = glm::vec3(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
            glm::vec3 vertex = normal * position;

            vertices.emplace_back(vertex.x);
            vertices.emplace_back(vertex.y);
            vertices.emplace_back(vertex.z);
        }
    }

    return vertices;
}

std::vector<float> Sphere::createUvData(int latitudeBands, int longitudeBands)
{
    std::vector<float> uvs;
    for (int lat = 0; lat <= latitudeBands; ++lat)
    {
        for (int lon = 0; lon <= longitudeBands; ++lon)
        {
            float u = static_cast<float>(lon) / longitudeBands;
            float v = static_cast<float>(lat) / latitudeBands;
            uvs.emplace_back(u);
            uvs.emplace_back(v);
        }
    }
    return uvs;
}

std::vector<uint32_t> Sphere::createIndexBufferData(uint32_t segments, uint32_t rings)
{
    std::vector<uint32_t> indices;

    for (uint32_t ring = 0; ring < rings - 1; ++ring)
    {
        for (uint32_t segment = 0; segment < segments; ++segment)
        {
            // Current vertex and next vertex indices
            uint32_t current = ring * segments + segment;
            uint32_t next = current + segments;

            // Connect vertices to form two triangles (quad)
            indices.emplace_back(current);
            indices.emplace_back(next);
            indices.emplace_back(current + 1);

            indices.emplace_back(current + 1);
            indices.emplace_back(next);
            indices.emplace_back(next + 1);
        }
    }

    return indices;
}

void Sphere::render(Camera& camera)
{
    m_texture->bind();

    glm::mat4 mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * Shape::Transform::getModel();
    Shape::Mesh::m_shader->use();

    m_shader->setMat4("mvp", mvp);
    m_shader->setSampler2D("textureSampler", 0);
    m_shader->setVec4f("color", m_color);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_numVertices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    m_texture->unbind();
    glUseProgram(0);
}

} // namespace spear::rendering::opengl
