#include <spear/rendering/opengl/error.hh>
#include <spear/rendering/opengl/material.hh>

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <iostream>

namespace spear::rendering::opengl
{

Material::Material(std::shared_ptr<Shader> shader, std::shared_ptr<Texture> texture)
    : spear::Material(shader, texture),
      m_glTexture(texture)
{
    if (!m_shader->isProgramLinked())
        m_shader->createShaderProgram();

    m_mvp = glGetUniformLocation(m_shader->getId(), "mvp");
    m_color = glGetUniformLocation(m_shader->getId(), "color");
    m_sampler = glGetUniformLocation(m_shader->getId(), "textureSampler");
}

void Material::use()
{
    SDL_GLContext currentContext = SDL_GL_GetCurrentContext();
    if (!currentContext)
    {
        std::cerr << "No active OpenGL context! SDL Error: " << SDL_GetError() << std::endl;
        return;
    }

    GLuint programId = m_shader->getId();
    if (!glIsProgram(programId))
    {
        std::cerr << "Invalid shader program ID: " << programId << std::endl;
        return;
    }

    GLint linkStatus = 0;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return;
    }

    glUseProgram(programId);
    openglError("glUseProgram");

    GLuint textureId = m_glTexture->getId();
    if (textureId == 0)
    {
        std::cerr << "Invalid texture ID" << std::endl;
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    openglError("glBindTexture");

    if (m_sampler != -1)
        glUniform1i(m_sampler, 0);
}

} // namespace spear::rendering::opengl
