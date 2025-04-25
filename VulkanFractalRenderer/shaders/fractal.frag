#version 450

// Input from vertex shader
layout(location = 0) in vec2 fragCoord;

// Output color
layout(location = 0) out vec4 outColor;

// Uniform buffer containing fractal parameters
layout(binding = 0) uniform FractalUBO {
    float centerX;      // Center position X
    float centerY;      // Center position Y
    float scale;        // Zoom scale (larger for zoomed out)
    float aspectRatio;  // Width/Height ratio of the viewport
    
    int fractalType;    // Type of fractal to render
    int maxIterations;  // Maximum iteration count
    int colorPalette;   // Color palette to use
    int padding;        // Padding to maintain alignment
    
    // For Julia set
    float juliaConstantX;
    float juliaConstantY;
    // For Multibrot
    float multibrotPower;
    float reserved;
} ubo;

// Fractal types
const int FRACTAL_MANDELBROT = 0;
const int FRACTAL_JULIA = 1;
const int FRACTAL_BURNING_SHIP = 2;
const int FRACTAL_TRICORN = 3;
const int FRACTAL_MULTIBROT = 4;

// Color palettes
const int PALETTE_RAINBOW = 0;
const int PALETTE_FIRE = 1;
const int PALETTE_OCEAN = 2;
const int PALETTE_GRAYSCALE = 3;
const int PALETTE_ELECTRIC = 4;

// Helper function to map complex plane to screen coordinates
vec2 mapToComplex(vec2 coord) {
    // Adjust for aspect ratio
    vec2 c = coord;
    c.x = c.x * 2.0 - 1.0;  // Map from [0,1] to [-1,1]
    c.y = c.y * 2.0 - 1.0;  // Map from [0,1] to [-1,1]
    
    // Apply aspect ratio correction
    c.x *= ubo.aspectRatio;
    
    // Apply zoom and panning
    c *= ubo.scale;
    c.x += ubo.centerX;
    c.y += ubo.centerY;
    
    return c;
}

// Mandelbrot fractal calculation
int calculateMandelbrot(vec2 c) {
    vec2 z = vec2(0.0, 0.0);
    int iterations = 0;
    
    for(int i = 0; i < ubo.maxIterations; i++) {
        // z = z² + c
        vec2 zSquared = vec2(
            z.x * z.x - z.y * z.y,
            2.0 * z.x * z.y
        );
        z = zSquared + c;
        
        // Check if escaped
        if(dot(z, z) > 4.0) {
            return i;
        }
        
        iterations++;
    }
    
    return iterations;
}

// Julia set calculation
int calculateJulia(vec2 z) {
    vec2 c = vec2(ubo.juliaConstantX, ubo.juliaConstantY);
    int iterations = 0;
    
    for(int i = 0; i < ubo.maxIterations; i++) {
        // z = z² + c
        vec2 zSquared = vec2(
            z.x * z.x - z.y * z.y,
            2.0 * z.x * z.y
        );
        z = zSquared + c;
        
        // Check if escaped
        if(dot(z, z) > 4.0) {
            return i;
        }
        
        iterations++;
    }
    
    return iterations;
}

// Burning Ship fractal calculation
int calculateBurningShip(vec2 c) {
    vec2 z = vec2(0.0, 0.0);
    int iterations = 0;
    
    for(int i = 0; i < ubo.maxIterations; i++) {
        // Take absolute values
        z = abs(z);
        
        // z = z² + c
        vec2 zSquared = vec2(
            z.x * z.x - z.y * z.y,
            2.0 * z.x * z.y
        );
        z = zSquared + c;
        
        // Check if escaped
        if(dot(z, z) > 4.0) {
            return i;
        }
        
        iterations++;
    }
    
    return iterations;
}

// Tricorn (Mandelbar) fractal calculation
int calculateTricorn(vec2 c) {
    vec2 z = vec2(0.0, 0.0);
    int iterations = 0;
    
    for(int i = 0; i < ubo.maxIterations; i++) {
        // z = conj(z)² + c
        vec2 zSquared = vec2(
            z.x * z.x - z.y * z.y,
            -2.0 * z.x * z.y  // conjugate
        );
        z = zSquared + c;
        
        // Check if escaped
        if(dot(z, z) > 4.0) {
            return i;
        }
        
        iterations++;
    }
    
    return iterations;
}

// Multibrot fractal calculation with customizable power
int calculateMultibrot(vec2 c) {
    vec2 z = vec2(0.0, 0.0);
    int iterations = 0;
    float power = ubo.multibrotPower;
    
    for(int i = 0; i < ubo.maxIterations; i++) {
        // z = z^power + c (using complex polar form)
        float r = length(z);
        if(r > 0.0) {
            float theta = atan(z.y, z.x);
            float rPow = pow(r, power);
            float newTheta = theta * power;
            z = vec2(rPow * cos(newTheta), rPow * sin(newTheta)) + c;
        } else {
            z = c;
        }
        
        // Check if escaped
        if(dot(z, z) > 4.0) {
            return i;
        }
        
        iterations++;
    }
    
    return iterations;
}

// Color palette functions
vec3 rainbowPalette(float t) {
    t = clamp(t, 0.0, 1.0);
    float r = 0.5 + 0.5 * sin(3.1415926 + t * 20.0);
    float g = 0.5 + 0.5 * sin(1.5 + t * 20.0);
    float b = 0.5 + 0.5 * sin(t * 20.0);
    return vec3(r, g, b);
}

vec3 firePalette(float t) {
    t = clamp(t, 0.0, 1.0);
    float r = min(1.0, t * 4.0);
    float g = max(0.0, min(1.0, t * 4.0 - 1.0));
    float b = max(0.0, min(1.0, t * 4.0 - 3.0));
    return vec3(r, g, b);
}

vec3 oceanPalette(float t) {
    t = clamp(t, 0.0, 1.0);
    float r = max(0.0, min(1.0, t * 4.0 - 3.0));
    float g = max(0.0, min(1.0, t * 4.0 - 2.0));
    float b = min(1.0, t * 4.0);
    return vec3(r, g, b);
}

vec3 grayscalePalette(float t) {
    t = clamp(t, 0.0, 1.0);
    return vec3(t, t, t);
}

vec3 electricPalette(float t) {
    t = clamp(t, 0.0, 1.0);
    vec3 color = vec3(0.0);
    color.r = 0.5 + 0.5 * sin(t * 25.0);
    color.g = 0.5 + 0.5 * sin(t * 25.0 + 2.1);
    color.b = 1.0;
    return color;
}

// Apply the selected color palette
vec3 applyColorPalette(float t) {
    switch(ubo.colorPalette) {
        case PALETTE_RAINBOW:
            return rainbowPalette(t);
        case PALETTE_FIRE:
            return firePalette(t);
        case PALETTE_OCEAN:
            return oceanPalette(t);
        case PALETTE_GRAYSCALE:
            return grayscalePalette(t);
        case PALETTE_ELECTRIC:
            return electricPalette(t);
        default:
            return rainbowPalette(t);
    }
}

// Calculate smooth coloring based on iteration count
vec3 calculateColor(int iterations) {
    // Black for maximum iterations (interior of set)
    if(iterations == ubo.maxIterations) {
        return vec3(0.0, 0.0, 0.0);
    }
    
    // Normalized iteration count (smooth coloring)
    float t = float(iterations) / float(ubo.maxIterations);
    return applyColorPalette(t);
}

void main() {
    // Map screen coordinates to complex plane
    vec2 c = mapToComplex(fragCoord);
    
    // Calculate iterations based on fractal type
    int iterations = 0;
    
    switch(ubo.fractalType) {
        case FRACTAL_MANDELBROT:
            iterations = calculateMandelbrot(c);
            break;
        case FRACTAL_JULIA:
            iterations = calculateJulia(c);
            break;
        case FRACTAL_BURNING_SHIP:
            iterations = calculateBurningShip(c);
            break;
        case FRACTAL_TRICORN:
            iterations = calculateTricorn(c);
            break;
        case FRACTAL_MULTIBROT:
            iterations = calculateMultibrot(c);
            break;
        default:
            iterations = calculateMandelbrot(c);
            break;
    }
    
    // Apply color palette
    vec3 color = calculateColor(iterations);
    
    // Output final color
    outColor = vec4(color, 1.0);
}
