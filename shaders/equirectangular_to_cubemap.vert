#version 460

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec3 fragDir;

layout (set = 0, binding = 0) uniform CaptureInfo
{
    mat4 views[6];
    mat4 projection;
};

layout (push_constant) uniform PushConstants
{
    int faceIndex;
};

const vec3 cubeVertices[36] = vec3[](
    // +X
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0, -1.0),

    // -X
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0,  1.0),

    // +Y
    vec3(-1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),

    // -Y
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3(-1.0, -1.0, -1.0),

    // +Z
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),

    // -Z
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0)
);

void main()
{
    vec3 localDir = cubeVertices[gl_VertexIndex];

    fragDir = localDir;

    mat4 viewNoTranslation = mat4(mat3(views[faceIndex]));
    gl_Position = projection * viewNoTranslation * vec4(localDir, 1.0);
}
