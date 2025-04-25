#include "WindowsApplication.h"
#include "VulkanContext.h"
#include "FractalRenderer.h"
#include <windowsx.h>
#include <CommCtrl.h>
#include <string>
#include <stdexcept>

#pragma comment(lib, "comctl32.lib")

// Mapping for window handles to application instances
std::unordered_map<HWND, WindowsApplication*> g_windowMap;

// Window class name
constexpr const wchar_t* WINDOW_CLASS_NAME = L"VulkanFractalRendererClass";

// Control IDs
constexpr int ID_FRACTAL_TYPE_COMBO = 101;
constexpr int ID_ITERATIONS_SLIDER = 102;
constexpr int ID_ITERATIONS_TEXT = 103;
constexpr int ID_PALETTE_COMBO = 104;
constexpr int ID_RESET_BUTTON = 105;

WindowsApplication::WindowsApplication(HINSTANCE hInstance, const std::wstring& title, int width, int height)
    : m_hInstance(hInstance)
    , m_title(title)
    , m_width(width)
    , m_height(height)
    , m_resizing(false)
    , m_hwnd(nullptr)
    , m_fractalType(FRACTAL_MANDELBROT)
    , m_maxIterations(100)
    , m_colorPalette(PALETTE_RAINBOW)
    , m_zoom(1.0f)
    , m_panX(0.0f)
    , m_panY(0.0f)
    , m_leftMouseDown(false)
    , m_lastMouseX(0)
    , m_lastMouseY(0)
    , m_fractalTypeCombo(nullptr)
    , m_iterationsSlider(nullptr)
    , m_iterationsText(nullptr)
    , m_paletteCombo(nullptr)
    , m_resetButton(nullptr) {

    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Register window class
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = WINDOW_CLASS_NAME;
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wcex)) {
        throw std::runtime_error("Failed to register window class");
    }

    // Calculate window size with controls area
    RECT windowRect = { 0, 0, width, height + 80 }; // Additional height for controls
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create window
    m_hwnd = CreateWindowW(
        WINDOW_CLASS_NAME,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!m_hwnd) {
        throw std::runtime_error("Failed to create window");
    }

    // Store this pointer for the window procedure
    g_windowMap[m_hwnd] = this;

    // Create UI controls
    CreateControls();

    // Show window
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    // Initialize Vulkan and renderer
    try {
        m_vulkanContext = std::make_unique<VulkanContext>(m_hwnd, width, height);
        m_fractalRenderer = std::make_unique<FractalRenderer>(m_vulkanContext.get());
        m_fractalRenderer->Initialize();
    } catch (const std::exception& e) {
        MessageBoxA(m_hwnd, e.what(), "Vulkan Initialization Error", MB_OK | MB_ICONERROR);
        throw;
    }
}

WindowsApplication::~WindowsApplication() {
    // Cleanup renderer and Vulkan before window destruction
    if (m_fractalRenderer) {
        m_fractalRenderer->Cleanup();
        m_fractalRenderer.reset();
    }
    
    m_vulkanContext.reset();

    // Remove from map and destroy window
    if (m_hwnd) {
        g_windowMap.erase(m_hwnd);
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    // Unregister window class
    if (m_hInstance) {
        UnregisterClassW(WINDOW_CLASS_NAME, m_hInstance);
    }
}

int WindowsApplication::Run() {
    MSG msg = {};
    bool running = true;

    // Main message loop
    while (running) {
        // Process all pending Windows messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // If we're still running, render the fractal
        if (running && m_fractalRenderer) {
            try {
                m_fractalRenderer->RenderFrame();
            } catch (const std::exception& e) {
                MessageBoxA(m_hwnd, e.what(), "Render Error", MB_OK | MB_ICONERROR);
                running = false;
                break;
            }
        }
    }

    // Make sure Vulkan device is idle before exiting
    if (m_vulkanContext) {
        try {
            // Wait for any in-progress GPU operations to complete
            vkDeviceWaitIdle(m_vulkanContext->GetDevice());
        } catch (...) {
            // Ignore errors during shutdown
        }
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WindowsApplication::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    // Get the application instance associated with this window
    auto it = g_windowMap.find(hwnd);
    if (it != g_windowMap.end()) {
        return it->second->HandleMessage(hwnd, message, wParam, lParam);
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT WindowsApplication::HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            int newWidth = LOWORD(lParam);
            int newHeight = HIWORD(lParam);
            
            // Adjust for control area
            newHeight -= 80;
            
            if (newWidth > 0 && newHeight > 0) {
                OnResize(newWidth, newHeight);
            }
        }
        break;

    case WM_ENTERSIZEMOVE:
        m_resizing = true;
        break;

    case WM_EXITSIZEMOVE:
        m_resizing = false;
        if (m_vulkanContext) {
            m_vulkanContext->RecreateSwapChain();
        }
        break;

    case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            OnMouseWheel(delta, x, y);
        }
        break;

    case WM_LBUTTONDOWN:
        m_leftMouseDown = true;
        m_lastMouseX = GET_X_LPARAM(lParam);
        m_lastMouseY = GET_Y_LPARAM(lParam);
        SetCapture(hwnd);
        break;

    case WM_LBUTTONUP:
        m_leftMouseDown = false;
        ReleaseCapture();
        break;

    case WM_MOUSEMOVE:
        if (m_leftMouseDown) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            OnMouseMove(x, y, true);
        }
        break;

    case WM_COMMAND:
        {
            int controlId = LOWORD(wParam);
            int notificationCode = HIWORD(wParam);
            HWND controlHwnd = (HWND)lParam;
            
            OnControlCommand(controlHwnd, notificationCode);
        }
        break;

    case WM_HSCROLL:
        {
            // Handle trackbar (slider) control notifications
            HWND controlHwnd = (HWND)lParam;
            int notificationCode = LOWORD(wParam);
            
            if (controlHwnd == m_iterationsSlider && 
                (notificationCode == TB_THUMBPOSITION || notificationCode == TB_THUMBTRACK || 
                 notificationCode == TB_ENDTRACK)) {
                
                int value = SendMessage(m_iterationsSlider, TBM_GETPOS, 0, 0);
                m_maxIterations = value;
                
                // Update text display
                wchar_t buffer[16];
                swprintf_s(buffer, L"%d", value);
                SetWindowTextW(m_iterationsText, buffer);
                
                if (m_fractalRenderer) {
                    m_fractalRenderer->SetMaxIterations(m_maxIterations);
                }
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    
    return 0;
}

void WindowsApplication::CreateControls() {
    // Get client area size
    RECT rect;
    GetClientRect(m_hwnd, &rect);
    int width = rect.right - rect.left;
    
    // Control dimensions
    const int CONTROL_HEIGHT = 25;
    const int LABEL_WIDTH = 100;
    const int CONTROL_WIDTH = 150;
    const int BUTTON_WIDTH = 100;
    const int MARGIN = 10;
    const int TOP_MARGIN = rect.bottom - 80; // Bottom 80 pixels reserved for controls
    
    // Create fractal type combo box
    CreateWindowW(L"STATIC", L"Fractal Type:", WS_CHILD | WS_VISIBLE,
        MARGIN, TOP_MARGIN + 10, LABEL_WIDTH, CONTROL_HEIGHT, m_hwnd, nullptr, m_hInstance, nullptr);
    
    m_fractalTypeCombo = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        MARGIN + LABEL_WIDTH, TOP_MARGIN + 10, CONTROL_WIDTH, CONTROL_HEIGHT * 6, m_hwnd, (HMENU)ID_FRACTAL_TYPE_COMBO, m_hInstance, nullptr);
    
    SendMessage(m_fractalTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Mandelbrot");
    SendMessage(m_fractalTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Julia");
    SendMessage(m_fractalTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Burning Ship");
    SendMessage(m_fractalTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Tricorn");
    SendMessage(m_fractalTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Multibrot");
    SendMessage(m_fractalTypeCombo, CB_SETCURSEL, 0, 0);
    RegisterControl(m_fractalTypeCombo, "fractalType");
    
    // Create iterations slider and label
    CreateWindowW(L"STATIC", L"Iterations:", WS_CHILD | WS_VISIBLE,
        MARGIN, TOP_MARGIN + 10 + CONTROL_HEIGHT + 5, LABEL_WIDTH, CONTROL_HEIGHT, m_hwnd, nullptr, m_hInstance, nullptr);
    
    m_iterationsSlider = CreateWindowW(L"TRACKBAR_CLASS", L"", 
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS, 
        MARGIN + LABEL_WIDTH, TOP_MARGIN + 10 + CONTROL_HEIGHT + 5, CONTROL_WIDTH, CONTROL_HEIGHT, 
        m_hwnd, (HMENU)ID_ITERATIONS_SLIDER, m_hInstance, nullptr);
    
    SendMessage(m_iterationsSlider, TBM_SETRANGE, TRUE, MAKELPARAM(10, 1000));
    SendMessage(m_iterationsSlider, TBM_SETPOS, TRUE, 100);
    RegisterControl(m_iterationsSlider, "iterationsSlider");
    
    m_iterationsText = CreateWindowW(L"STATIC", L"100", WS_CHILD | WS_VISIBLE | SS_RIGHT,
        MARGIN + LABEL_WIDTH + CONTROL_WIDTH + 5, TOP_MARGIN + 10 + CONTROL_HEIGHT + 5, 50, CONTROL_HEIGHT,
        m_hwnd, (HMENU)ID_ITERATIONS_TEXT, m_hInstance, nullptr);
    
    // Create color palette combo box
    CreateWindowW(L"STATIC", L"Color Palette:", WS_CHILD | WS_VISIBLE,
        width - MARGIN - LABEL_WIDTH - CONTROL_WIDTH, TOP_MARGIN + 10, LABEL_WIDTH, CONTROL_HEIGHT,
        m_hwnd, nullptr, m_hInstance, nullptr);
    
    m_paletteCombo = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        width - MARGIN - CONTROL_WIDTH, TOP_MARGIN + 10, CONTROL_WIDTH, CONTROL_HEIGHT * 6,
        m_hwnd, (HMENU)ID_PALETTE_COMBO, m_hInstance, nullptr);
    
    SendMessage(m_paletteCombo, CB_ADDSTRING, 0, (LPARAM)L"Rainbow");
    SendMessage(m_paletteCombo, CB_ADDSTRING, 0, (LPARAM)L"Fire");
    SendMessage(m_paletteCombo, CB_ADDSTRING, 0, (LPARAM)L"Ocean");
    SendMessage(m_paletteCombo, CB_ADDSTRING, 0, (LPARAM)L"Grayscale");
    SendMessage(m_paletteCombo, CB_ADDSTRING, 0, (LPARAM)L"Electric");
    SendMessage(m_paletteCombo, CB_SETCURSEL, 0, 0);
    RegisterControl(m_paletteCombo, "paletteCombo");
    
    // Reset view button
    m_resetButton = CreateWindowW(L"BUTTON", L"Reset View", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        width - MARGIN - BUTTON_WIDTH, TOP_MARGIN + 10 + CONTROL_HEIGHT + 5, BUTTON_WIDTH, CONTROL_HEIGHT,
        m_hwnd, (HMENU)ID_RESET_BUTTON, m_hInstance, nullptr);
    RegisterControl(m_resetButton, "resetButton");
}

void WindowsApplication::RegisterControl(HWND control, const std::string& id) {
    if (control) {
        m_controlMap[control] = id;
    }
}

void WindowsApplication::OnResize(int width, int height) {
    m_width = width;
    m_height = height;

    // Only update Vulkan if not currently in a resize operation
    if (!m_resizing && m_vulkanContext) {
        m_vulkanContext->SetWindowSize(width, height);
        
        if (m_fractalRenderer) {
            m_fractalRenderer->RecreateSwapChain();
        }
    }

    // Update layout of controls
    LayoutControls();
}

void WindowsApplication::LayoutControls() {
    // Get updated client area size
    RECT rect;
    GetClientRect(m_hwnd, &rect);
    int width = rect.right - rect.left;
    
    // Move controls as needed
    // (in a real application, we would update positions of all controls here)
    if (m_paletteCombo) {
        SetWindowPos(m_paletteCombo, nullptr, 
            width - 10 - 150, rect.bottom - 70, 
            0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
    
    if (m_resetButton) {
        SetWindowPos(m_resetButton, nullptr, 
            width - 10 - 100, rect.bottom - 40, 
            0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void WindowsApplication::OnMouseWheel(int delta, int x, int y) {
    const float ZOOM_FACTOR = 1.1f;
    
    if (delta > 0) {
        m_zoom *= ZOOM_FACTOR;
    } else {
        m_zoom /= ZOOM_FACTOR;
    }
    
    if (m_fractalRenderer) {
        m_fractalRenderer->SetZoom(m_zoom);
    }
}

void WindowsApplication::OnMouseMove(int x, int y, bool leftButtonDown) {
    if (leftButtonDown) {
        // Calculate delta movement in screen coordinates
        float deltaX = static_cast<float>(x - m_lastMouseX);
        float deltaY = static_cast<float>(y - m_lastMouseY);
        
        // Convert to fractal coordinate space
        // The conversion factor depends on zoom level and screen size
        float moveScaleX = 2.0f / (m_width * m_zoom);
        float moveScaleY = 2.0f / (m_height * m_zoom);
        
        // Invert Y direction for natural panning
        m_panX += deltaX * moveScaleX;
        m_panY -= deltaY * moveScaleY;
        
        if (m_fractalRenderer) {
            m_fractalRenderer->SetPan(m_panX, m_panY);
        }
    }
    
    m_lastMouseX = x;
    m_lastMouseY = y;
}

void WindowsApplication::OnControlCommand(HWND controlHwnd, int notificationCode) {
    if (!controlHwnd) {
        return;
    }
    
    auto it = m_controlMap.find(controlHwnd);
    if (it == m_controlMap.end()) {
        return;
    }

    const std::string& controlId = it->second;
    
    if (controlId == "fractalType" && notificationCode == CBN_SELCHANGE && m_fractalTypeCombo) {
        int selection = SendMessage(m_fractalTypeCombo, CB_GETCURSEL, 0, 0);
        m_fractalType = selection;
        
        if (m_fractalRenderer) {
            m_fractalRenderer->SetFractalType(static_cast<FractalType>(m_fractalType));
        }
    }
    else if (controlId == "iterationsSlider" && notificationCode == TB_ENDTRACK && m_iterationsSlider && m_iterationsText) {
        int value = SendMessage(m_iterationsSlider, TBM_GETPOS, 0, 0);
        m_maxIterations = value;
        
        // Update text display
        wchar_t buffer[16];
        swprintf_s(buffer, L"%d", value);
        SetWindowTextW(m_iterationsText, buffer);
        
        if (m_fractalRenderer) {
            m_fractalRenderer->SetMaxIterations(m_maxIterations);
        }
    }
    else if (controlId == "paletteCombo" && notificationCode == CBN_SELCHANGE && m_paletteCombo) {
        int selection = SendMessage(m_paletteCombo, CB_GETCURSEL, 0, 0);
        m_colorPalette = selection;
        
        if (m_fractalRenderer) {
            m_fractalRenderer->SetColorPalette(static_cast<ColorPalette>(m_colorPalette));
        }
    }
    else if (controlId == "resetButton" && notificationCode == BN_CLICKED) {
        // Reset view parameters
        m_zoom = 1.0f;
        m_panX = 0.0f;
        m_panY = 0.0f;
        
        if (m_fractalRenderer) {
            m_fractalRenderer->ResetView();
        }
    }
}

void WindowsApplication::SetFractalType(int type) {
    m_fractalType = type;
    if (m_fractalTypeCombo) {
        SendMessage(m_fractalTypeCombo, CB_SETCURSEL, type, 0);
    }
    
    if (m_fractalRenderer) {
        m_fractalRenderer->SetFractalType(static_cast<FractalType>(type));
    }
}

void WindowsApplication::SetMaxIterations(int iterations) {
    m_maxIterations = iterations;
    if (m_iterationsSlider) {
        SendMessage(m_iterationsSlider, TBM_SETPOS, TRUE, iterations);
    }
    
    // Update text display
    if (m_iterationsText) {
        wchar_t buffer[16];
        swprintf_s(buffer, L"%d", iterations);
        SetWindowTextW(m_iterationsText, buffer);
    }
    
    if (m_fractalRenderer) {
        m_fractalRenderer->SetMaxIterations(iterations);
    }
}

void WindowsApplication::SetColorPalette(int palette) {
    m_colorPalette = palette;
    if (m_paletteCombo) {
        SendMessage(m_paletteCombo, CB_SETCURSEL, palette, 0);
    }
    
    if (m_fractalRenderer) {
        m_fractalRenderer->SetColorPalette(static_cast<ColorPalette>(palette));
    }
}

void WindowsApplication::SetZoom(float zoom) {
    m_zoom = zoom;
    
    if (m_fractalRenderer) {
        m_fractalRenderer->SetZoom(zoom);
    }
}

void WindowsApplication::SetPanX(float x) {
    m_panX = x;
    
    if (m_fractalRenderer) {
        m_fractalRenderer->SetPan(m_panX, m_panY);
    }
}

void WindowsApplication::SetPanY(float y) {
    m_panY = y;
    
    if (m_fractalRenderer) {
        m_fractalRenderer->SetPan(m_panX, m_panY);
    }
}