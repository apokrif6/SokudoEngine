#version 460

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 localDir;

layout (location = 0) out vec3 fragDir;

layout (set = 0, binding = 0) uniform Capture
{
    mat4 views[6];
    mat4 projection;
};

layout (push_constant) uniform PushConstants
{
    int faceIndex;
};

void main()
{
    fragDir = localDir;
    gl_Position = projection * views[faceIndex] * vec4(localDir, 1.0);
}
