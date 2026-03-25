#version 460 core

#extension GL_GOOGLE_include_directive : require
#include "shared_scene.glsl"

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

layout (location = 0) out vec4 lineColor;

void main() {
    gl_Position = scene.projection * scene.view * vec4(aPos, 1.0);
    lineColor = vec4(aColor, 1.0);
}
