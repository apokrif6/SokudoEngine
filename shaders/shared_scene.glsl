struct PointLight
{
    vec4 position;  // xyz = position,  w = radius
    vec4 color;     // rgb = color,     a = intensity
};

const int MAX_LIGHTS = 64;
layout (set = 0, binding = 0) uniform GlobalScene
{
    mat4 view;
    mat4 projection;
    vec4 camPos;
    PointLight lights[MAX_LIGHTS];
    ivec4 lightCount;
} scene;