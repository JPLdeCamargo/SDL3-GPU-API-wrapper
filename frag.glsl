#version 450 core

layout(set = 2, binding = 0) uniform sampler2D Texture;

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 FragColor;


void main()
{
    FragColor = texture(Texture, TexCoords);
} 