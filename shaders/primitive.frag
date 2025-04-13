#version 460 core

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 textCoord;
layout (location = 2) in vec4 vertColor;

layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0) uniform sampler2D tex;

layout (set = 1, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
//vec3 lightPos;
//vec3 lightColor;
};

layout (set = 2, binding = 0) uniform Material {
    vec4 baseColorFactor;
    int useTexture;
};

float ambientLight = 0.3;
vec3 lightPos =  vec3(0.0, 0.0, 0.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

void main() {
    vec4 baseColor = baseColorFactor;

    if (useTexture != 0) {
        baseColor *= texture(tex, textCoord);
    }

    float ambient = 0.3;
    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));
    float diffuse = max(dot(lightDir, normalize(vec3(0.0, 0.0, 1.0))), 0.0);
    FragColor = baseColor * (ambient + 0.7 * diffuse);
}