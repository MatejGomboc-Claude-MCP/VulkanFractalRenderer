@echo off
REM This batch file compiles the GLSL shaders to SPIR-V

echo Compiling shaders...

if not defined VULKAN_SDK (
    echo Error: VULKAN_SDK environment variable is not defined. 
    echo Please install the Vulkan SDK and ensure the environment variable is set.
    exit /b 1
)

echo Using Vulkan SDK at: %VULKAN_SDK%

REM Ensure the output directories exist
if not exist "shaders" mkdir shaders
if not exist "x64\Debug\shaders" mkdir "x64\Debug\shaders"
if not exist "x64\Release\shaders" mkdir "x64\Release\shaders"

REM Compile the vertex shader
echo Compiling vertex shader...
"%VULKAN_SDK%\Bin\glslc.exe" VulkanFractalRenderer\shaders\fractal.vert -o VulkanFractalRenderer\shaders\fractal.vert.spv
if %ERRORLEVEL% neq 0 (
    echo Error compiling vertex shader!
    exit /b 1
)

REM Compile the fragment shader
echo Compiling fragment shader...
"%VULKAN_SDK%\Bin\glslc.exe" VulkanFractalRenderer\shaders\fractal.frag -o VulkanFractalRenderer\shaders\fractal.frag.spv
if %ERRORLEVEL% neq 0 (
    echo Error compiling fragment shader!
    exit /b 1
)

REM Copy compiled shaders to output directories
echo Copying shaders to output directories...
copy /Y VulkanFractalRenderer\shaders\*.spv x64\Debug\shaders\
copy /Y VulkanFractalRenderer\shaders\*.spv x64\Release\shaders\

echo Shader compilation completed successfully!
