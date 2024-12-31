#version 450 core

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoords);
} 