#version 460 core

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 textCoord;

layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0) uniform sampler2D tex;

layout (set = 1, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
    vec3 lightPos;
    vec3 lightColor;
};

float ambientLight = 0.3;

void main() {
    float lightAngle = max(dot(normalize(normal), normalize(lightPos)), 0.0);
    FragColor = texture(tex, textCoord) * vec4((ambientLight + 0.7 * lightAngle) * lightColor, 1.0);
}