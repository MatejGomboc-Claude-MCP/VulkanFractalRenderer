#include "WindowsApplication.h"
#include <Windows.h>
#include <memory>
#include <stdexcept>
#include <string>

// Convert a regular string to a wide string for Windows API
std::wstring StringToWString(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }
    
    // Get the required buffer size
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (size <= 0) {
        throw std::runtime_error("Failed to convert string to wide string");
    }
    
    // Allocate the wide string buffer (with correct size)
    std::wstring wstr(size - 1, 0); // -1 because size includes null terminator
    
    // Perform the conversion
    if (MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr.data(), size) == 0) {
        throw std::runtime_error("Failed to convert string to wide string");
    }
    
    return wstr;
}

// Entry point for Windows applications
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    try {
        // Create and run the application
        const int initialWidth = 1280;
        const int initialHeight = 720;
        const std::wstring appTitle = L"Vulkan Fractal Renderer";

        std::unique_ptr<WindowsApplication> app = std::make_unique<WindowsApplication>(
            hInstance, 
            appTitle, 
            initialWidth, 
            initialHeight
        );
        
        return app->Run();
    } catch (const std::exception& e) {
        // Display error message
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }
}
