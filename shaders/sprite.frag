#version 460 core

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 FragColor;

layout (set = 2, binding = 0) uniform sampler2D spriteTexture;

void main() {
    vec4 color = texture(spriteTexture, inUV);
    if (color.a < 0.1)
    {
        discard;
    }

    FragColor = color;
}