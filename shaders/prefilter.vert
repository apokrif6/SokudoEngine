#version 460 core

layout (location = 0) out vec3 localPos;

layout (set = 0, binding = 0) uniform CaptureInfo
{
    mat4 projection;
    mat4 views[6];
};

layout (push_constant) uniform Push
{
    uint face;
    float roughness;
};

const vec3 positions[8] = vec3[8](
    vec3(-1.0, -1.0, -1.0),
    vec3(1.0, -1.0, -1.0),
    vec3(-1.0, 1.0, -1.0),
    vec3(1.0, 1.0, -1.0),
    vec3(-1.0, -1.0, 1.0),
    vec3(1.0, -1.0, 1.0),
    vec3(-1.0, 1.0, 1.0),
    vec3(1.0, 1.0, 1.0)
);

const int indices[36] = int[36](
    // Front face
    0, 1, 2, 2, 1, 3,
    // Back face
    4, 6, 5, 5, 6, 7,
    // Left face
    0, 2, 4, 4, 2, 6,
    // Right face
    1, 5, 3, 3, 5, 7,
    // Bottom face
    0, 4, 1, 1, 4, 5,
    // Top face
    2, 3, 6, 6, 3, 7
);

void main()
{
    localPos = positions[indices[gl_VertexIndex]];
    gl_Position = projection * views[face] * vec4(localPos, 1.0);
}