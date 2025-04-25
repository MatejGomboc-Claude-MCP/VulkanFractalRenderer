# Vulkan Fractal Renderer

A high-performance fractal renderer using C++ and Vulkan, optimized for slower Windows PCs.

## Features

- Real-time rendering of various fractal types (Mandelbrot, Julia, etc.)
- Windows API-based user interface
- Vulkan-accelerated rendering for optimal performance
- Zoom and pan controls for fractal exploration
- Color palette customization
- Parameter controls for fractal variation

## Requirements

- Windows 10/11
- Visual Studio 2019 or 2022
- Vulkan SDK (1.3.x or later)
- A GPU with Vulkan support (even low-end GPUs will work)

## Building the Project

1. Ensure you have the Vulkan SDK installed
2. Set the `VULKAN_SDK` environment variable to your Vulkan SDK installation path
3. Open `VulkanFractalRenderer.sln` in Visual Studio
4. Build the solution

## Usage

After building, run the application from Visual Studio or navigate to the output directory and run `VulkanFractalRenderer.exe`.

- Left mouse button: Pan around the fractal
- Mouse wheel: Zoom in/out
- UI controls: Adjust fractal parameters and color settings

## Optimization Features

- SIMD instructions for CPU-side calculations
- Vulkan compute shaders for parallel processing
- Memory pooling to reduce allocation overhead
- Adaptive rendering quality based on system performance
