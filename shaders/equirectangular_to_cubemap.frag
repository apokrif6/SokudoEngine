#version 460

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 fragDir;

layout (location = 0) out vec4 FragColor;

layout (set = 1, binding = 0) uniform sampler2D equirectangularMap;

const float PI = 3.14159265359;

void main()
{
    vec3 dir = normalize(fragDir);

    vec2 uv;
    uv.x = atan(dir.z, dir.x) / (2.0 * PI) + 0.5;
    uv.y = asin(dir.y) / PI + 0.5;

    FragColor = texture(equirectangularMap, uv);
}
