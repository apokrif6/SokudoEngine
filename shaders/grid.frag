#version 460 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 nearPoint;

layout(location = 1) in vec3 farPoint;

void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    outColor = vec4(1.0, 0.0, 0.0, 1.0 * float (t > 0));
}
