# Vulkan Fractal Renderer

A high-performance fractal renderer using C++ and Vulkan, optimized for slower Windows PCs. This application renders various fractal types in real-time with an easy-to-use Windows interface.

![Fractal Renderer Screenshot](https://raw.githubusercontent.com/MatejGomboc-Claude-MCP/VulkanFractalRenderer/main/screenshot.png)

## Features

- Real-time rendering of multiple fractal types:
  - Mandelbrot Set
  - Julia Set
  - Burning Ship Fractal
  - Tricorn (Mandelbar) Fractal
  - Multibrot Set (with adjustable power)

- Interactive controls:
  - Pan using left mouse button drag
  - Zoom with mouse wheel
  - UI controls for fractal type, iterations, and color palette

- Performance optimizations:
  - Vulkan GPU acceleration
  - SIMD instructions for CPU calculations
  - Optimized shader code for maximum performance on slower hardware
  - Multi-frame rendering with double buffering

- Color palettes:
  - Rainbow
  - Fire
  - Ocean
  - Grayscale
  - Electric

## Requirements

- Windows 10 or 11
- Visual Studio 2019 or 2022
- Vulkan SDK 1.3.x or later
- A GPU with Vulkan support (even low-end integrated GPUs will work)

## Building the Project

1. Install the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) (version 1.3.x or later)
2. Make sure the `VULKAN_SDK` environment variable is set to your Vulkan SDK installation path
3. Clone this repository:
   ```
   git clone https://github.com/MatejGomboc-Claude-MCP/VulkanFractalRenderer.git
   ```
4. Compile the shaders:
   ```
   cd VulkanFractalRenderer
   %VULKAN_SDK%\Bin\glslc.exe VulkanFractalRenderer\shaders\fractal.vert -o VulkanFractalRenderer\shaders\fractal.vert.spv
   %VULKAN_SDK%\Bin\glslc.exe VulkanFractalRenderer\shaders\fractal.frag -o VulkanFractalRenderer\shaders\fractal.frag.spv
   ```
5. Open `VulkanFractalRenderer.sln` in Visual Studio
6. Build the solution (F7 or Ctrl+Shift+B)
7. Copy the compiled shader files (`fractal.vert.spv` and `fractal.frag.spv`) to the output directory

## Usage

After building, run the application from Visual Studio or navigate to the output directory and run `VulkanFractalRenderer.exe`.

### Controls

- **Left Mouse Button**: Click and drag to move around the fractal
- **Mouse Wheel**: Zoom in and out of the fractal
- **UI Controls**:
  - **Fractal Type**: Select the type of fractal to render
  - **Iterations**: Adjust the level of detail (higher values show more detail but reduce performance)
  - **Color Palette**: Select color scheme for visualization
  - **Reset View**: Return to the default view

## Implementation Details

### Architecture

The application consists of three main components:

1. **Windows Application Layer** (`WindowsApplication.h/cpp`)
   - Handles Windows API, user input, and UI
   - Creates the window and message loop
   - Processes UI events and user interactions

2. **Vulkan Context** (`VulkanContext.h/cpp`)
   - Initializes the Vulkan API
   - Manages device selection and swap chain
   - Handles presentation and synchronization

3. **Fractal Renderer** (`FractalRenderer.h/cpp`)
   - Sets up rendering pipeline
   - Manages fractal parameters and shaders
   - Updates and draws each frame

### Rendering Process

The fractals are rendered using the following process:

1. A full-screen quad is drawn using a vertex shader
2. The fragment shader computes the fractal for each pixel:
   - Maps screen coordinates to complex plane coordinates
   - Iterates the fractal formula for the specific fractal type
   - Uses the iteration count to determine color
   - Applies the selected color palette

### Performance Optimizations

The renderer includes several optimizations to ensure smooth performance even on slower hardware:

1. **GPU-based computation**: The fractal calculations are performed in the fragment shader on the GPU
2. **SIMD instructions**: CPU-side calculations use SIMD instructions where appropriate
3. **Optimal memory usage**: Minimizes allocations and uses memory pooling
4. **Double buffering**: Allows simultaneous rendering and display
5. **Efficient command buffer usage**: Reduces API overhead
6. **Compiler optimizations**: Release builds use optimized builds with enhanced instruction sets

## License

This project is open source and available under the MIT License.

## Acknowledgements

- The Vulkan SDK provided by [LunarG](https://www.lunarg.com/)
- Various resources on fractal mathematics and rendering techniques
