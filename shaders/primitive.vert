#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec4 aColor;
layout (location = 4) in vec2 aUV;
layout (location = 5) in vec4 aWeights;
layout (location = 6) in ivec4 aBoneIDs;

layout (location = 0) out vec3 normal;
layout (location = 1) out vec2 textCoord;
layout (location = 2) out vec4 vertColor;

const int MAX_BONES = 200;

layout (set = 1, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

layout (set = 3, binding = 0) uniform Bones {
    mat4 bones[MAX_BONES];
};

void main() {
    mat4 boneTransform = mat4(1.0f);

    boneTransform  = bones[aBoneIDs[0]] * aWeights[0];
    boneTransform += bones[aBoneIDs[1]] * aWeights[1];
    boneTransform += bones[aBoneIDs[2]] * aWeights[2];
    boneTransform += bones[aBoneIDs[3]] * aWeights[3];

    vec4 animPos = boneTransform * vec4(aPos, 1.0);

    gl_Position = projection * view * animPos;

    normal = aNormal;
    textCoord = aUV;
}