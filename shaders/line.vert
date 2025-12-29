#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

layout (location = 0) out vec4 lineColor;
layout (location = 1) out vec2 texCoord;

layout (set = 0, binding = 0) uniform GlobalScene
{
    mat4 view;
    mat4 projection;
    vec4 camPos;
    vec4 lightPositions[4];
    vec4 lightColors[4];
    ivec4 lightCount;
} scene;

void main() {
    gl_Position = scene.projection * scene.view * vec4(aPos, 1.0);
    lineColor = vec4(aColor, 1.0);
}
