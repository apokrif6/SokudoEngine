#version 460 core

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec3 texCoord;

layout (set = 0, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
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

void main() {
    int idx = indices[gl_VertexIndex];
    vec3 position = positions[idx];

    mat4 viewWithoutTranslation = mat4(mat3(view));

    gl_Position = projection * viewWithoutTranslation * vec4(position, 1.0);

    texCoord = position;
}