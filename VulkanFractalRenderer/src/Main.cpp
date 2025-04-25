#include "WindowsApplication.h"
#include <Windows.h>
#include <memory>
#include <stdexcept>
#include <string>

// Convert a regular string to a wide string for Windows API
std::wstring StringToWString(const std::string& str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr.data(), size);
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
