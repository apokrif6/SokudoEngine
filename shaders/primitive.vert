#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aTangent;
layout (location = 3) in vec4 aColor;
layout (location = 4) in vec2 aUV;
layout (location = 5) in vec4 aWeights;
layout (location = 6) in ivec4 aBoneIDs;

layout (location = 0) out vec3 worldPos;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec2 textCoord;
layout (location = 3) out vec4 tangent;
layout (location = 4) out vec4 vertColor;

layout (set = 0, binding = 0) uniform GlobalScene
{
    mat4 view;
    mat4 projection;
    vec4 camPos;
    vec4 lightPositions[4];
    vec4 lightColors[4];
    ivec4 lightCount;
} scene;

const int MAX_BONES = 200;
layout (set = 1, binding = 0) uniform PrimitiveData
{
    mat4 model;
    mat4 bones[MAX_BONES];
} primitiveData;

layout (push_constant) uniform PrimitiveFlagsPushConstants
{
    int useSkinning;
} pushConstants;

void main()
{
    mat4 boneTransform = mat4(1.0f);

    if (pushConstants.useSkinning == 1)
    {
        boneTransform  = primitiveData.bones[aBoneIDs[0]] * aWeights[0];
        boneTransform += primitiveData.bones[aBoneIDs[1]] * aWeights[1];
        boneTransform += primitiveData.bones[aBoneIDs[2]] * aWeights[2];
        boneTransform += primitiveData.bones[aBoneIDs[3]] * aWeights[3];
    }

    vec4 animPos = boneTransform * vec4(aPos, 1.0);

    vec4 worldPosition = primitiveData.model * animPos;
    worldPos = worldPosition.xyz;

    normal = normalize(mat3(primitiveData.model) * mat3(boneTransform) * aNormal);

    tangent.xyz = normalize(normal * aTangent.xyz);
    tangent.w = aTangent.w;

    textCoord = aUV;
    vertColor = aColor;

    gl_Position = scene.projection * scene.view * worldPosition;
}