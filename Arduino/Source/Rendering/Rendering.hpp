#pragma once
#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <stdexcept>

class Rendering {
public:
    Rendering()
    {
        CreateWindowAndDX11();
        InitializeImGui();
    }

    ~Rendering()
    {
        SAFE_RELEASE(render_target_view);
        SAFE_RELEASE(swap_chain);
        SAFE_RELEASE(device_context);
        SAFE_RELEASE(device);
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        DestroyWindow(hwnd);
        UnregisterClassW(L"ImGuiDX11App", GetModuleHandle(nullptr));
    }

    void CreateWindowAndDX11();
    void InitializeImGui();
    void ResizeBuffers(UINT width, UINT height);
    void Run();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); 
private:
    HWND hwnd = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* device_context = nullptr;
    IDXGISwapChain* swap_chain = nullptr;
    ID3D11RenderTargetView* render_target_view = nullptr;
public: 
    ImFont* m_default_font; 
    ImFont* m_icon_font; 
};

inline Rendering* g_Rendering; 