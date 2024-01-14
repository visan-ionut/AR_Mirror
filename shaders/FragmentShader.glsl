#version 430

// Inputs from geometry shader
in vec3 fragPosition; // Position in world space
in vec3 fragNormal;   // Normal in world space

// Uniform properties
uniform samplerCube texture_cubemap;
uniform vec3 cameraPos;
uniform vec4 contourColor; // Optional: Use if you want to blend with a solid color

// Output
layout(location = 0) out vec4 out_color;

void main() {
    // Calculate reflection vector
    vec3 viewDir = normalize(fragPosition - cameraPos);
    vec3 reflectDir = reflect(viewDir, normalize(fragNormal));

    // Sample the color from the cubemap
    vec3 cubemapColor = texture(texture_cubemap, reflectDir).rgb;

    // Combine the cubemap color with the contour color
    // The mix factor can be adjusted or based on some condition
    float mixFactor = 0.5; // Adjust this value as needed
    vec3 finalColor = mix(contourColor.rgb, cubemapColor, mixFactor);

    out_color = vec4(finalColor, contourColor.a); // Use contourColor's alpha for transparency
}
