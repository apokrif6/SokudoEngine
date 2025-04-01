#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 textCoord;
layout (location = 2) flat in uint texIndex;

layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0) uniform sampler2D tex[];

layout (set = 1, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
    //vec3 lightPos;
    //vec3 lightColor;
};

float ambientLight = 0.3;
vec3 lightPos =  vec3(0.0, 0.0, 0.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

void main() {
    float lightAngle = max(dot(normalize(normal), normalize(lightPos)), 0.0);
    //FragColor = texture(tex, textCoord) * vec4((ambientLight + 0.7 * lightAngle) * lightColor, 1.0);
    FragColor = texture(tex[texIndex], textCoord);
}