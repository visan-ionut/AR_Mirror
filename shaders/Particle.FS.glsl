#version 430

// Input
layout(location = 0) in vec2 texture_coord;

// Uniform properties
uniform sampler2D texture_1;
uniform float time; // Uniform for global time

// Output
layout(location = 0) out vec4 out_color;

// Function to rotate texture coordinates
vec2 rotateTextureCoord(vec2 coord, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    mat2 rotationMatrix = mat2(c, -s, s, c);
    return rotationMatrix * (coord - 0.5) + 0.5; // Translate, rotate, translate back
}

void main() {
    float rotationSpeed = 1.0; // Adjust rotation speed as needed
    float rotationAngle = time * rotationSpeed; // Dynamic rotation angle

    // Rotate the texture coordinates
    vec2 rotatedCoord = rotateTextureCoord(texture_coord, rotationAngle);

    // Sample the texture with rotated coordinates
    vec4 texColor = texture(texture_1, rotatedCoord);
    out_color = vec4(texColor.rgb, 1); // Use the rotated texture color
}
