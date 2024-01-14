#version 430

// Input and output topologies
layout(triangles) in;
layout(line_strip, max_vertices = 18) out;

// Inputs from vertex shader
layout(location = 0) in vec3 fragPosition[]; // World space positions
layout(location = 1) in vec3 fragNormal[];   // World space normals

// Uniform properties
uniform mat4 Projection;
uniform vec3 cameraPos; // Camera position in world space
uniform mat4 viewMatrices[6]; // View matrices for each cubemap face

// Function to determine if an edge is a silhouette
bool IsSilhouetteEdge(vec3 normal1, vec3 normal2, vec3 viewVector) {
    return (dot(normal1, viewVector) * dot(normal2, viewVector) < 0);
}

// Interpolation function to find the silhouette point on an edge
vec3 InterpolateSilhouettePoint(vec3 pos1, vec3 pos2, float dot1, float dot2) {
    float t = dot1 / (dot1 - dot2);
    return pos1 + (pos2 - pos1) * t;
}

void main() {
    int layer;
    for (layer = 0; layer < 6; layer++) {
        gl_Layer = layer;
        vec3 silhouettePoints[2];
        int silhouetteCount = 0;

        // Adjust the view vector for the current cubemap face
        mat4 currentViewMatrix = viewMatrices[layer];
        vec3 viewVectors[3];
        for (int i = 0; i < 3; ++i) {
            vec3 worldPosition = vec3(currentViewMatrix * vec4(fragPosition[i], 1.0));
            viewVectors[i] = normalize(-worldPosition); // View vector is towards the origin in cubemap space
        }

        // Check each edge of the triangle
        for (int i = 0; i < 3; ++i) {
            int next = (i + 1) % 3;

            // Calculate dot products for the edge vertices
            float dot1 = dot(fragNormal[i], viewVectors[i]);
            float dot2 = dot(fragNormal[next], viewVectors[next]);

            if (IsSilhouetteEdge(fragNormal[i], fragNormal[next], viewVectors[i])) {
                // Find the silhouette points by interpolating along the edge
                silhouettePoints[silhouetteCount++] = InterpolateSilhouettePoint(fragPosition[i], fragPosition[next], dot1, dot2);
                if (silhouetteCount >= 2) break; // Only need two points for a line
            }
        }

        // If two silhouette points have been found, draw a line between them
        if (silhouetteCount == 2) {
            gl_Position = Projection * vec4(silhouettePoints[0], 1.0);
            EmitVertex();

            gl_Position = Projection * vec4(silhouettePoints[1], 1.0);
            EmitVertex();

            EndPrimitive();
        }
    }
}
