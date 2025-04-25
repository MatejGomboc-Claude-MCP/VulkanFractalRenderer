#version 450

// Output position and texture coordinates
layout(location = 0) out vec2 fragCoord;

// Vertex shader for a full-screen quad (no input vertices needed)
// Using a more efficient approach with a single triangle that covers the screen

void main() {
    // Use vertex ID to create a large triangle that covers the entire screen
    // This is more efficient than using two triangles
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),  // Bottom left
        vec2(3.0, -1.0),   // Bottom right (extended to ensure coverage)
        vec2(-1.0, 3.0)    // Top left (extended to ensure coverage)
    );
    
    // Get the position from the vertex ID
    vec2 position = positions[gl_VertexIndex % 3];
    
    // Pass the position to the fragment shader as texture coordinates
    // Map from [-1,1] to [0,1] range
    fragCoord = position * 0.5 + 0.5;
    
    // Output vertex position (no transformation needed since already in NDC)
    gl_Position = vec4(position, 0.0, 1.0);
}
