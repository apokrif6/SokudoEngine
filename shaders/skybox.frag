#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 texCoord;

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform samplerCube cubemapSampler;

void main() {
    outColor = texture(cubemapSampler, normalize(texCoord));
}