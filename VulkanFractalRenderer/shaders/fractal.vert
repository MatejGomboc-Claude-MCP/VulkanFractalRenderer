#version 450

// Output position and texture coordinates
layout(location = 0) out vec2 fragCoord;

// Vertex shader for a full-screen quad (no input vertices needed)
// We use a triangle strip to create two triangles that cover the entire screen

void main() {
    // Create a full-screen quad with just the vertex ID
    // Use 6 vertices to create 2 triangles with a triangle list
    vec2 positions[6] = vec2[](
        vec2(-1.0, -1.0), // Bottom-left triangle
        vec2( 1.0, -1.0),
        vec2(-1.0,  1.0),
        
        vec2( 1.0, -1.0), // Top-right triangle
        vec2( 1.0,  1.0),
        vec2(-1.0,  1.0)
    );
    
    // Normalized device coordinates (NDC) position
    vec2 position = positions[gl_VertexIndex];
    
    // Pass the position to the fragment shader as texture coordinates
    fragCoord = position * 0.5 + 0.5; // Convert from [-1,1] to [0,1] range
    
    // Output vertex position (no transformation needed since already in NDC)
    gl_Position = vec4(position, 0.0, 1.0);
}
