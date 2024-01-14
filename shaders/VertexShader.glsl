#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Output
layout(location = 0) out vec3 fragPosition; // Position in world space
layout(location = 1) out vec3 fragNormal;   // Normal in world space

void main()
{
    // Transform the position to world space
    fragPosition = vec3(Model * vec4(v_position, 1.0));

    // Transform the normal to world space
    fragNormal = mat3(transpose(inverse(Model))) * v_normal;

    // Transform the position to clip space
    gl_Position = Projection * View * vec4(fragPosition, 1.0);
}
