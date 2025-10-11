#version 460 core

layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec4 vertColor;

layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0) uniform sampler2D albedoMap;
layout (set = 0, binding = 1) uniform sampler2D normalMap;
layout (set = 0, binding = 2) uniform sampler2D metallicRoughnessMap;
layout (set = 0, binding = 3) uniform sampler2D aoMap;
layout (set = 0, binding = 4) uniform sampler2D emissiveMap;

layout (set = 1, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

layout (set = 2, binding = 0) uniform Material {
    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    vec3 emissiveFactor;
    int useAlbedoMap;
    int useNormalMap;
    int useMetallicRoughnessMap;
    int useAOMap;
    int useEmissiveMap;
};

layout (set = 5, binding = 0) uniform Camera {
    vec3 camPos;
};

const int MAX_LIGHTS = 4;
layout (set = 6, binding = 0) uniform Lights {
    vec3 lightPositions[MAX_LIGHTS];
    vec3 lightColors[MAX_LIGHTS];
    int lightCount;
};

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

mat3 calculateTBN(vec3 N, vec3 T)
{
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    return mat3(T, B, N);
}


void main() {
    vec3 N = normalize(normal);
    if (useNormalMap != 0)
    {
        mat3 TBN = calculateTBN(N, tangent);
        vec3 normalFromMap = texture(normalMap, textCoord).rgb * 2.0 - 1.0;
        N = normalize(TBN * normalFromMap);
    }

    vec3 V = normalize(camPos - worldPos);

    vec3 albedo = baseColorFactor.rgb;
    if (useAlbedoMap != 0)
    {
        albedo *= texture(albedoMap, textCoord).rgb;
    }

    float metallic = 0.0;
    float roughness = 0.5;
    if (useMetallicRoughnessMap != 0)
    {
        vec4 mr = texture(metallicRoughnessMap, textCoord);
        metallic = metallicFactor * mr.b;
        roughness = roughnessFactor * mr.g;
    }

    float ao = 1.0;
    if (useAOMap != 0)
    {
        ao = texture(aoMap, textCoord).r;
    }

    vec3 emissive = vec3(0.0);
    if (useEmissiveMap != 0)
    {
        emissive = emissiveFactor * texture(emissiveMap, textCoord).rgb;
    }

    vec3 Lo = vec3(0.0);
    for (int i = 0 ; i < lightCount; i++)
    {
        vec3 L = normalize(lightPositions[i] - worldPos);
        vec3 H = normalize(V + L);

        float distance = length(lightPositions[i] - worldPos);
        float attenuation = 1.0 / (distance*distance);
        vec3 radiance = lightColors[i] * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), mix(vec3(0.04), albedo, metallic));
        vec3 specular = NDF * G * F / (4*max(dot(N, V), 0.0)*max(dot(N, L), 0.0)+0.001);

        vec3 kS = F;
        vec3 kD = vec3(1.0)-kS;
        kD *= 1.0-metallic;

        Lo += (kD*albedo/PI + specular) * radiance * max(dot(N, L), 0.0);
    }

    vec3 ambient = 0.03 * albedo * ao;
    vec3 color = ambient + Lo + emissive;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
