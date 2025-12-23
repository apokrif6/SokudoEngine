#version 460

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 fragDir;

layout (location = 0) out vec4 FragColor;

layout (set = 1, binding = 0) uniform sampler2D equirectangularMap;

const float PI = 3.14159265359;

vec2 sampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(2 * PI, 1 / PI);
    uv += 0.5;
    return uv;
}

void main()
{
    vec3 dir = normalize(fragDir);
    vec2 uv = sampleSphericalMap(dir);
    vec3 color = texture(equirectangularMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}
