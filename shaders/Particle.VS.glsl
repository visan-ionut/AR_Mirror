#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform float time; // Uniform for global time
uniform vec3 bezierP0; // Bezier curve control points
uniform vec3 bezierP1;
uniform vec3 bezierP2;
uniform vec3 bezierP3;

struct Particle {
    vec4 position;
    vec4 speed; // Now used to control speed along the curve
    vec4 iposition;
    vec4 ispeed;
    float startTime; // Time when the particle starts moving along the curve
};

layout(std430, binding = 0) buffer particles {
    Particle data[];
};

vec3 CalculateBezierPosition(float t, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
    float u = 1.0 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    vec3 p = uuu * p0;
    p += 3.0 * uu * t * p1;
    p += 3.0 * u * tt * p2;
    p += ttt * p3;

    return p;
}

void main() {
    // Use the x-component of speed to adjust the duration for each particle
    float adjustedDuration = 10.0 / data[gl_VertexID].speed.x;
    float t = mod(time - data[gl_VertexID].startTime, adjustedDuration) / adjustedDuration;
    t = clamp(t, 0.0, 1.0);

    vec3 bezierPosition = CalculateBezierPosition(t, bezierP0, bezierP1, bezierP2, bezierP3);

    gl_Position = Model * vec4(bezierPosition, 1.0);
}
