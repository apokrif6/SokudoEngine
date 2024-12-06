#version 460 core

layout(set = 1, binding = 0) uniform Matrices  {
    mat4 view;
    mat4 projection;
    vec3 position;
};

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

layout(location = 0) out vec3 nearPoint;
layout(location = 1) out vec3 farPoint;

vec3 gridPlane[6] = vec3[](
vec3(1, 1, 0), vec3(-1, 1, 0), vec3(-1, -1, 0),
vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0)
);

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint = viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    vec3 p = gridPlane[gl_VertexIndex].xyz;
    nearPoint = UnprojectPoint(p.x, p.y, 0.0, view, projection).xyz;
    farPoint = UnprojectPoint(p.x, p.y, 1.0, view, projection).xyz;

    gl_Position = vec4(p, 1.0);
}