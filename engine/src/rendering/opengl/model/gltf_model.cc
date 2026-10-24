#include <spear/rendering/opengl/error.hh>
#include <spear/rendering/opengl/frame_context.hh>
#include <spear/rendering/opengl/model/gltf_model.hh>
#include <spear/rendering/opengl/shader.hh>

#include <GL/glew.h>

#include <iostream>

namespace spear::rendering::opengl
{

GLTFModel::GLTFModel(const std::string& file_path, physics::bullet::ObjectData&& object_data)
    : BaseModel(std::shared_ptr<rendering::BaseShader>(rendering::opengl::Shader::create(rendering::ShaderType::material)), std::move(object_data))
{
    GLTFLoader loader;
    if (!loader.load(file_path))
        return;
    for (const auto& material : loader.getMaterials())
        createMaterial(material);
    rendering::opengl::openglError("GLTFModel: initialize");
    std::cout << "GLTFModel: loaded " << file_path << " with " << m_materialData.size() << " materials" << std::endl;
}

GLTFModel::~GLTFModel()
{
    for (auto& mat : m_materialData)
    {
        glDeleteVertexArrays(1, &mat.vao);
        glDeleteBuffers(1, &mat.vbo);
        glDeleteTextures(1, &mat.texture);
    }
}

void GLTFModel::createMaterial(const GLTFLoader::Material& material)
{
    // Matches the material shader's attributes: position, UV, normal.
    using Vertex = GLTFLoader::Vertex;
    MaterialRenderData data;
    data.vertexCount = static_cast<int32_t>(material.vertices.size());
    data.alphaMode = material.alphaMode;
    data.alphaCutoff = material.alphaCutoff;
    for (const auto& vertex : material.vertices)
        data.center += vertex.position;
    if (!material.vertices.empty())
        data.center /= static_cast<float>(material.vertices.size());

    glGenVertexArrays(1, &data.vao);
    glBindVertexArray(data.vao);
    glGenBuffers(1, &data.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
    glBufferData(GL_ARRAY_BUFFER, material.vertices.size() * sizeof(Vertex), material.vertices.data(), GL_STATIC_DRAW);

    constexpr GLsizei stride = sizeof(Vertex);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // glTF images are stored top row first and UV (0, 0) is the top left, so the rows are
    // uploaded as they are.
    const auto& image = material.baseColor;
    glGenTextures(1, &data.texture);
    glBindTexture(GL_TEXTURE_2D, data.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image.pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    m_materialData.push_back(data);
}

void GLTFModel::render(Camera& camera)
{
    using AlphaMode = GLTFLoader::AlphaMode;
    const glm::mat4& model = GameObject::Transform::getModel();
    if (beginShadowDraw(model))
    {
        for (const auto& mat : m_materialData)
        {
            if (mat.alphaMode == AlphaMode::Blend)
                continue;
            glBindVertexArray(mat.vao);
            glDrawArrays(GL_TRIANGLES, 0, mat.vertexCount);
        }
        glBindVertexArray(0);
        return;
    }

    const glm::mat4 view = camera.getViewMatrix();
    const glm::mat4 mvp = camera.getProjectionMatrix() * view * model;
    for (const auto& mat : m_materialData)
    {
        if (mat.alphaMode == AlphaMode::Blend)
        {
            const float depth = -(view * model * glm::vec4(mat.center, 1.0f)).z;
            g_frameContext.transparentDraws.push_back({depth, [this, &mat, mvp]()
                                                       { drawMaterial(mat, mvp); }});
            continue;
        }
        drawMaterial(mat, mvp);
    }
}

void GLTFModel::drawMaterial(const MaterialRenderData& material, const glm::mat4& mvp)
{
    Mesh::m_shader->use();
    m_shader->setMat4("mvp", mvp);
    m_shader->setSampler2D("textureSampler", 0);
    // The base color factor is already multiplied into the texture.
    m_shader->setVec3f("material.diffuseColor", glm::vec3(1.0f));
    m_shader->setFloat("alphaCutoff", material.alphaMode == GLTFLoader::AlphaMode::Mask ? material.alphaCutoff : 0.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.texture);
    glBindVertexArray(material.vao);
    glDrawArrays(GL_TRIANGLES, 0, material.vertexCount);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

} // namespace spear::rendering::opengl
