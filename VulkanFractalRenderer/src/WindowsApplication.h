#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

// Forward declaration
class VulkanContext;
class FractalRenderer;

class WindowsApplication {
public:
    WindowsApplication(HINSTANCE hInstance, const std::wstring& title, int width, int height);
    ~WindowsApplication();

    // Delete copy constructors
    WindowsApplication(const WindowsApplication&) = delete;
    WindowsApplication& operator=(const WindowsApplication&) = delete;

    // Run the application
    int Run();

    // Window properties
    HWND GetHwnd() const { return m_hwnd; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    bool IsResizing() const { return m_resizing; }

    // Fractal parameters
    void SetFractalType(int type);
    void SetMaxIterations(int iterations);
    void SetColorPalette(int palette);
    void SetZoom(float zoom);
    void SetPanX(float x);
    void SetPanY(float y);

private:
    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // UI creation
    void CreateControls();
    void LayoutControls();
    void RegisterControl(HWND control, const std::string& id);
    
    // Message handlers
    void OnResize(int width, int height);
    void OnMouseWheel(int delta, int x, int y);
    void OnMouseMove(int x, int y, bool leftButtonDown);
    void OnControlCommand(HWND controlHwnd, int notificationCode);

    // Window data
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    std::wstring m_title;
    int m_width;
    int m_height;
    bool m_resizing;

    // Fractal parameters
    int m_fractalType;
    int m_maxIterations;
    int m_colorPalette;
    float m_zoom;
    float m_panX;
    float m_panY;
    bool m_leftMouseDown;
    int m_lastMouseX;
    int m_lastMouseY;

    // UI Controls
    HWND m_fractalTypeCombo;
    HWND m_iterationsSlider;
    HWND m_iterationsText;
    HWND m_paletteCombo;
    HWND m_resetButton;
    std::unordered_map<HWND, std::string> m_controlMap;

    // Rendering objects
    std::unique_ptr<VulkanContext> m_vulkanContext;
    std::unique_ptr<FractalRenderer> m_fractalRenderer;
};
