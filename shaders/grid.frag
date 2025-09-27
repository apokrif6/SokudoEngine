#version 460 core

layout (location = 0) in vec3 nearPoint;
layout (location = 1) in vec3 farPoint;

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

float near = 0.1;
float far  = 100.0;

vec4 grid(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    // z axis
    if (fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.z = 1.0;
    // x axis
    if (fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.x = 1.0;

    return color;
}

float computeDepth(vec3 pos) {
    vec4 clip_space_pos = projection * view * vec4(pos.xyz, 1.0);
    return clip_space_pos.z / clip_space_pos.w;
}

float computeLinearDepth(vec3 pos) {
    vec4 clip_space_pos = projection * view * vec4(pos.xyz, 1.0);
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near));
    return linearDepth / far;
}

void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
    if (t > 0.0 && t < 1.0) {
        gl_FragDepth = computeDepth(fragPos3D);
        outColor = grid(fragPos3D, 10);

        float linearDepth = computeLinearDepth(fragPos3D);
        float fading = max(0, (0.5 - linearDepth));

        outColor = (grid(fragPos3D, 10) + grid(fragPos3D, 1)) * float(t > 0);
        outColor.a *= fading;
    } else {
        discard;
    }
}