#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec4 aColor;
layout (location = 4) in vec2 aUV;
layout (location = 5) in vec4 aWeights;
layout (location = 6) in ivec4 aBoneIDs;

layout (location = 0) out vec3 worldPos;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec2 textCoord;
layout (location = 3) out vec3 tangent;
layout (location = 4) out vec4 vertColor;

const int MAX_BONES = 200;

layout (set = 1, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

layout (set = 3, binding = 0) uniform Bones {
    mat4 bones[MAX_BONES];
};

layout (set = 4, binding = 0) uniform Model {
    mat4 model;
};

layout (push_constant) uniform PrimitiveFlagsPushConstants {
    int useSkinning;
} pushConstants;

void main() {
    mat4 boneTransform = mat4(1.0f);

    if (pushConstants.useSkinning == 1)
    {
        boneTransform  = bones[aBoneIDs[0]] * aWeights[0];
        boneTransform += bones[aBoneIDs[1]] * aWeights[1];
        boneTransform += bones[aBoneIDs[2]] * aWeights[2];
        boneTransform += bones[aBoneIDs[3]] * aWeights[3];
    }

    vec4 animPos = boneTransform * vec4(aPos, 1.0);

    vec4 worldPosition = model * animPos;
    worldPos = worldPosition.xyz;

    normal = normalize(mat3(model) * mat3(boneTransform) * aNormal);

    tangent = normalize(mat3(model) * mat3(boneTransform) * aTangent);

    gl_Position = projection * view * worldPosition;

    textCoord = aUV;
    vertColor = aColor;
}