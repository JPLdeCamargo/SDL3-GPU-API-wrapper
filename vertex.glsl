#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 text_coords;

layout (location = 0) out vec2 TexCoords;
void main()
{
    gl_Position = vec4(in_position.x, in_position.y, in_position.z, 1.0);
    TexCoords = text_coords;
}