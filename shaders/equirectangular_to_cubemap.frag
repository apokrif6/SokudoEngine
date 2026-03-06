#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require
#include "pbr_utils.glsl"

layout (location = 0) in vec3 fragDir;

layout (location = 0) out vec4 FragColor;

layout (set = 1, binding = 0) uniform sampler2D equirectangularMap;

void main()
{
    vec3 dir = normalize(fragDir);
    vec2 uv = SampleSphericalMap(dir);
    vec3 color = texture(equirectangularMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}
