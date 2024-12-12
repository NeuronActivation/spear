#version 450 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D textureSampler;

void main()
{
    FragColor = texture(textureSampler, TexCoords);
}
