#version 460 core

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 textCoord;

layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0) uniform sampler2D tex;

layout (set = 1, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};


void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}