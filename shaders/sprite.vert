#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "shared_scene.glsl"

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aTangent;
layout (location = 3) in vec4 aColor;
layout (location = 4) in vec2 aUV;

layout (location = 0) out vec2 outUV;

layout (set = 1, binding = 0) uniform PrimitiveData
{
    mat4 model;
    // TODO
    // replace it with SpriteData and remove unused fields
    mat4 bones[200];
} primitiveData;

void main() {
    outUV = aUV;

    vec3 worldPos = primitiveData.model[3].xyz;

    vec3 cameraRight = vec3(scene.view[0][0], scene.view[1][0], scene.view[2][0]);
    vec3 cameraUp = vec3(scene.view[0][1], scene.view[1][1], scene.view[2][1]);

    vec3 vertexWorldPos = worldPos
        + cameraRight * aPos.x * primitiveData.model[0][0]
        + cameraUp * aPos.y * primitiveData.model[1][1];

    gl_Position = scene.projection * scene.view * vec4(vertexWorldPos, 1.0);
}